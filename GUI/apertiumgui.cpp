#include "apertiumgui.h"
#include "ui_apertiumgui.h"
#include "translator.h"
#include "tablecombobox.h"
#include "downloadwindow.h"
#include "initializer.h"
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
#include <set>
#include <QLocale>
#include <QDir>
#include <QTableView>
#include <QScrollBar>
#include <QMessageBox>
#include <QProgressDialog>
#include <QFileInfoList>
#include <QFileInfo>
#include <QStyle>
#include <QNetworkConfiguration>
#include <QDebug>

//TODO: choose language of app
ApertiumGui::ApertiumGui(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ApertiumGui)
{
    ui->setupUi(this);

}

bool ApertiumGui::initialize()
{
#ifdef Q_OS_LINUX
    auto dlg = new QProgressDialog(tr("Starting server..."),tr("Cancel"),0,100,this);
    dlg->setRange(0,0);
    dlg->setValue(0);
    dlg->show();
#endif
    appdata  = new QDir(DATALOCATION);
    //TODO: delete this line
    qDebug(DATALOCATION.toLatin1());
    setWindowTitle("Apertium-GP");
    //open file with Initializer::conf
    loadConf();
    //initialize language selection buttons
    SourceLangBtns.resize(3);
    TargetLangBtns.resize(3);
    SourceLangBtns[0]=ui->SourceLang1;
    SourceLangBtns[1]=ui->SourceLang2;
    SourceLangBtns[2]=ui->SourceLang3;

    TargetLangBtns[0]=ui->TargetLang1;
    TargetLangBtns[1]=ui->TargetLang2;
    TargetLangBtns[2]=ui->TargetLang3;

    for(int i=0;i<SourceLangBtns.size();++i)
    {
        connect(SourceLangBtns[i],&HeadButton::clicked,this,&ApertiumGui::clearOtherSButtons);
        connect(SourceLangBtns[i],&HeadButton::toggled,SourceLangBtns[i],&HeadButton::changeButtonColor);
    }
    for(int i=0;i<TargetLangBtns.size();++i)
    {
        connect(TargetLangBtns[i],&HeadButton::clicked,this,&ApertiumGui::clearOtherEButtons);
        connect(TargetLangBtns[i],&HeadButton::toggled,TargetLangBtns[i],&HeadButton::changeButtonColor);
    }
    SourceLangBtns[0]->setChecked(true);
    TargetLangBtns[0]->setChecked(true);

    ui->SourceLangComboBox->setFixedHeight(ui->SourceLang1->height()-2);
    ui->TargetLangComboBox->setFixedHeight(ui->SourceLang1->height()-2);
    ui->boxInput->setTextColor(QColor(0,0,0));
    ui->boxOutput->setTextColor(QColor(0,0,0));
    translator = new Translator(this);
    translator->moveToThread(&thread);
#ifdef Q_OS_LINUX

    //start server and get available language pairs
    auto request = new QNetworkRequest;
    requestSender = new QNetworkAccessManager(this);
    url = "http://localhost:2737";

    apy = new QProcess;
    apy->setWorkingDirectory(serverPath);
    apy->setArguments(QStringList() << langPairsPath);
    if(!QFile(apy->workingDirectory()+"/servlet.py").exists())
    {
        QMessageBox box;
        box.critical(this,"Path error","Incorrect server directory");
        close();
        return false;
    }
    apy->setArguments(QStringList() << langPairsPath);
    apy->setProgram("./servlet.py");
    apy->start();

    //wait while server starts
    while(true)
    {
        QEventLoop loop;
        auto reply = requestSender->get(QNetworkRequest(QUrl(url.toString()+"/stats")));
        connect(reply, &QNetworkReply::finished,&loop, &QEventLoop::quit);
        loop.exec();
        if (reply->error()== QNetworkReply::NoError)
        {
            reply->deleteLater();
            break;
        }
        reply->deleteLater();
    }


    QAction *dlAction =  new QAction(style()->standardIcon(QStyle::SP_ArrowDown),
                                     tr("Install packages"),ui->toolBar);
    ui->toolBar->addAction(dlAction);
    ui->toolBar->setMovable(false);
    //Installing new packages
    connect(dlAction, &QAction::triggered,this,&ApertiumGui::dlAction_triggered);
    const QString mode = "/listPairs";
    request->setUrl(QUrl(url.toString()+mode));
    QEventLoop loop;
    auto reply= requestSender->get(*request);
    connect(reply,&QNetworkReply::finished,&loop,&QEventLoop::quit);
    loop.exec();
    createListOfLangs(reply);
    if (!initRes)
        dlAction->trigger();

    connect(requestSender,&QNetworkAccessManager::finished,this,&ApertiumGui::getReplyFromAPY);
    connect(ui->boxInput,&InputTextEdit::printEnded,this,&ApertiumGui::createRequests);
    reply->deleteLater();
    dlg->close();
    dlg->deleteLater();

#else
    ui->toolBar->addAction(style()->standardIcon(QStyle::SP_ArrowDown),tr("Install packages"),[&]()
    {DownloadWindow dlWindow;
        if(dlWindow.getData(false))
            dlWindow->exec();
        createListOfLangs();});

    connect(ui->boxInput,&InputTextEdit::printEnded,translator,&Translator::nonLinuxTranslate);
    connect(ui->boxInput,&InputTextEdit::printEnded,this,&ApertiumGui::saveMru);
    connect(translator,&Translator::resultReady,this,&ApertiumGui::translateReceived);
    if (!appdata->exists("usr/share/apertium/modes") || !QDir(DATALOCATION+"/apertium-all-dev").exists())
        dlAction->trigger();
    else
        return initRes;
#endif  
    thread.start();
    connect(ui->SourceLangComboBox->view(), &QTableView::activated, this, &ApertiumGui::updateComboBox);
    connect(ui->SourceLangComboBox->view(), &QTableView::clicked, this, &ApertiumGui::updateComboBox);

    connect(ui->TargetLangComboBox->view(), &QTableView::activated, this, &ApertiumGui::updateEndComboBox);
    connect(ui->TargetLangComboBox->view(), &QTableView::clicked, this, &ApertiumGui::updateEndComboBox);

    connect(ui->actionExit, &QAction::triggered, this, &ApertiumGui::close);
    connect(ui->actionFont_preferences,&QAction::triggered,this,&ApertiumGui::fontSizeBox);
    return initRes;
}

