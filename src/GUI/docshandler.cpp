/*
* Copyright (C) 2016, Denys Senkin <denisx9.0c@gmail.com>
*
* This file is part of apertium-gp
*
* apertium-gp is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* apertium-gp is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with apertium-gp.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "docshandler.h"
#include "ui_docshandler.h"
#include "translator.h"
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>
#include <QDesktopServices>
#include <QProgressDialog>
#include <QDebug>
const QStringList DocsHandler::fileTypes {"txt", "docx", "pptx", "html",
                                           "rtf", "odt", "xlsx", "xtg"};
DocsHandler::DocsHandler(GpMainWindow *parent)
    : ui(new Ui::DocsHandler),
      parent(parent)
{
    ui->setupUi(this);
    QPalette Pal(palette());
    // устанавливаем цвет фона
    Pal.setColor(QPalette::Background, Qt::white);
    ui->DropWidget->setPalette(Pal);
    setAttribute(Qt::WA_DeleteOnClose);

    connect(this, &DocsHandler::docForTransChoosed, [&]()
    {
        docTransWaitDlg = new QProgressDialog(tr("Translating document..."),"",0,0,this);
    docTransWaitDlg->setCancelButton(nullptr);
    docTransWaitDlg->setWindowFlags(docTransWaitDlg->windowFlags() & ~Qt::WindowCloseButtonHint);
    docTransWaitDlg->setModal(true);
    docTransWaitDlg->show();
    });
    connect(this, &DocsHandler::docForTransChoosed, parent->getTranslator(), &Translator::docTranslate);
    connect(ui->DropWidget, &DragnDropWidget::documentDropped, this, &DocsHandler::docForTransChoosed);
    connect(parent->getTranslator(),&Translator::docTranslated,this,&DocsHandler::showPostDocTransDlg);
    connect(parent->getTranslator(), &Translator::docTranslateRejected,this,&DocsHandler::docTranslateFailed);
}

DocsHandler::~DocsHandler()
{
    delete ui;
}

void DocsHandler::on_browseBtn_clicked()
{
    //FIXME: format names
    QString filePath = QFileDialog::getOpenFileName(this,tr("Choose document to translate"),
                                                     QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                                                  tr(QString("Documents (*."+fileTypes.join(" *.")+")").toStdString().c_str()));
    if(filePath.isEmpty())
        return;

    //TODO: implement cancel button
    emit docForTransChoosed(filePath);
}

void DocsHandler::showPostDocTransDlg(QString trFilePath)
{
    docTransWaitDlg->accept();
    auto btnDlg = new QMessageBox(this);
    btnDlg->setWindowTitle(tr("Translation finished"));
    btnDlg->setText(tr("Document has been successfully translated."));
    auto openFileButton = new QPushButton(tr("Open translated file"),btnDlg);
    connect(openFileButton, &QPushButton::clicked, [&]()
    {
        QDesktopServices::openUrl(QUrl("file:///"+trFilePath,QUrl::TolerantMode));
        btnDlg->accept();
        qApp->processEvents();
        btnDlg->deleteLater();
    });
    auto openFolderButton = new QPushButton(tr("Open folder with translated file"),btnDlg);
    connect(openFolderButton, &QPushButton::clicked, [&]()
    {
        QFileInfo f(trFilePath);
        QDesktopServices::openUrl(QUrl("file:///"+f.absolutePath(),QUrl::TolerantMode));
        btnDlg->accept();
        qApp->processEvents();
        btnDlg->deleteLater();
    });
    //QMessageBox::RejectRole - fix to enable close button
    btnDlg->addButton(openFileButton,QMessageBox::RejectRole);
    btnDlg->addButton(openFolderButton,QMessageBox::ActionRole);
    btnDlg->setModal(true);
    btnDlg->exec();
}
void DocsHandler::docTranslateFailed()
{
    docTransWaitDlg->reject();
    QMessageBox box;
    box.critical(this,tr("Document has not been translated"),tr("An error occured during the translation of this file."));
}
