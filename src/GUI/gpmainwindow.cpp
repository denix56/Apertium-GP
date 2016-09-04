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
#include <set>

#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QByteArray>
#include <QTextBlock>
#include <QPixmap>
#include <QMenu>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QLocale>
#include <QDir>
#include <QTableView>
#include <QScrollBar>
#include <QMessageBox>
#include <QProgressDialog>
#include <QFileInfoList>
#include <QFileInfo>
#include <QStyle>
#include <QFileDialog>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QDebug>

#include "translator.h"
#include "docshandler.h"
#include "tablecombobox.h"
#include "downloadwindow.h"
#include "settingsdialog.h"

#include "gpmainwindow.h"
#include "ui_gpmainwindow.h"

//TODO: choose language of app
GpMainWindow::GpMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::GpMainWindow)
{
    ui->setupUi(this);
    ui->horizontalSpacer_2->changeSize(36,30);
}

struct GpMainWindow::langpairUsed
{
    QString name;
    unsigned long long n;
    langpairUsed(QString _name, unsigned long long _n)
    {
        name = _name;
        n = _n;
    }
    bool operator <(const langpairUsed &op) const
    {
        //reverse
        return n > op.n;
    }
};

bool GpMainWindow::initialize()
{
    setAttribute(Qt::WA_AlwaysShowToolTips);
    setContextMenuPolicy(Qt::NoContextMenu);
#ifdef Q_OS_LINUX
    auto dlg = new QProgressDialog(tr("Starting server..."),tr("Cancel"),0,0,this);
    dlg->setValue(0);
    dlg->show();
#endif
    appdata  = new QDir(DATALOCATION);

    //initialize language selection buttons
    SourceLangBtns.resize(3);
    TargetLangBtns.resize(3);
    SourceLangBtns[0]=ui->SourceLang1;
    SourceLangBtns[1]=ui->SourceLang2;
    SourceLangBtns[2]=ui->SourceLang3;

    TargetLangBtns[0]=ui->TargetLang1;
    TargetLangBtns[1]=ui->TargetLang2;
    TargetLangBtns[2]=ui->TargetLang3;

    //when button clicked, uncheck other buttons
    for(HeadButton *btn: SourceLangBtns)
        connect(btn,&HeadButton::clicked,this,&GpMainWindow::clearOtherSButtons);

    for(HeadButton *btn: TargetLangBtns)
        connect(btn,&HeadButton::clicked,this,&GpMainWindow::clearOtherEButtons);

    SourceLangBtns[0]->setChecked(true);
    TargetLangBtns[0]->setChecked(true);

    trayWidget = new TrayWidget(this);

    trayIcon = new QSystemTrayIcon(QIcon(":/images/Apertium_box_white_small.png"),this);
    trayIcon->setToolTip(tr("Apertium-GP"));
    auto trayMenu = new QMenu(this);
    showFullAction = new QAction(this);
    if (!this->isVisible())
        showFullAction->setText(tr("Hide main window"));
    else
        showFullAction->setText(tr("Show main window"));
    connect(showFullAction,&QAction::triggered,[&]()
    {
        if (!this->isVisible()) {
            showFullAction->setText(tr("Hide main window"));
            show();
        } else {
            showFullAction->setText(tr("Show main window"));
            close();
        }
    });
    trayMenu->addAction(showFullAction);

    exitAction = new QAction(tr("Exit"),this);
    connect(exitAction, &QAction::triggered, qApp, &QApplication::quit);
    trayMenu->addAction(exitAction);
    trayIcon->setContextMenu(trayMenu);
    ui->SourceLangComboBox->setFixedHeight(ui->SourceLang1->height()-2);
    ui->TargetLangComboBox->setFixedHeight(ui->SourceLang1->height()-2);
    ui->boxInput->setTextColor(Qt::black);
    ui->boxOutput->setTextColor(Qt::black);

    //translate in the other thread
    translator = new Translator(this);
    translator->moveToThread(&thread);

    ui->swapBtn->setFixedHeight(ui->SourceLang1->height());

#ifdef Q_OS_LINUX
    //start server and get available language pairs
    requestSender = new QNetworkAccessManager(this);
    QNetworkRequest request;
    apy = new QProcess;
    if(!QFile("/usr/share/apertium-apy/servlet.py").exists()
            && !QFile("/usr/share/apertium-gp/apertium-apy/apertium-apy/servlet.py").exists()) {
        QMessageBox box;
        box.critical(this,tr("Path error"),tr("Incorrect server directory"));
        close();
        return false;
    }
    serverStartedExitCode = apy->execute("pkexec", QStringList() << scriptPath << "--start");
    if(serverStartedExitCode) {
        if (serverStartedExitCode == 2) {
            apy->start(SERVERSTARTCMD);
            apy->waitForStarted();
        }
        else
            return false;
    }
    //wait while server starts
    while(true) {
        QEventLoop loop;
        auto reply = requestSender->get(QNetworkRequest(URL+"/stats"));
        connect(reply, &QNetworkReply::finished,&loop, &QEventLoop::quit);
        loop.exec();
        if (reply->error()== QNetworkReply::NoError) {
            reply->deleteLater();
            break;
        }
        reply->deleteLater();
    }

    auto dlAction =  new QAction(style()->standardIcon(QStyle::SP_ArrowDown),
                                     tr("Install packages"),ui->toolBar);
    ui->toolBar->addAction(dlAction);
    //Installing new packages
    connect(dlAction, &QAction::triggered,this,&GpMainWindow::dlAction_triggered);

    request.setUrl(URL+"/listPairs");
    QEventLoop loop;
    auto reply = requestSender->get(request);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    createListOfLangs(reply);
    if (!initRes) {
        dlg->accept();
        dlAction->trigger();
    }

    connect(requestSender,&QNetworkAccessManager::finished,this,&GpMainWindow::getReplyFromAPY);
    connect(ui->boxInput,&InputTextEdit::printEnded,[&](){this->createRequests();});
    connect (trayWidget,&TrayWidget::prindEnded,this,&GpMainWindow::createRequests);
    dlg->accept();
    dlg->deleteLater();
    reply->deleteLater();  

#else
    checked = true;
    auto dlAction= new QAction(style()->standardIcon(QStyle::SP_ArrowDown),tr("Install packages"),ui->toolBar);
    ui->toolBar->addAction(dlAction);
    connect(dlAction, &QAction::triggered,this, &GpMainWindow::dlAction_triggered);
    connect(ui->boxInput,&InputTextEdit::printEnded,translator,&Translator::boxTranslate);   
    connect(ui->boxInput,&InputTextEdit::printEnded,this,&GpMainWindow::saveMru);
    connect(translator,&Translator::resultReady,this,&GpMainWindow::translateReceived);

    connect(trayWidget, &TrayWidget::prindEnded, translator, &Translator::winTrayTranslate);
    connect(translator, &Translator::trayResultReady, trayWidget, &TrayWidget::translationReceived);
    if (!QDir(DATALOCATION+"/usr/share/apertium/modes").exists() || !QDir(DATALOCATION+"/apertium-all-dev").exists()) {
        QMessageBox box;
        if(box.critical(this, "Required packages are not installed.",
                        "The program cannot find required core tools and/or even one language pair installed. "
                        "Please, install them.",QMessageBox::Ok, QMessageBox::Cancel)==QMessageBox::Ok)
            dlAction->trigger();
        else
            return false;
    } else {
        initRes = true;
        createListOfLangs();
    }
#endif  
    thread.start();

    connect(ui->SourceLangComboBox->view(), &QTableView::activated, this, &GpMainWindow::updateComboBox);
    connect(ui->SourceLangComboBox->view(), &QTableView::clicked, this, &GpMainWindow::updateComboBox);

    connect(ui->TargetLangComboBox->view(), &QTableView::activated, this, &GpMainWindow::updateEndComboBox);
    connect(ui->TargetLangComboBox->view(), &QTableView::clicked, this, &GpMainWindow::updateEndComboBox);

    connect(ui->actionExit, &QAction::triggered, this, &GpMainWindow::close);

    connect(trayWidget->inputComboBox(),&QComboBox::currentTextChanged,[&](QString text)
    {
        if(!text.isEmpty()) {
#ifdef Q_OS_LINUX
            if (trayWidget->inputComboBox()->findText(IDLANGTEXT)==-1) {
                trayWidget->inputComboBox()->blockSignals(true);
                trayWidget->inputComboBox()->removeItem(trayWidget->inputComboBox()->findText(
                                                            IDLANGTEXT, Qt::MatchStartsWith | Qt::MatchCaseSensitive));
                trayWidget->inputComboBox()->addItem(IDLANGTEXT);
                trayWidget->inputComboBox()->setCurrentText(text);
                trayWidget->inputComboBox()->blockSignals(false);
            }
#endif
            setLangpair(text);
        }
    });
    connect(trayWidget->outputComboBox(),&QComboBox::currentTextChanged,[&](QString text)
    {
        if(!text.isEmpty())
            setLangpair("",text);
    });
    connect(ui->actionOptions, &QAction::triggered,[&]()
    {
        SettingsDialog dlg(this);
        dlg.exec();
        loadConf();
    });

    //read settings
    loadConf();

    return initRes;
}