void ApertiumGui::dlAction_triggered()
{

    DownloadWindow dlWindow(this);
    if (dlWindow.getData(checked)) {
        qDebug() << dlWindow.exec();
        checked = true;
        apy->terminate();
        apy->waitForFinished();
        apy->start();
        while(true) {
            QEventLoop loop;
            auto reply = requestSender->get(QNetworkRequest(QUrl(url.toString()+"/listPairs")));
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
        auto reply = tmp.get(QNetworkRequest(QUrl(url.toString()+mode)));
        QEventLoop loop;
        connect(reply, &QNetworkReply::finished,&loop, &QEventLoop::quit);
        loop.exec();
        createListOfLangs(reply);
    }
}

struct ApertiumGui::langpairUsed
{
    QString name;
    unsigned long long n;
    langpairUsed(QString _name, long long _n)
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

//get available language pairs
void ApertiumGui::createListOfLangs(QNetworkReply *reply)
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
    if (reply->error() == QNetworkReply::NoError)
    {
        // read server response
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonArray array = doc.object().value("responseData").toArray();
        if (array.isEmpty()) {
            QMessageBox box;
            box.setModal(true);
            box.critical(this,tr("No langpairs found."),tr("It seems, that no language pairs have been found."),
                         QMessageBox::Ok);
            initRes = false;
            qDebug() << initRes;
            return;
        }
        //parse json response

        //delete non 2-letter names
        for(auto it = array.begin(); it != array.end();)
        {
            QString sourceLanguage = it->toObject().value("sourceLanguage").toString();
            QString targetLanguage = it->toObject().value("targetLanguage").toString();
            if(sourceLanguage.length() > 3 || targetLanguage.length() > 3)
                it = array.erase(it);
            else
                ++it;
        }
        for (auto it : array)
        {
            bool unique = true;
            auto sourceLang = it.toObject().value("sourceLanguage").toString();
            auto targetLang = it.toObject().value("targetLanguage").toString();
            if (Initializer::conf->value(sourceLang + "-" + targetLang).toULongLong()>0ULL)
            {
                langs.insert(langpairUsed(Initializer::langNamesMap[sourceLang]+ "-" + Initializer::langNamesMap[targetLang],
                                          Initializer::conf->value(sourceLang + "-" + targetLang).toULongLong()));
            }
            //unique languages
            for(int j=0;j< ui->SourceLangComboBox->model()->columnCount();++j)
                for(int i=0; i < ui->SourceLangComboBox->model()->rowCount();++i)
                if(ui->SourceLangComboBox->model()->
                        data(ui->SourceLangComboBox->model()->index(i,j)).toString()==Initializer::langNamesMap[sourceLang])
                    unique=false;
            if(unique)
                ui->SourceLangComboBox->model()->addItem(Initializer::langNamesMap[sourceLang]);
        }
        int i = 0;
        //add to mru
        for(auto it = langs.begin(); i < 3 && it != langs.end();it++, i++)
        {
            auto item = new QListWidgetItem(it->name);
            item->setTextAlignment(Qt::AlignCenter);
            ui->mru->addItem(item);
        }
        //fill target ComboBox
        for (auto it : array)
            if(Initializer::langNamesMap[it.toObject().value("sourceLanguage").toString()]==
                    ui->SourceLangComboBox->model()->data(ui->SourceLangComboBox->model()->index(0,0)).toString())
                ui->TargetLangComboBox->model()->addItem(Initializer::langNamesMap[it.toObject().value("targetLanguage").toString()]);
    }
    // Error occured
    else
        {
            QMessageBox box;
            box.critical(this,"Server error",reply->errorString());
            initRes = false;
            return;
        }
#else
        if (!appdata->exists("usr/share/apertium/modes") || !QDir(DATALOCATION+"/apertium-all-dev").exists()) {
            QMessageBox box;
            box.critical(this,tr("No installed langpairs."), tr("You have not installed any langpairs. The application will be closed."));
            return false;
        }
        //get list of availiable languages
        QDir moded(appdata->absoluteFilePath("usr/share/apertium/modes"));
        auto modes = moded.entryInfoList(QStringList() << "*.mode");

        for (QFileInfo mode : modes) {

            bool unique = true;
            auto sourceLanguage = mode.baseName().left(mode.baseName().indexOf("-"));
            auto targetLanguage = mode.baseName().mid(mode.baseName().indexOf("-")+1);
            if (sourceLanguage.length() > 3 || targetLanguage.length() > 3)
                continue;
            if (Initializer::conf->value(sourceLanguage + "-" + targetLanguage).toULongLong()>0ULL)
            {
                langs.insert(langpairUsed(Initializer::langNamesMap[sourceLanguage]+ " - "
                                          + Initializer::langNamesMap[targetLanguage],
                                          Initializer::conf->value(sourceLanguage + "-" + targetLanguage).toULongLong()));
            }
            for(int j=0;j< ui->SourceLangComboBox->model()->columnCount();++j)
                for(int i=0; i < ui->SourceLangComboBox->model()->rowCount();++i)
                if(ui->SourceLangComboBox->model()->data(ui->SourceLangComboBox->model()->index(i,j)).toString()
                        ==Initializer::langNamesMap[sourceLanguage])
                    unique=false;
            if(unique)
            {
                ui->SourceLangComboBox->model()->addItem(Initializer::langNamesMap[sourceLanguage]);
            }
        }
        int i=0;
        for(auto it = langs.begin(); i < 3 && it != langs.end();++it, ++i)
        {
            auto item = new QListWidgetItem(it->name);
            item->setTextAlignment(Qt::AlignCenter);
            ui->mru->addItem(item);
        }
        for (auto mode : modes) {
            auto sourceLanguage = mode.baseName().left(mode.baseName().indexOf("-"));
            auto targetLanguage = mode.baseName().mid(mode.baseName().indexOf("-")+1);
            if(Initializer::langNamesMap[sourceLanguage]==ui->SourceLangComboBox->model()->
                    data(ui->SourceLangComboBox->model()->index(0,0)).toString())
                ui->TargetLangComboBox->model()->addItem(Initializer::langNamesMap[targetLanguage]);
        }
#endif

        //fill buttons with first languages
        for(int i=0;i<SourceLangBtns.size();++i)
        {
            SourceLangBtns[i]->setText(ui->SourceLangComboBox->model()->
                                       data(ui->SourceLangComboBox->model()->index(i,0)).toString());
            SourceLangBtns[i]->setEnabled(!SourceLangBtns[i]->text().isEmpty());
        }

        for(int i=0;i<TargetLangBtns.size();++i)
        {
            TargetLangBtns[i]->setText(ui->TargetLangComboBox->model()->
                                       data(ui->TargetLangComboBox->model()->index(i,0)).toString());
            TargetLangBtns[i]->setEnabled(!TargetLangBtns[i]->text().isEmpty());
        }
        for(int i=0;i<SourceLangBtns.size();++i)
            ui->SourceLangComboBox->model()->removeItem(0,0);
        for(int i=0;i<TargetLangBtns.size();++i)
            ui->TargetLangComboBox->model()->removeItem(0,0);

        ui->SourceLangComboBox->setCurrentIndex(-1);
        ui->TargetLangComboBox->setCurrentIndex(-1);
#ifdef Q_OS_LINUX
        currentSourceLang = Initializer::langNamesMap.key(SourceLangBtns[0]->text());
        currentTargetLang = Initializer::langNamesMap.key(TargetLangBtns[0]->text());
        reply->deleteLater();
#else
        for (auto key : Initializer::langNamesMap.keys(SourceLangBtns[0]->text())) {

            if (key.length()==2)
                currentSourceLang = key;
            else
                currentSourceLang3 = key;
        };
        for (auto key : Initializer::langNamesMap.keys(TargetLangBtns[0]->text())) {
            if (key.length()==2)
                currentTargetLang = key;
            else
                currentTargetLang3 = key;
        };
#endif        
        emit listOfLangsSet();
        initRes = true;
        return;
}

//update ComboBoxes when new source language, that is choosed
void ApertiumGui::updateComboBox(QModelIndex index)
{
    auto curr = index.data().toString();
    if (curr.isEmpty())
        return;
    ui->SourceLangComboBox->model()->removeItem(index.row(),index.column());
    ui->SourceLangComboBox->model()->addItem(SourceLangBtns[2]->text());
    SourceLangBtns[2]->setText(SourceLangBtns[1]->text());
    SourceLangBtns[1]->setText(SourceLangBtns[0]->text());
    SourceLangBtns[0]->setText(curr);
    ui->TargetLangComboBox->model()->clear();
#ifdef Q_OS_LINUX
    //query.addQueryItem("include_deprecated_codes","yes");
    QNetworkRequest request(QUrl(url.toString()+"/listPairs"));

    QEventLoop loop;
    QNetworkAccessManager tmpN(this);
    if (tmpN.networkAccessible() != QNetworkAccessManager::Accessible) {
        QMessageBox::critical(this, tr("Network Inaccessible"), tr("Can't check for updates as you appear to be offline."));
        return;
    }
    connect(&tmpN,&QNetworkAccessManager::finished,&loop, &QEventLoop::quit);
    auto reply = tmpN.get(request);
    loop.exec();
    auto doc = QJsonDocument::fromJson(reply->readAll());
    auto array = doc.object().value("responseData").toArray();
    for(auto it = array.begin(); it != array.end();)
     {
         QString sourceLanguage = (*it).toObject().value("sourceLanguage").toString();
         if (sourceLanguage.length() > 3)
             it = array.erase(it);
         else
             ++it;
     }
    for (auto it : array)
        if(Initializer::langNamesMap[it.toObject().value("sourceLanguage").toString()]==SourceLangBtns[0]->text())
            ui->TargetLangComboBox->model()->addItem(
                        Initializer::langNamesMap[it.toObject().value("targetLanguage").toString()]);

    reply->deleteLater();
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
void ApertiumGui::updateEndComboBox(QModelIndex index)
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
void ApertiumGui::clearOtherSButtons()
{
    ui->boxOutput->clear();
    currentSButton = qobject_cast<HeadButton*>(sender());
    for (int i=0;i<SourceLangBtns.size();++i) {
        if (SourceLangBtns[i]->text().isEmpty())
            SourceLangBtns[i]->setEnabled(false);
        else
        {
            SourceLangBtns[i]->setEnabled(true);
            if(SourceLangBtns[i]!=currentSButton)
                SourceLangBtns[i]->setChecked(false);
            else
#ifdef Q_OS_LINUX
                currentSourceLang = Initializer::langNamesMap.key(SourceLangBtns[i]->text());
#else
                for (auto key: Initializer::langNamesMap.keys(SourceLangBtns[i]->text())) {
                    if (key.length()==2)
                        currentSourceLang = key;
                    else
                        currentSourceLang3 = key;
                }
#endif
        }
    }
#ifdef Q_OS_LINUX
    auto requestSenderTmp = new QNetworkAccessManager;
    QUrlQuery query;
    query.addQueryItem("include_deprecated_codes","yes");
    QNetworkRequest request(url.toString()+"/listPairs?"+query.toString());
    connect(requestSenderTmp,&QNetworkAccessManager::finished,this,&ApertiumGui::getResponseOfAvailLang);
    requestSenderTmp->get(request);
#else
    QDir moded(appdata->absoluteFilePath("usr/share/apertium/modes"));
    auto modes = moded.entryInfoList(QStringList() << "*.mode");
    ui->TargetLangComboBox->model()->clear();
    for (auto mode : modes) {
        if (QRegExp("^[a-z]+-[a-z]+$").exactMatch(mode.baseName())
                && Initializer::langNamesMap[mode.baseName().left(mode.baseName().indexOf("-"))]==currentSButton->text())
            ui->TargetLangComboBox->model()->
                    addItem(Initializer::langNamesMap[mode.baseName().mid(mode.baseName().indexOf("-")+1)]);
    }
    for(int i=0;i<TargetLangBtns.size();++i)
        TargetLangBtns[i]->setText(ui->TargetLangComboBox->model()->
                                   data(ui->TargetLangComboBox->model()->index(i,0)).toString());
    for(int i=0;i<TargetLangBtns.size();++i)
        ui->TargetLangComboBox->model()->removeItem(0,0);
    TargetLangBtns[0]->setEnabled(true);
    TargetLangBtns[0]->click();
    emit listOfLangsSet();
#endif
}

//Uncheck Other Target language buttons when the new one is checked
void ApertiumGui::clearOtherEButtons()
{
    auto currentButton = qobject_cast<HeadButton*>(sender());
    for (int i=0;i<TargetLangBtns.size();++i) {
        if (TargetLangBtns[i]->text().isEmpty())
        {
            TargetLangBtns[i]->setEnabled(false);
            TargetLangBtns[i]->setChecked(false);
        }
        else
        {
            TargetLangBtns[i]->setEnabled(true);
            if(TargetLangBtns[i]!=currentButton)
                TargetLangBtns[i]->setChecked(false);
            else
#ifdef Q_OS_LINUX
                currentTargetLang = Initializer::langNamesMap.key(TargetLangBtns[i]->text());
#else
                for (auto key: Initializer::langNamesMap.keys(TargetLangBtns[i]->text())) {
                    if (key.length()==2)
                        currentTargetLang = key;
                    else
                        currentTargetLang3 = key;
                }
#endif
        }
    }
}

//FOR LINUX
//update available Target languages
void ApertiumGui::getResponseOfAvailLang(QNetworkReply *reply)
{
    auto docAvailLang = QJsonDocument::fromJson(reply->readAll());
    auto array = docAvailLang.object().value("responseData").toArray();
    for(auto it = array.begin(); it != array.end();)
    {
        QString sourceLanguage = (*it).toObject().value("sourceLanguage").toString();
        QString targetLanguage = it->toObject().value("targetLanguage").toString();
        if(sourceLanguage.length() > 3 || targetLanguage.length() > 3)
            it = array.erase(it);
        else
            ++it;
    }
    ui->TargetLangComboBox->model()->clear();
    for (auto it : array)
        if(Initializer::langNamesMap[it.toObject().value("sourceLanguage").toString()]==currentSButton->text())
            ui->TargetLangComboBox->model()->addItem(Initializer::langNamesMap[it.toObject().value("targetLanguage").toString()]);

    for(int i=0;i<TargetLangBtns.size();++i)
        TargetLangBtns[i]->setText(ui->TargetLangComboBox->model()->data(ui->TargetLangComboBox->model()->index(i,0)).toString());
    for(int i=0;i<TargetLangBtns.size();++i)
        ui->TargetLangComboBox->model()->removeItem(0,0);

    TargetLangBtns[0]->click();
    emit listOfLangsSet();
    reply->deleteLater();
}

//send translation request for paragraph
void ApertiumGui::createRequests()
{
    auto request = new QNetworkRequest;
    QString mode = "/translate?";
    QUrlQuery urlQ;
    urlQ.addQueryItem("langpair", currentSourceLang+"|"+currentTargetLang);

    if (lastBlockCount!=ui->boxInput->document()->blockCount())
    {
        ui->boxOutput->clear();
        outputDoc.clear();
        for(auto paragraph : ui->boxInput->toPlainText().split("\n"))
        {
            urlQ.addQueryItem("q",QUrl::toPercentEncoding(paragraph));
            request->setUrl(QUrl(url.toString()+mode+urlQ.query()));
            request->setRawHeader("whole","yes");
            translator->linuxTranslate(*request);
        }

        //TODO: rewrite this part
        auto cursor = ui->boxOutput->textCursor();
        for(int i=0; i<outputDoc.blockCount();i++)
        {
            if (outputDoc.findBlockByNumber(i)
                    != ui->boxOutput->document()->findBlockByNumber(i)
                    || i>=ui->boxOutput->document()->blockCount())
            {

                if (i>=ui->boxOutput->document()->blockCount())
                {
                    cursor.movePosition(QTextCursor::End);
                    cursor.insertBlock();
                }
                cursor.setPosition(
                            ui->boxOutput->document()->findBlockByNumber(i).position());
                cursor.movePosition(QTextCursor::EndOfBlock,QTextCursor::KeepAnchor);
                auto format = cursor.charFormat();
                format.setForeground(QColor(0, 0, 0));
                cursor.insertText(outputDoc.findBlockByNumber(i).text()+"\n", format);
            }
        }
        ui->boxOutput->setTextCursor(cursor);
        ui->boxOutput->verticalScrollBar()->setValue(ui->boxInput->verticalScrollBar()->value());
    }
    else
    {
        auto cursor = ui->boxInput->textCursor();
        urlQ.addQueryItem("q",QUrl::toPercentEncoding(cursor.block().text()));
        request->setUrl(QUrl(url.toString()+mode+urlQ.query()));
        request->setRawHeader("blockNumber",
                              QByteArray::number(cursor.block().blockNumber()));
        request->setRawHeader("whole","no");
        translator->linuxTranslate(*request);

    }
    lastBlockCount = ui->boxInput->document()->blockCount();
    auto langpair = currentSourceLang+"-"+currentTargetLang;
    Initializer::conf->setValue(langpair, QVariant(Initializer::conf->value
                                          (langpair,QVariant(0ULL)).toULongLong() + 1ULL));
}


//parse json response
void ApertiumGui::getReplyFromAPY(QNetworkReply* reply)
{
    if (reply->error() == QNetworkReply::NoError)
    {
        auto doc = QJsonDocument::fromJson(reply->readAll());
        if (reply->request().rawHeader("whole")=="no")
        {
            auto cursor = ui->boxOutput->textCursor();
            int blockNumber = reply->request().rawHeader("blockNumber").toInt();
            if(blockNumber>=ui->boxOutput->document()->blockCount())
            {
                cursor.movePosition(QTextCursor::End);
                cursor.insertBlock();
            }
            cursor.setPosition(ui->boxOutput->
                               document()->findBlockByNumber(blockNumber).position());
            cursor.movePosition(QTextCursor::EndOfBlock,QTextCursor::KeepAnchor);
            auto format = cursor.charFormat();
            format.setForeground(QColor(0, 0, 0));
            cursor.insertText(doc.object().value("responseData")
                              .toObject().value("translatedText").toString(),format);

            cursor.movePosition(QTextCursor::StartOfBlock);
            ui->boxOutput->verticalScrollBar()->setValue(ui->boxInput->verticalScrollBar()->value());
            while(cursor.blockNumber()==blockNumber && !cursor.atBlockEnd())
            {

                auto cursor1 = cursor.document()->
                        find(QRegularExpression ("\\*\\w+\\W?"),cursor.position());
                if (cursor1.isNull())
                    break;
                cursor = cursor1;
                auto format = cursor.charFormat();
                format.setForeground(QColor(255, 0, 0));
                cursor.insertText(cursor.selectedText().mid(1),format);

            }
            ui->boxOutput->setTextCursor(cursor);
        }
        else
            outputDoc.setPlainText(outputDoc.toPlainText()
                                   +doc.object().value("responseData")
                                   .toObject().value("translatedText").toString()+"\n");
    }
    else
        qDebug() << reply->errorString();

    reply->deleteLater();
}

void ApertiumGui::resizeEvent(QResizeEvent* e)
{
    ui->boxInput->resize(this->width()/2-100,300);
    ui->boxOutput->resize(this->width()/2-100,300);
    ui->label->setPixmap(ui->label->pixmap()->scaled(ui->label->pixmap()->width(),ui->label->pixmap()->height(),Qt::KeepAspectRatio));
    ui->label->repaint();
}
ApertiumGui::~ApertiumGui()
{
#ifdef Q_OS_LINUX
    apy->terminate();
    apy->waitForFinished();
    if (apy->state()!=QProcess::NotRunning)
        apy->kill();
#endif
    thread.quit();
    thread.wait();
    delete ui;
}

void ApertiumGui::fontSizeBox()
{
    fSizeBox = new QDialog;
    fSizeBox->setWindowTitle("Set fontsize of translation boxes");
    auto layout = new QVBoxLayout(fSizeBox);
    auto fontSize = new QSpinBox(fSizeBox);
    currentFontSize=ui->boxInput->fontInfo().pointSize();
    fontSize->setValue(currentFontSize);
    connect(fontSize, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &ApertiumGui::changeFontSize);
    layout->addWidget(fontSize);
    auto button = new QDialogButtonBox(fSizeBox);
    button->addButton(QDialogButtonBox::Ok);
    button->addButton(QDialogButtonBox::Cancel);
    connect(button->button(QDialogButtonBox::Ok),&QPushButton::clicked,[&](){fSizeBox->close(); fSizeBox->deleteLater();});
    connect(button->button(QDialogButtonBox::Cancel),&QPushButton::clicked,this,&ApertiumGui::fontSizeCancel);
    layout->addWidget(button);
    fSizeBox->setLayout(layout);
    fSizeBox->setModal(true);
    fSizeBox->show();
}
void ApertiumGui::changeFontSize(int size)
{
    QFont font(ui->boxInput->font());
    font.setPointSize(size);
    ui->boxInput->setFont(font);
    ui->boxOutput->setFont(font);
    Initializer::conf->setValue(FONTSIZE,QVariant(font.pointSize()));
}

void ApertiumGui::fontSizeCancel()
{
    QFont font(ui->boxInput->font());
    font.setPointSize(currentFontSize);
    ui->boxInput->setFont(font);
    ui->boxOutput->setFont(font);
    Initializer::conf->setValue(FONTSIZE,QVariant(font.pointSize()));
    fSizeBox->close();
}

void ApertiumGui::on_mru_currentTextChanged(const QString &currentText)
{
    int sBtnIndex = -1;
    int tBtnIndex = -1;
    for (int i = 0; i<SourceLangBtns.size();++i)
        if (SourceLangBtns[i]->text()==currentText.left(currentText.indexOf(' ')))
        {
            sBtnIndex = i;
            break;
        }
    if (sBtnIndex!=-1)
        SourceLangBtns[sBtnIndex]->click();
    else
    {
        QEventLoop loop;
        connect(this, &ApertiumGui::listOfLangsSet,&loop, &QEventLoop::quit);
        emit ui->SourceLangComboBox->view()->activated(ui->SourceLangComboBox->model()->
                                                       findText(currentText.left(currentText.indexOf(' '))));
        loop.exec();
    }
    for (int i = 0; i<TargetLangBtns.size();++i)
    {
        if (TargetLangBtns[i]->text()==currentText.mid(currentText.indexOf("- ")+2))
        {
            tBtnIndex = i;
            break;
        }
    }
    if (tBtnIndex!=-1)
        TargetLangBtns[tBtnIndex]->click();
    else
        emit ui->TargetLangComboBox->view()->activated(ui->TargetLangComboBox->model()->
                                                       findText(currentText.mid(currentText.indexOf("- ")+2)));

}

void ApertiumGui::loadConf()
{
    //Save entered pathes to Server and langpairs
    serverPath = Initializer::conf->value(SERVERPATH).toString();
    langPairsPath = Initializer::conf->value(LANGPATH).toString();
    QFont font(ui->boxInput->font());
    font.setPointSize(Initializer::conf->value(FONTSIZE,QVariant(11)).toInt());
    ui->boxInput->setFont(font);
    ui->boxOutput->setFont(font);
}


//NOT FOR LINUX
//taken from Simpleton

void ApertiumGui::saveMru()
{
    auto langpair = currentSourceLang+"-"+currentTargetLang;
    Initializer::conf->setValue(langpair, QVariant(Initializer::conf->value
                                          (langpair,QVariant(0ULL)).toULongLong() + 1ULL));
}
void ApertiumGui::translateReceived(const QString &result)
{
    ui->boxOutput->clear();
    auto cursor = ui->boxOutput->textCursor();
    cursor.movePosition(QTextCursor::Start);
    auto format = cursor.charFormat();
    format.setForeground(QColor(0, 0, 0));
    cursor.insertText(result, format);
    cursor.movePosition(QTextCursor::Start);
    while(!cursor.atEnd())
    {
        auto cursor1 = cursor.document()->
                find(QRegularExpression ("[\\*#]\\w+\\W?"),cursor.position());
        if (cursor1.isNull())
            break;
        cursor = cursor1;
        auto format = cursor.charFormat();
        format.setForeground(QColor(255, 0, 0));
        cursor.insertText(cursor.selectedText().mid(1),format);
    }
    ui->boxOutput->setTextCursor(cursor);
    ui->boxOutput->verticalScrollBar()->setValue(ui->boxInput->verticalScrollBar()->value());

}

void ApertiumGui::on_boxInput_currentCharFormatChanged(const QTextCharFormat &format)
{
    ui->boxInput->setTextColor(Qt::black);
}
