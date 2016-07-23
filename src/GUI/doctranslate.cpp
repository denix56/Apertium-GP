#include "doctranslate.h"
#include "ui_doctranslate.h"
#include "translator.h"
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>
#include <QDesktopServices>
#include <QProgressDialog>
#include <QDebug>
const QStringList DocTranslate::fileTypes {"txt", "docx", "pptx", "html",
                                           "rtf", "odt", "xlsx", "xtg"};
DocTranslate::DocTranslate(ApertiumGui *parent) :
    parent(parent),
    ui(new Ui::DocTranslate)
{
    ui->setupUi(this);
    QPalette Pal(palette());
    // устанавливаем цвет фона
    Pal.setColor(QPalette::Background, Qt::white);
    ui->DropWidget->setPalette(Pal);
    setAttribute(Qt::WA_DeleteOnClose);

    connect(this, &DocTranslate::docForTransChoosed, parent->getTranslator(), &Translator::docTranslate);
    connect(ui->DropWidget, &DragnDropWidget::documentDropped, this, &DocTranslate::docForTransChoosed);
    connect(parent->getTranslator(),&Translator::docTranslated,this,&DocTranslate::showPostDocTransDlg);
}

DocTranslate::~DocTranslate()
{
    delete ui;
}

void DocTranslate::on_browseBtn_clicked()
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

void DocTranslate::showPostDocTransDlg(QString trFilePath)
{
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