int GpMainWindow::getFontSize() const
{
    return ui->boxInput->fontInfo().pointSize();
}

QString GpMainWindow::getText() const
{
    return ui->boxInput->toPlainText();
}

void GpMainWindow::setTrayWidgetEnabled(bool b)
{
    trayWidget->setVisible(b);
    trayIcon->setVisible(b);
    qApp->setQuitOnLastWindowClosed(!b);
}

GpMainWindow::~GpMainWindow()
{
#ifdef Q_OS_LINUX
    QProcess cmd;
    if (!serverStartedExitCode)
        cmd.execute("pkexec", QStringList() << scriptPath << "--stop");
    else
        if (serverStartedExitCode==2)
            apy->kill();
#endif
    thread.quit();
    thread.wait();
    delete Initializer::conf;
    delete apy;
    delete ui;
}

void GpMainWindow::resizeEvent(QResizeEvent *e)
{
    ui->label->setPixmap(ui->label->pixmap()->
                         scaled(ui->label->pixmap()->width(),
                                ui->label->pixmap()->height(),Qt::KeepAspectRatio));
    ui->boxOutput->setMinimumWidth(ui->boxInput->width());
    QMainWindow::resizeEvent(e);
}

void GpMainWindow::closeEvent(QCloseEvent *event)
{
    emit showFullAction->triggered();
    QMainWindow::closeEvent(event);
}

void GpMainWindow::setFontSize(int size)
{
    QFont font(ui->boxInput->font());
    font.setPointSize(size);
    ui->boxInput->setFont(font);
    ui->boxOutput->setFont(font);
    Initializer::conf->setValue(FONTSIZE,size);
}

//get available language pairs
void GpMainWindow::createListOfLangs(QNetworkReply *reply)
{
    std::multiset <langpairUsed> langs;
    ui->SourceLangComboBox->model()->clear();
    ui->TargetLangComboBox->model()->clear();
    ui->mru->clear();
    for (auto b : SourceLangBtns)
        b->setText("");

    for (auto b : TargetLangBtns)
        b->setText("");

#ifdef Q_OS_LINUX
    if (reply->error() == QNetworkReply::NoError) {
        // read server response
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonArray array = doc.object().value("responseData").toArray();
        if (array.isEmpty()) {
            QMessageBox box;
            box.setModal(true);
            box.critical(this,tr("No langpairs found."),
                         tr("It seems, that no language pairs have been found."),
                         QMessageBox::Ok);
            initRes = false;
            return;
        }
        //parse json response

        //delete non 2-letter names
        for(auto it = array.begin(); it != array.end();) {
            QString sourceLanguage = (*it).toObject().value("sourceLanguage").toString();
            QString targetLanguage = (*it).toObject().value("targetLanguage").toString();
            if(sourceLanguage.length() > 3 || targetLanguage.length() > 3)
                it = array.erase(it);
            else
                ++it;
        }
        trayWidget->inputComboBox()->blockSignals(true);
        for (auto it : array) {
            bool unique = true;
            auto sourceLang = it.toObject().value("sourceLanguage").toString();
            auto targetLang = it.toObject().value("targetLanguage").toString();
            if (Initializer::conf->value(sourceLang + "-" + targetLang).toULongLong()>0ULL) {
                langs.insert(langpairUsed(Initializer::langNamesMap[sourceLang]+
                                          " - " + Initializer::langNamesMap[targetLang],
                                          Initializer::conf->value(sourceLang +
                                                                   "-" + targetLang).toULongLong()));
            }
            //unique languages
            for(int j=0;j< ui->SourceLangComboBox->model()->columnCount();++j)
                for(int i=0; i < ui->SourceLangComboBox->model()->rowCount();++i)
                    if(ui->SourceLangComboBox->model()->
                            data(ui->SourceLangComboBox->model()->index(i,j))
                            .toString()==Initializer::langNamesMap[sourceLang])
                        unique=false;
            if(unique) {
                ui->SourceLangComboBox->model()->addItem(Initializer::langNamesMap[sourceLang]);
                trayWidget->inputComboBox()->addItem(Initializer::langNamesMap[sourceLang]);
            }
        }
        int i = 0;
        //add to mru
        for(auto it = langs.begin(); i < 3 && it != langs.end();++it, ++i) {
            auto item = new QListWidgetItem(it->name);
            item->setTextAlignment(Qt::AlignCenter);
            ui->mru->addItem(item);
        }

        ui->SourceLangComboBox->model()->addItem(IDLANGTEXT);
        trayWidget->inputComboBox()->addItem(IDLANGTEXT);
        trayWidget->inputComboBox()->blockSignals(false);
    }
    // Error occured
    else {
        QMessageBox box;
        box.critical(this,"Server error",reply->errorString());
        initRes = false;
        return;
    }
#else
    if (!QDir(DATALOCATION+"/usr/share/apertium/modes").exists() || !QDir(DATALOCATION+"/apertium-all-dev").exists()) {
        QMessageBox box;
        box.critical(this,tr("No installed langpairs."), tr("You have not installed any langpairs. The application will be closed."));
        initRes = false;
        return;
    }
    //get list of availiable languages
    QDir moded(appdata->absoluteFilePath("usr/share/apertium/modes"));
    auto modes = moded.entryInfoList(QStringList() << "*.mode");

    for (auto mode : modes) {
        bool unique = true;
        auto sourceLanguage = mode.baseName().left(mode.baseName().indexOf("-"));
        auto targetLanguage = mode.baseName().mid(mode.baseName().indexOf("-")+1);
        if (sourceLanguage.length() > 3 || targetLanguage.length() > 3)
            continue;
        if (Initializer::conf->value(sourceLanguage + "-" + targetLanguage).toULongLong()>0ULL) {
            langs.insert(langpairUsed(Initializer::langNamesMap[sourceLanguage]+ " - "
                                      + Initializer::langNamesMap[targetLanguage],
                                      Initializer::conf->value(sourceLanguage + "-" + targetLanguage).toULongLong()));
        }

        for(int j=0;j< ui->SourceLangComboBox->model()->columnCount();++j)
            for(int i=0; i < ui->SourceLangComboBox->model()->rowCount();++i)
                if(ui->SourceLangComboBox->model()->data(ui->SourceLangComboBox->model()->index(i,j)).toString()
                        ==Initializer::langNamesMap[sourceLanguage])
                    unique=false;
        if(unique) {
            ui->SourceLangComboBox->model()->addItem(Initializer::langNamesMap[sourceLanguage]);
            trayWidget->inputComboBox()->addItem(Initializer::langNamesMap[sourceLanguage]);
        }
    }

    int i=0;
    for(auto it = langs.begin(); i < 3 && it != langs.end();++it, ++i) {
        auto item = new QListWidgetItem(it->name);
        item->setTextAlignment(Qt::AlignCenter);
        ui->mru->addItem(item);
    }
#endif

    //fill buttons with start languages
    for(int i=0;i<SourceLangBtns.size();++i) {
        SourceLangBtns[i]->setText(ui->SourceLangComboBox->model()->
                                   data(ui->SourceLangComboBox->model()->index(i,0)).toString());
        SourceLangBtns[i]->setEnabled(!SourceLangBtns[i]->text().isEmpty());
    }

    //remove items, that are showed in buttons
    for(int i=0;i<SourceLangBtns.size();++i)
        ui->SourceLangComboBox->model()->removeItem(0,0);

#ifdef Q_OS_LINUX
    if (SourceLangBtns[0]->text().contains(IDLANGTEXT)) {
        SourceLangBtns[0]->setText(SourceLangBtns[2]->text());
        SourceLangBtns[2]->setText(IDLANGTEXT);
    }
#endif
    ui->SourceLangComboBox->setCurrentIndex(-1);
    ui->TargetLangComboBox->setCurrentIndex(-1);
    emit SourceLangBtns[0]->clicked();

#ifdef Q_OS_LINUX
    //get iso3 names of current languages
    QString name = "";
    for (auto tmp : Initializer::langNamesMap.keys(SourceLangBtns[0]->text()))
        if(tmp.length()>name.length())
            name = tmp;
    currentSourceLang = name;
    name = "";
    for (auto tmp : Initializer::langNamesMap.keys(TargetLangBtns[0]->text()))
        if(tmp.length()>name.length())
            name = tmp;
    currentTargetLang = name;
    reply->deleteLater();

#else
    QDir path(DATALOCATION+"/usr/share/apertium/modes");
    for (auto key : Initializer::langNamesMap.keys(SourceLangBtns[0]->text()))
        for (auto tKey : Initializer::langNamesMap.keys(TargetLangBtns[0]->text()))
        if (!path.entryList(QStringList() << key+"-"+tKey+".mode").isEmpty()) {
            currentSourceLang = key;
            currentTargetLang = tKey;
            break;
        }
#endif
    emit listOfLangsSet();
    initRes = true;
}

//update ComboBoxes when new source language, that is choosed
void GpMainWindow::updateComboBox(QModelIndex index)
{
    auto curr = index.data().toString();
    if (curr.isEmpty())
        return;
    //TODO: make replace the third button with the sourceLangComboBox
    ui->SourceLangComboBox->model()->removeItem(index.row(),index.column());
#ifdef Q_OS_LINUX
    if (SourceLangBtns[2]->text().contains(IDLANGTEXT))
        ui->SourceLangComboBox->model()->addItem(IDLANGTEXT);
    else
#endif
        ui->SourceLangComboBox->model()->addItem(SourceLangBtns[2]->text());
    SourceLangBtns[2]->setText(SourceLangBtns[1]->text());
    SourceLangBtns[1]->setText(SourceLangBtns[0]->text());
    SourceLangBtns[0]->setText(curr);
    ui->TargetLangComboBox->model()->clear();
    trayWidget->outputComboBox()->clear();

#ifdef Q_OS_LINUX
    if (curr != IDLANGTEXT) {
    QNetworkRequest request(URL+"/listPairs");
    QEventLoop loop;
    QNetworkAccessManager tmpN(this);
    if (tmpN.networkAccessible() != QNetworkAccessManager::Accessible) {
        QMessageBox::critical(this, tr("Network Inaccessible"),
                              tr("Can't check for updates as you appear to be offline."));
        return;
    }
    connect(&tmpN,&QNetworkAccessManager::finished,&loop, &QEventLoop::quit);
    auto reply = tmpN.get(request);
    loop.exec();
    auto doc = QJsonDocument::fromJson(reply->readAll());
    auto array = doc.object().value("responseData").toArray();
    for(auto it = array.begin(); it != array.end();) {
        QString sourceLanguage = (*it).toObject().value("sourceLanguage").toString();
        if (sourceLanguage.length() > 3)
            it = array.erase(it);
        else
            ++it;
    }
    trayWidget->outputComboBox()->blockSignals(true);
    for (auto it : array)
        if(Initializer::langNamesMap[it.toObject().value("sourceLanguage").toString()]
                ==SourceLangBtns[0]->text()) {
            ui->TargetLangComboBox->model()->addItem(
                        Initializer::langNamesMap[it.toObject().value("targetLanguage").toString()]);
            trayWidget->outputComboBox()->addItem(
                        Initializer::langNamesMap[it.toObject().value("targetLanguage").toString()]);
        }
    trayWidget->outputComboBox()->blockSignals(false);
    reply->deleteLater();
    }
#else
    QDir moded(appdata->absoluteFilePath("usr/share/apertium/modes"));
    auto modes = moded.entryInfoList(QStringList() << "*.mode");

    for (auto mode : modes) {
        auto sourceLang = mode.baseName().left(mode.baseName().indexOf("-"));
        auto targetLang = mode.baseName().mid(mode.baseName().indexOf("-")+1);
        if (!Initializer::langNamesMap.contains(sourceLang))

            if(Initializer::langNamesMap[sourceLang]==SourceLangBtns[0]->text())
                ui->TargetLangComboBox->model()->addItem(Initializer::langNamesMap[targetLang]);
    }
#endif
    ui->SourceLangComboBox->setCurrentIndex(-1);
    emit SourceLangBtns[0]->clicked(true);
}

//update ComboBox with Tatget Languages when the new one is choosed
void GpMainWindow::updateEndComboBox(QModelIndex index)
{
    auto curr = index.data().toString();
    if (curr.isEmpty())
        return;
    ui->TargetLangComboBox->model()->removeItem(index.row(),index.column());
    ui->TargetLangComboBox->model()->addItem(TargetLangBtns[2]->text());
    TargetLangBtns[2]->setText(TargetLangBtns[1]->text());
    TargetLangBtns[1]->setText(TargetLangBtns[0]->text());
    TargetLangBtns[0]->setText(curr);
    ui->TargetLangComboBox->setCurrentIndex(-1);
}


//Uncheck Other Source language buttons when the new one is checked
void GpMainWindow::clearOtherSButtons()
{
    ui->boxOutput->clear();
    currentSButton = qobject_cast<HeadButton*>(sender());
    for (HeadButton *btn : SourceLangBtns) {
        if (btn->text().isEmpty())
            btn->setEnabled(false);
        else {
            btn->setEnabled(true);
            if(btn!=currentSButton) {
                btn->setChecked(false);
                if (btn->text().contains(IDLANGTEXT))
                    btn->setText(IDLANGTEXT);
            }
            else {
                trayWidget->inputComboBox()->blockSignals(true);
                trayWidget->inputComboBox()->setCurrentText(btn->text());
                trayWidget->inputComboBox()->blockSignals(false);
                #ifdef Q_OS_LINUX
                btn->setChecked(true);
                if (btn->text()==IDLANGTEXT) {
                    currentSourceLang = IDLANGTEXT;
                    for (HeadButton *btn : TargetLangBtns) {
                        btn->setText("");
                        btn->setEnabled(false);
                    }
                }
                else {
                    QString name = "";
                    for (auto tmp : Initializer::langNamesMap.keys(btn->text()))
                        if(tmp.length()>name.length())
                            name = tmp;
                    currentSourceLang = name;
                }
                #endif
            }
        }
    }
#ifdef Q_OS_LINUX
    auto requestSenderTmp = new QNetworkAccessManager;
    QNetworkRequest request(URL+"/listPairs");
    connect(requestSenderTmp,&QNetworkAccessManager::finished,this,
            &GpMainWindow::getResponseOfAvailLang);
    requestSenderTmp->get(request);
#else
    QDir moded(appdata->absoluteFilePath("usr/share/apertium/modes"));
    auto modes = moded.entryInfoList(QStringList() << "*.mode");
    ui->TargetLangComboBox->model()->clear();
    trayWidget->outputComboBox()->clear();
    trayWidget->outputComboBox()->blockSignals(true);
    for (auto mode : modes) {
        if (QRegExp("^[a-z]+-[a-z]+$").exactMatch(mode.baseName())
                && Initializer::langNamesMap[mode.baseName().
                left(mode.baseName().indexOf("-"))]==currentSButton->text()) {
            ui->TargetLangComboBox->model()->
                    addItem(Initializer::langNamesMap[mode.baseName()
                    .mid(mode.baseName().indexOf("-")+1)]);
            trayWidget->outputComboBox()->addItem(Initializer::langNamesMap[mode.baseName()
                    .mid(mode.baseName().indexOf("-")+1)]);
        }
    }
    trayWidget->outputComboBox()->blockSignals(false);
    for(int i=0;i<TargetLangBtns.size();++i) {
        TargetLangBtns[i]->setText(ui->TargetLangComboBox->model()->
                                   data(ui->TargetLangComboBox->model()->index(i,0)).toString());

    }
    for(int i=0;i<TargetLangBtns.size();++i)
        ui->TargetLangComboBox->model()->removeItem(0,0);
    TargetLangBtns[0]->setEnabled(true);
    TargetLangBtns[0]->click();
    emit listOfLangsSet();
#endif
}

//Uncheck Other Target language buttons when the new one is checked
void GpMainWindow::clearOtherEButtons()
{
    auto currentButton = qobject_cast<HeadButton*>(sender());
    for (HeadButton *btn : TargetLangBtns) {
        if (btn->text().isEmpty()) {
            btn->setEnabled(false);
            btn->setChecked(false);
        }
        else {
            btn->setEnabled(true);
            if(btn!=currentButton)
                btn->setChecked(false);
            else {
                trayWidget->outputComboBox()->setCurrentText(btn->text());
#ifdef Q_OS_LINUX
                btn->setChecked(true);
                QString name = "";
                for (auto tmp : Initializer::langNamesMap.keys(btn->text()))
                    if(tmp.length()>name.length())
                        name = tmp;
                currentTargetLang = name;
#else
            QDir path(DATALOCATION+"/usr/share/apertium/modes");
            HeadButton *currentSourceButton;
            for (HeadButton *btn : SourceLangBtns)
                if (btn->isChecked())
                    currentSourceButton = btn;
            for (auto key : Initializer::langNamesMap.keys(currentSourceButton->text()))
                for (auto tKey : Initializer::langNamesMap.keys(TargetLangBtns[0]->text()))
                if (!path.entryList(QStringList() << key+"-"+tKey+".mode").isEmpty()) {
                    currentSourceLang = key;
                    currentTargetLang = tKey;
                    break;
                }
#endif
            }
        }
    }
}

//update available Target languages
void GpMainWindow::getResponseOfAvailLang(QNetworkReply *reply)
{
    auto docAvailLang = QJsonDocument::fromJson(reply->readAll());
    auto array = docAvailLang.object().value("responseData").toArray();
    for(auto it = array.begin(); it != array.end();) {
        QString sourceLanguage = (*it).toObject().value("sourceLanguage").toString();
        QString targetLanguage = (*it).toObject().value("targetLanguage").toString();
        if(sourceLanguage.length() > 3 || targetLanguage.length() > 3)
            it = array.erase(it);
        else
            ++it;
    }
    ui->TargetLangComboBox->model()->clear();
    //TODO: make trayWidget standalone
    trayWidget->outputComboBox()->clear();
    trayWidget->outputComboBox()->blockSignals(true);
    for (auto it : array)
        if(Initializer::langNamesMap[it.toObject().
                value("sourceLanguage").toString()]==Initializer::langNamesMap[currentSourceLang]) {
            ui->TargetLangComboBox->model()->addItem(
                        Initializer::langNamesMap[it.toObject().value("targetLanguage").toString()]);
            trayWidget->outputComboBox()->addItem(
                        Initializer::langNamesMap[it.toObject().value("targetLanguage").toString()]);
        }
    trayWidget->outputComboBox()->blockSignals(false);

    //fill buttons with new target languages
    for(int i=0;i<TargetLangBtns.size();++i)
        TargetLangBtns[i]->setText(ui->TargetLangComboBox->model()
                                   ->data(ui->TargetLangComboBox->model()->index(i,0)).toString());
    for(int i=0;i<TargetLangBtns.size();++i)
        ui->TargetLangComboBox->model()->removeItem(0,0);
    emit TargetLangBtns[0]->clicked(true);
    emit listOfLangsSet();
    reply->deleteLater();
}

//send translation request for paragraph
void GpMainWindow::createRequests(QString text)
{
    if (currentSButton->text().contains(IDLANGTEXT)) {

        const QString mode = "/identifyLang?";
        QUrlQuery urlQ;
        //only first paragraph
        if (text.isEmpty())
            urlQ.addQueryItem("q", QUrl::toPercentEncoding(ui->boxInput->toPlainText().split("\n")[0]));
        else
            urlQ.addQueryItem("q", QUrl::toPercentEncoding(text));
        QUrl u(URL+mode);
        u.setQuery(urlQ);
        QNetworkRequest request(u);
        translator->linuxTranslate(request);
    }
    QNetworkRequest request;
    const QString mode = "/translate?";
    QUrlQuery urlQ;
    urlQ.addQueryItem("langpair", currentSourceLang+"|"+currentTargetLang);
    if (text.isEmpty() && lastBlockCount!=ui->boxInput->document()->blockCount()) {
        ui->boxOutput->clear();
        outputDoc.clear();
        QUrl u(URL+mode);
        for(auto paragraph : ui->boxInput->toPlainText().split("\n")) {
            urlQ.addQueryItem("q",QUrl::toPercentEncoding(paragraph));
            u.setQuery(urlQ);
            request.setUrl(u);
            request.setRawHeader("whole","yes");
            translator->linuxTranslate(request);
            urlQ.removeQueryItem("q");
        }

        ui->boxOutput->verticalScrollBar()->setValue(ui->boxInput->verticalScrollBar()->value());
    }
    else {
        if (text.isEmpty()) {
            auto cursor = ui->boxInput->textCursor();
            urlQ.addQueryItem("q",QUrl::toPercentEncoding(cursor.block().text()));
            request.setRawHeader("blockNumber",
                                 QByteArray::number(cursor.block().blockNumber()));
        }
        else {
            urlQ.addQueryItem("q",QUrl::toPercentEncoding(text));
            request.setRawHeader("tray","yes");
        }
        QUrl u(URL+mode);
        u.setQuery(urlQ);
        request.setUrl(u);
        request.setRawHeader("whole","no");
        translator->linuxTranslate(request);

    }
    lastBlockCount = ui->boxInput->document()->blockCount();
    auto langpair = currentSourceLang+"-"+currentTargetLang;
    Initializer::conf->setValue(langpair, QVariant(Initializer::conf->value
                                                   (langpair,QVariant(0ULL)).toULongLong() + 1ULL));
}

//parse json response
void GpMainWindow::getReplyFromAPY(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        auto doc = QJsonDocument::fromJson(reply->readAll());
        if (reply->request().url().toDisplayString().contains(QRegExp("^http://localhost:2737/identify"))) {
            auto list = doc.object();
            //choose the most likely language
            double max=0;
            for(auto value : list)
                if (value.toDouble()>max)
                    max = value.toDouble();

            QString tmp = list.toVariantMap().key(max);
            if (tmp==currentSourceLang)
                return;
            currentSourceLang = tmp;
            for (HeadButton *btn : SourceLangBtns)
                if (btn->isChecked()) {
                    btn->setText(IDLANGTEXT+" ("+Initializer::langNamesMap[currentSourceLang]+")");
                    trayWidget->inputComboBox()->blockSignals(true);
                    trayWidget->inputComboBox()->removeItem(trayWidget->inputComboBox()->findText(
                                                                IDLANGTEXT,Qt::MatchStartsWith | Qt::MatchCaseSensitive));
                    trayWidget->inputComboBox()->addItem(IDLANGTEXT+" ("+Initializer::langNamesMap[currentSourceLang]+")");
                    trayWidget->inputComboBox()->setCurrentText(IDLANGTEXT+" ("+Initializer::langNamesMap[currentSourceLang]+")");
                    trayWidget->inputComboBox()->blockSignals(false);
                    break;
                }
            auto requestSenderTmp = new QNetworkAccessManager;
            QNetworkRequest request(URL+"/listPairs");
            QEventLoop loop;
            connect(requestSenderTmp,&QNetworkAccessManager::finished,this,
                    &GpMainWindow::getResponseOfAvailLang);
            connect(requestSenderTmp,&QNetworkAccessManager::finished,&loop,&QEventLoop::quit);
            requestSenderTmp->get(request);
            loop.exec();
            return;
        }
        if (reply->request().rawHeader("whole")=="no") {
            if (reply->request().rawHeader("tray")=="yes")
                trayWidget->translationReceived(doc.object().value("responseData")
                                                .toObject().value("translatedText").toString());
            else {
                auto cursor = ui->boxOutput->textCursor();
                int blockNumber = reply->request().rawHeader("blockNumber").toInt();
                if(blockNumber>=ui->boxOutput->document()->blockCount()) {
                    cursor.movePosition(QTextCursor::End);
                    cursor.insertBlock();
                }
                cursor.setPosition(ui->boxOutput->
                                   document()->findBlockByNumber(blockNumber).position());
                cursor.movePosition(QTextCursor::EndOfBlock,QTextCursor::KeepAnchor);
                auto format = cursor.charFormat();
                format.setForeground(Qt::black);
                cursor.insertText(doc.object().value("responseData")
                                  .toObject().value("translatedText").toString(),format);

                cursor.movePosition(QTextCursor::StartOfBlock);
                ui->boxOutput->verticalScrollBar()->setValue(ui->boxInput->verticalScrollBar()->value());
                while(cursor.blockNumber()==blockNumber && !cursor.atBlockEnd()) {

                    auto cursor1 = cursor.document()->
                            find(QRegExp ("\\*\\w+\\W?"),cursor);
                    if (cursor1.isNull())
                        break;
                    cursor = cursor1;
                    auto format = cursor.charFormat();
                    format.setForeground(Qt::red);
                    cursor.insertText(cursor.selectedText().mid(1),format);
                }
                ui->boxOutput->setTextCursor(cursor);
            }
        }
        else {
            QTextCursor cursor = ui->boxOutput->textCursor();
            cursor.movePosition(QTextCursor::End);
            auto format = cursor.charFormat();
            format.setForeground(Qt::black);
            cursor.insertText(doc.object().value("responseData")
                              .toObject().value("translatedText").toString(),format);
            cursor.movePosition(QTextCursor::StartOfBlock);
            while(!cursor.atBlockEnd()) {
                auto cursor1 = cursor.document()->
                        find(QRegExp("\\*\\w+\\W?"),cursor);
                if (cursor1.isNull())
                    break;
                cursor = cursor1;
                auto format = cursor.charFormat();
                format.setForeground(Qt::red);
                cursor.insertText(cursor.selectedText().mid(1),format);
            }
            cursor.movePosition(QTextCursor::End);
            cursor.insertBlock();
            ui->boxOutput->setTextCursor(cursor);
        }
    }
    else
        qDebug() << reply->errorString();
    reply->deleteLater();
}

void GpMainWindow::loadConf()
{
    //Save entered pathes to Server and langpairs
    QFont font(ui->boxInput->font());
    if (!Initializer::conf->contains(FONTSIZE))
        Initializer::conf->setValue(FONTSIZE,QVariant(14));
    font.setPointSize(Initializer::conf->value(FONTSIZE).toInt());
    ui->boxInput->setFont(font);
    ui->boxOutput->setFont(font);

    if (!Initializer::conf->contains("extra/traywidget/position"))
        Initializer::conf->setValue("extra/traywidget/position", QVariant::fromValue(BottomRight));
    setTrayWidgetPosition(static_cast<Position>(Initializer::conf->value("extra/traywidget/position").toUInt()));
    if (!Initializer::conf->contains("extra/traywidget/enabled"))
        Initializer::conf->setValue("extra/traywidget/enabled", false);
    setTrayWidgetEnabled(Initializer::conf->value("extra/traywidget/enabled").toBool());

}

void GpMainWindow::saveMru()
{
    if (currentSourceLang==IDLANGTEXT)
        return;
    auto langpair = currentSourceLang+"-"+currentTargetLang;
    Initializer::conf->setValue(langpair, QVariant(Initializer::conf->value
                                                   (langpair,QVariant(0ULL)).toULongLong() + 1ULL));
}

void GpMainWindow::translateReceived(const QString &result)
{
    ui->boxOutput->clear();
    auto cursor = ui->boxOutput->textCursor();
    cursor.movePosition(QTextCursor::Start);
    auto format = cursor.charFormat();
    format.setForeground(Qt::black);
    cursor.insertText(result, format);
    cursor.movePosition(QTextCursor::Start);
    while(!cursor.atEnd()) {
        auto cursor1 = cursor.document()->
                find(QRegExp("[\\*#]\\w+\\W?"),cursor);
        if (cursor1.isNull())
            break;
        cursor = cursor1;
        auto format = cursor.charFormat();
        format.setForeground(Qt::red);
        cursor.insertText(cursor.selectedText().mid(1),format);
    }
    ui->boxOutput->setTextCursor(cursor);
    ui->boxOutput->verticalScrollBar()->setValue(ui->boxInput->verticalScrollBar()->value());
}

void GpMainWindow::on_boxInput_currentCharFormatChanged(const QTextCharFormat &)
{
    ui->boxInput->setTextColor(Qt::black);
}

void GpMainWindow::dlAction_triggered()
{
    DownloadWindow dlWindow(this);
    setDisabled(true);
    if (dlWindow.getData(checked)) {
        setDisabled(false);
#ifdef Q_OS_LINUX
        checked = true;
        dlWindow.exec();
        if (serverStartedExitCode == 2) {
            apy->kill();
            apy->start(SERVERSTARTCMD);
        }
        //wait while server starts
        while(true) {
            QEventLoop loop;
            auto reply = requestSender->get(QNetworkRequest(URL+"/stats"));
            connect(reply, &QNetworkReply::finished,&loop, &QEventLoop::quit);
            loop.exec();
            if (reply->error()== QNetworkReply::NoError) {
                reply->deleteLater();
                break;
            }
            reply->deleteLater();
        }
        const QString mode = "/listPairs";
        QNetworkAccessManager tmp(this);
        auto reply = tmp.get(QNetworkRequest(URL+mode));
        QEventLoop loop;
        connect(reply, &QNetworkReply::finished,&loop, &QEventLoop::quit);
        loop.exec();
        createListOfLangs(reply);
    }
#else
        dlWindow.exec();
        createListOfLangs();
    }
    setDisabled(false);
#endif
}

void GpMainWindow::on_mru_itemClicked(QListWidgetItem *item)
{
    setLangpair(item->text().left(item->text().indexOf(' ')),
                item->text().mid(item->text().indexOf("- ")+2));
}

void GpMainWindow::on_swapBtn_clicked()
{
    setLangpair(Initializer::langNamesMap[currentTargetLang],
                Initializer::langNamesMap[currentSourceLang]);
}

void GpMainWindow::on_docTranslateBtn_clicked()
{
    auto DocTranslateWidget = new DocsHandler(this);
    ui->boxInput->setEnabled(false);
    ui->boxOutput->setEnabled(false);
    DocTranslateWidget->show();
    connect(DocTranslateWidget, &DocsHandler::destroyed, [&]() { ui->boxInput->setEnabled(true);});
    connect(DocTranslateWidget, &DocsHandler::destroyed, [&]() { ui->boxOutput->setEnabled(true);});
}

void GpMainWindow::setLangpair(QString source, QString target) {
    if (source.isEmpty() && target.isEmpty())
        qApp->exit(10);
    int sBtnIndex = -1;
    int tBtnIndex = -1;
    if (!source.isEmpty()) {
    for (int i = 0; i<SourceLangBtns.size();++i)
        if (SourceLangBtns[i]->text()==source) {
            sBtnIndex = i;
            break;
        }
    QEventLoop loop;
    connect(this, &GpMainWindow::listOfLangsSet,&loop, &QEventLoop::quit);
    if (sBtnIndex!=-1)
        emit SourceLangBtns[sBtnIndex]->click();
    else {
        QModelIndex index = ui->SourceLangComboBox->model()->
                findText(source);
        if (index.isValid())
            emit ui->SourceLangComboBox->view()->activated(index);
    }
    loop.exec();
    }
    if (target.isEmpty()) {
        TargetLangBtns[0]->click();
        return;
    }
    for (int i = 0; i<TargetLangBtns.size();++i) {
        if (TargetLangBtns[i]->text()==target) {
            tBtnIndex = i;
            break;
        }
    }
    if (tBtnIndex!=-1)
        TargetLangBtns[tBtnIndex]->click();
    else {
        QModelIndex index = ui->TargetLangComboBox->model()->
                findText(target);
        if (index.isValid())
            emit ui->TargetLangComboBox->view()->activated(index);
    }
}

 void GpMainWindow::setTrayWidgetPosition(Position position)
 {
     QRect desktop = qApp->desktop()->availableGeometry();
     QMap <Position, QRect> pos_coords {
         { TopLeft, QRect(desktop.left(), desktop.top(),trayWidget->width(), trayWidget->height()) },
         { TopRight, QRect(desktop.right()-trayWidget->width(),desktop.top(),
                                     trayWidget->width(), trayWidget->height()) },
         { BottomLeft, QRect(desktop.left(), desktop.bottom()-trayWidget->height(),
                                       trayWidget->width(), trayWidget->height()) },
         { BottomRight, QRect(desktop.right()-trayWidget->width(), desktop.bottom()-trayWidget->height(),
                                        trayWidget->width(), trayWidget->height()) },
     };
     trayWidget->setGeometry(pos_coords[position]);
 }
