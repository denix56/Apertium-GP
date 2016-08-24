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

#include "translator.h"
#include "gpmainwindow.h"
#include "initializer.h"
#include <QDir>
#include <QProcess>
#include <QRegularExpression>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTextCodec>
#include <QUrlQuery>
#include <QHttpMultiPart>
#include <QDebug>
#include <QApplication>

Translator::Translator(GpMainWindow* parent)
{
    this->parent = parent;
}

void Translator::boxTranslate()
{
    QString result = notLinuxTranslate(parent->getText());
    emit resultReady(result.left(result.length()-2));
}

void Translator::winTrayTranslate(QString text)
{
    QString result = notLinuxTranslate(text);
    emit trayResultReady(result.left(result.length()-2));
}

#ifndef Q_OS_LINUX
void Translator::translateTxt(QString filePath, QDir &docDir)
{
    QFile f(filePath);
    QFileInfo fileInfo(filePath);
    QTemporaryFile tmpDoc(fileInfo.baseName());
    auto cmd = new QProcess(this);
#ifdef Q_OS_WIN
    cmd->setWorkingDirectory(QDir(DATALOCATION).absoluteFilePath("apertium-all-dev/bin"));
#endif
    qApp->processEvents();
    //destxt
    cmd->start("cmd.exe", QStringList() << "/u" << "/c" << "type" << "\""+filePath.replace('/', QDir::separator())+"\""
               << "|" << "apertium-destxt");
    cmd->waitForFinished();
    cmd->waitForReadyRead();
    if (!tmpDoc.open()) {
        emit docTranslateRejected();
        cmd->deleteLater();
        return;
    }
    tmpDoc.setTextModeEnabled(true);
    tmpDoc.write(notLinuxTranslate(cmd->readAllStandardOutput()).toUtf8());
    //retxt
    cmd->start("cmd.exe", QStringList() << "/u" << "/c" << "type" << "\""+tmpDoc.fileName().replace('/', QDir::separator())
               +"\"" << "|" << "apertium-retxt");
    cmd->waitForFinished();
    cmd->waitForReadyRead();
    QString trFilePath = docDir.absoluteFilePath(fileInfo.baseName()+"_"+parent->getCurrentSourceLang()
                                                 +"-"+parent->getCurrentTargetLang()+"."+fileInfo.suffix());
    QFile newDoc(trFilePath);
    if (!newDoc.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        emit docTranslateRejected();
        cmd->deleteLater();
        return;
    }
    newDoc.write(cmd->readAllStandardOutput());
    newDoc.close();
    emit docTranslated(trFilePath);
    cmd->deleteLater();
}

void Translator::translateDocx(QString filePath, QDir &docDir)
{
    QFileInfo fileInfo(filePath);
    QTemporaryDir dir(docDir.absoluteFilePath(fileInfo.baseName()+".XXXXXX"));
    auto cmd = new QProcess(this);
    QDir exep (qApp->applicationDirPath());
    cmd->start(exep.absoluteFilePath("7z"), QStringList() << "t" << fileInfo.absoluteFilePath());
    cmd->waitForReadyRead();
    if (cmd->readAllStandardOutput().contains("ERROR")) {
        emit docTranslateRejected();
        cmd->deleteLater();
        return;
    }
    cmd->waitForFinished();
    cmd->start(exep.absoluteFilePath("7z"), QStringList() << "x" << "-y" << fileInfo.absoluteFilePath()
               << "-o"+dir.path());
    cmd->waitForReadyRead();
    cmd->waitForFinished();

    QTemporaryFile tmpDoc("document.xml");
    QFile doc(dir.path()+"/word/document.xml");
    cmd->setWorkingDirectory(QDir(DATALOCATION).absoluteFilePath("apertium-all-dev/bin"));
    QStringList args;
    qApp->processEvents();
    //deswxml
#ifdef Q_OS_WIN
    //windows bug with backslash
    //use replace to fix windows bug
    args << "/u" << "/c" << "type" << "\""+doc.fileName().replace('/',QDir::separator())+"\""
         << "|" << "apertium-deswxml";
    cmd->setNativeArguments(args.join(' '));
    cmd->start("cmd.exe");
#endif
    cmd->waitForFinished();
    cmd->waitForReadyRead();
    if(!tmpDoc.open()) {
        emit docTranslateRejected();
        cmd->deleteLater();
        return;
    }
    tmpDoc.setTextModeEnabled(true);
    tmpDoc.write(notLinuxTranslate(cmd->readAllStandardOutput()).toUtf8());
    tmpDoc.close();
    //rewxml
    args.clear();
#ifdef Q_OS_WIN
    args << "/u" << "/c" << "type" << "\""+tmpDoc.fileName().replace('/',QDir::separator())+"\""
         << "|" << "apertium-rewxml";
    cmd->setNativeArguments(args.join(' '));
    cmd->start("cmd.exe");
#endif
    cmd->waitForFinished();
    cmd->waitForReadyRead();
    if(!doc.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        emit docTranslateRejected();
        cmd->deleteLater();
        return;
    }
    doc.write(cmd->readAllStandardOutput());
    doc.close();
    cmd->setWorkingDirectory(dir.path());
    QString trFilePath = docDir.absoluteFilePath(fileInfo.baseName()+"_"+parent->getCurrentSourceLang()
                                                 +"-"+parent->getCurrentTargetLang()+"."+fileInfo.suffix());
    cmd->start(exep.absoluteFilePath("7z"), QStringList() << "a" << "-tzip" << "-y"
               << trFilePath << dir.path()+"/*");
    cmd->waitForFinished();
    emit docTranslated(trFilePath);
    cmd->deleteLater();
}

//TODO: add support of URL
void Translator::translateHtml(QString filePath, QDir &docDir)
{
    QFileInfo fileInfo(filePath);
    auto cmd = new QProcess(this);

    QTemporaryFile tmpDoc("document.html");
    cmd->setWorkingDirectory(QDir(DATALOCATION).absoluteFilePath("apertium-all-dev/bin"));
    QStringList args;
    qApp->processEvents();
    //deshtml
#ifdef Q_OS_WIN
    //windows bug with backslash
    //use replace to fix windows bug
    args << "/u" << "/c" << "type" << "\""+filePath.replace('/',QDir::separator())+"\""
         << "|" << "apertium-deshtml";
    cmd->setNativeArguments(args.join(' '));
    cmd->start("cmd.exe");
#endif
    cmd->waitForFinished();
    cmd->waitForReadyRead();
    if(!tmpDoc.open()) {
        emit docTranslateRejected();
        cmd->deleteLater();
        return;
    }
    tmpDoc.setTextModeEnabled(true);
    tmpDoc.write(notLinuxTranslate(cmd->readAllStandardOutput()).toUtf8());
    tmpDoc.close();
    //rehtml
    //FIXME: rehtml or rehtml-noent???
    args.clear();
#ifdef Q_OS_WIN
    args << "/u" << "/c" << "type" << "\""+tmpDoc.fileName().replace('/',QDir::separator())+"\""
         << "|" << "apertium-rehtml";
    cmd->setNativeArguments(args.join(' '));
    cmd->start("cmd.exe");
#endif
    cmd->waitForFinished();
    cmd->waitForReadyRead();
    QString trFilePath = docDir.absoluteFilePath(fileInfo.baseName()+"_"+parent->getCurrentSourceLang()
                                                 +"-"+parent->getCurrentTargetLang()+"."+fileInfo.suffix());
    QFile newDoc(trFilePath);
    if (!newDoc.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        emit docTranslateRejected();
        cmd->deleteLater();
        return;
    }
    newDoc.write(cmd->readAllStandardOutput());
    newDoc.close();
    emit docTranslated(trFilePath);
    cmd->deleteLater();
    //#else
    //    // cmd->start("ar", QStringList() << "x" << "data.tmp");
    //#endif
}


void Translator::translatePptx(QString filePath, QDir &docDir)
{
    QFileInfo fileInfo(filePath);
    QTemporaryDir dir(docDir.absoluteFilePath(fileInfo.baseName()+".XXXXXX"));
    auto cmd = new QProcess(this);
    QDir exep (qApp->applicationDirPath());
#ifndef Q_OS_LINUX
    cmd->start(exep.absoluteFilePath("7z"), QStringList() << "t" << fileInfo.absoluteFilePath());
#endif
    cmd->waitForReadyRead();
    QString res = cmd->readAllStandardOutput();
    if (res.contains("ERROR")) {
        emit docTranslateRejected();
        cmd->deleteLater();
        return;
    }
    cmd->waitForFinished();
    cmd->start(exep.absoluteFilePath("7z"), QStringList() << "x" << "-y" << fileInfo.absoluteFilePath()
               << "-o"+dir.path());
    cmd->waitForReadyRead();
    cmd->waitForFinished();
    QDir slideDir(dir.path()+"/ppt/slides");
    QStringList args;
    for (QString slideName : slideDir.entryList(QStringList() << "slide*.xml")) {
        QFile slide (slideDir.absoluteFilePath(slideName));
        QTemporaryFile tmpSlide(slide.fileName());
        cmd->setWorkingDirectory(QDir(DATALOCATION).absoluteFilePath("apertium-all-dev/bin"));

        qApp->processEvents();
        //despptx
        args << "/u" << "/c" << "type" << "\""+slide.fileName().replace('/',QDir::separator())+"\""
             << "|" << "apertium-despptx";
        cmd->setNativeArguments(args.join(' '));
        args.clear();
        cmd->start("cmd.exe");
        cmd->waitForReadyRead();
        cmd->waitForFinished();
        if(!tmpSlide.open()) {
            emit docTranslateRejected();
            cmd->deleteLater();
            return;
        }
        tmpSlide.setTextModeEnabled(true);
        tmpSlide.write(notLinuxTranslate(cmd->readAllStandardOutput()).toUtf8());
        tmpSlide.close();
        //repptx
        args << "/u" << "/c" << "type" << "\""+tmpSlide.fileName().replace('/',QDir::separator())+"\""
             << "|" << "apertium-repptx";
        cmd->setNativeArguments(args.join(" "));
        args.clear();
        cmd->start("cmd.exe");
        cmd->waitForReadyRead();
        if(!slide.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            emit docTranslateRejected();
            cmd->deleteLater();
            return;
        }
        slide.write(cmd->readAllStandardOutput());
        cmd->waitForFinished();

        slide.close();
    }
    cmd->setWorkingDirectory(dir.path());
    QString trFilePath = docDir.absoluteFilePath(fileInfo.baseName()+"_"+parent->getCurrentSourceLang()
                                                 +"-"+parent->getCurrentTargetLang()+"."+fileInfo.suffix());
    QFile(trFilePath).remove();
    args << "a" << "-tzip" << "-y"
         << "\""+trFilePath+"\""
         << "\""+dir.path()+"/*\"";
    cmd->setNativeArguments(args.join(' '));
    cmd->start(exep.absoluteFilePath("7z"));
    cmd->waitForFinished();
    emit docTranslated(trFilePath);
    cmd->deleteLater();
}

void Translator::translateXlsx(QString filePath, QDir &docDir)
{
    QFileInfo fileInfo(filePath);
    QTemporaryDir dir(docDir.absoluteFilePath(fileInfo.baseName()+".XXXXXX"));
    auto cmd = new QProcess(this);
    QDir exep (qApp->applicationDirPath());
#ifndef Q_OS_LINUX
    cmd->start(exep.absoluteFilePath("7z"), QStringList() << "t" << fileInfo.absoluteFilePath());
#endif
    cmd->waitForReadyRead();
    QString res = cmd->readAllStandardOutput();
    if (res.contains("ERROR")) {
        emit docTranslateRejected();
        cmd->deleteLater();
        return;
    }
    cmd->waitForFinished();
    cmd->start(exep.absoluteFilePath("7z"), QStringList() << "x" << "-y" << fileInfo.absoluteFilePath()
               << "-o"+dir.path());
    cmd->waitForReadyRead();
    cmd->waitForFinished();
    QStringList args;
    //for (QString sheetName : sheetDir.entryList(QStringList() << "sheet*.xml")) {
        QFile sheet (QDir(dir.path()+"/xl").absoluteFilePath("sharedStrings.xml"));
        QTemporaryFile tmpSheet(sheet.fileName());
        cmd->setWorkingDirectory(QDir(DATALOCATION).absoluteFilePath("apertium-all-dev/bin"));

        qApp->processEvents();
        //desxlsx
        args << "/u" << "/c" << "type" << "\""+sheet.fileName().replace('/',QDir::separator())+"\""
             << "|" << "apertium-desxlsx";
        cmd->setNativeArguments(args.join(' '));
        args.clear();
        cmd->start("cmd.exe");
        cmd->waitForReadyRead();
        cmd->waitForFinished();
        if(!tmpSheet.open()) {
            emit docTranslateRejected();
            cmd->deleteLater();
            return;
        }
        tmpSheet.setTextModeEnabled(true);
        tmpSheet.write(notLinuxTranslate(cmd->readAllStandardOutput()).toUtf8());
        tmpSheet.close();
        //rexlsx
        args << "/u" << "/c" << "type" << "\""+tmpSheet.fileName().replace('/',QDir::separator())+"\""
             << "|" << "apertium-rexlsx";
        cmd->setNativeArguments(args.join(" "));
        args.clear();
        cmd->start("cmd.exe");
        cmd->waitForReadyRead();
        if(!sheet.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            emit docTranslateRejected();
            cmd->deleteLater();
            return;
        }
        sheet.write(cmd->readAllStandardOutput());
        cmd->waitForFinished();

        sheet.close();
    cmd->setWorkingDirectory(dir.path());
    QString trFilePath = docDir.absoluteFilePath(fileInfo.baseName()+"_"+parent->getCurrentSourceLang()
                                                 +"-"+parent->getCurrentTargetLang()+"."+fileInfo.suffix());
    QFile(trFilePath).remove();
    args << "a" << "-tzip" << "-y"
         << "\""+trFilePath+"\""
         << "\""+dir.path()+"/*\"";
    cmd->setNativeArguments(args.join(' '));
    cmd->start(exep.absoluteFilePath("7z"));
    cmd->waitForFinished();
    emit docTranslated(trFilePath);
    cmd->deleteLater();
}

void Translator::translateRtf(QString filePath, QDir &docDir)
{
//    QFileInfo fileInfo(filePath);
//    QTemporaryFile tmpDoc(QDir(QDir::tempPath()).absoluteFilePath(fileInfo.baseName()));
//    auto cmd = new QProcess(this);
//#ifdef Q_OS_WIN
//    cmd->setWorkingDirectory(parent->appdata->absoluteFilePath("apertium-all-dev/bin"));
//#endif
//    qApp->processEvents();
//    //desrtf
//    QEventLoop loop;

//    //connect(cmd, &QProcess::readyReadStandardOutput, [&](){qDebug() << cmd->readAllStandardOutput();});
//    QStringList args;
//    cmd->setEnvironment( QProcess::systemEnvironment() );
//    //cmd->setProcessChannelMode( QProcess:);
//    #ifdef Q_OS_WIN
//    //windows bug with backslash
//    //use replace to fix windows bug
//    args << "/c" << "type" << "\""+filePath.replace('/', QDir::separator())+"\"" << "|" <<"apertium-desrtf";
//    cmd->setNativeArguments(args.join(' '));
//    //args.clear();
//    //cmd->setTextModeEnabled(true);
//    //cmd->setStandardOutputFile("C:/Users/Denis/Documents/t.txt");
//    //cmd->setArguments(args);
//    cmd->start("cmd.exe");
//    cmd->execute("cmd.exe /c type \""+filePath.replace('/', QDir::separator())+"\" | \""
//                 +parent->appdata->absoluteFilePath("apertium-all-dev/bin").replace('/', QDir::separator())+"\\apertium-desrtf\"");
//   // cmd->waitForFinished();
//    //loop.exec();
//    //cmd->waitForFinished();
//    cmd->waitForReadyRead();
//#endif
//    if(!tmpDoc.open()) {
//        emit docTranslateRejected();
//        cmd->deleteLater();
//        return;
//    }
//    tmpDoc.setTextModeEnabled(true);
//    cmd->waitForFinished();
//    //qDebug() << cmd->readAllStandardError();
//    QString out = cmd->readAllStandardOutput();
//    //qDebug() << out;
//    //qDebug() << notLinuxTranslate(out).toUtf8();
//    tmpDoc.write(notLinuxTranslate(out).toUtf8());
//    tmpDoc.close();
//    //rertf
//#ifdef Q_OS_WIN
//    args  << "/c" << "type" << "\""+tmpDoc.fileName().replace('/',QDir::separator())+"\""
//         << "|" << "apertium-rertf";
//    cmd->setNativeArguments(args.join(' '));
//    cmd->start("cmd.exe");
//#endif
//    QString trFilePath = docDir.absoluteFilePath(fileInfo.baseName()+"_"+parent->getCurrentSourceLang()
//                                                 +"-"+parent->getCurrentTargetLang()+"."+fileInfo.suffix());
//    QFile newDoc(trFilePath);
//    if (!newDoc.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
//        emit docTranslateRejected();
//        cmd->deleteLater();
//        return;
//    }
//    QByteArray ar = cmd->readAllStandardOutput();
//    qDebug() << ar;
//    newDoc.write(ar);
//    newDoc.close();
//    cmd->waitForFinished();
//    emit docTranslated(trFilePath);
//    cmd->deleteLater();
//    //#else
//    //    // cmd->start("ar", QStringList() << "x" << "data.tmp");
//    //#endif
}
#endif
void Translator::docTranslate(QString filePath)
{

    QDir outputDocDir(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    QString dirName = tr("Apertium Translated Documents");
    outputDocDir.mkdir(dirName);
    if (!outputDocDir.cd(dirName)) {
        emit docTranslateRejected();
        return;
    }
    QFileInfo fileInfo(filePath);
#ifdef Q_OS_LINUX
    auto multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QVariant("form-data; filename=\""+fileInfo.fileName()+"\"; name=\"file\""));
    auto file = new QFile(filePath);
    if(!file->open(QIODevice::ReadOnly)) {
        emit docTranslateRejected();
        return;
    }
    filePart.setBodyDevice(file);
    file->setParent(multiPart);
    multiPart->append(filePart);

    QUrlQuery query;
    query.addQueryItem("langpair",parent->getCurrentSourceLang()+"|"+parent->getCurrentTargetLang());
    QUrl url("http://localhost:2737/translateDoc?");
    QNetworkRequest request(QUrl(url.url()+query.query()));
    QNetworkAccessManager mngr;
    QEventLoop loop;
    QNetworkReply *reply = mngr.post(request,multiPart);
    connect(reply,&QNetworkReply::finished,[&](){
        loop.exit();
        if (reply->error()!=QNetworkReply::NoError) {
            qDebug() << reply->errorString();
            emit docTranslateRejected();
            return;
        }
        QFile outDoc(outputDocDir.absoluteFilePath(fileInfo.baseName()+"_"+parent->getCurrentSourceLang()
                                             +"-"+parent->getCurrentTargetLang()+"."+fileInfo.completeSuffix()));
        if (!outDoc.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                emit docTranslateRejected();
                reply->deleteLater();
                return;
            }
        outDoc.write(reply->readAll());
        outDoc.close();
        emit docTranslated(outDoc.fileName());
        reply->deleteLater();
    });
            loop.exec();
#else
    if (fileInfo.suffix()=="txt")
        translateTxt(filePath, outputDocDir);
    else
        if (fileInfo.suffix()=="docx")
            translateDocx(filePath, outputDocDir);
        else
            if (fileInfo.suffix()=="pptx")
                translatePptx(filePath, outputDocDir);
            else
                if (fileInfo.suffix()=="xlsx")
                    translateXlsx(filePath, outputDocDir);
                else
                    if (fileInfo.suffix()=="html")
                        translateHtml(filePath, outputDocDir);
                    else
                        if (fileInfo.suffix()=="rtf")
                            translateRtf(filePath, outputDocDir);
#endif
}

QString Translator::notLinuxTranslate(QString text)
{
    QString name = parent->getCurrentSourceLang() + "-" + parent->getCurrentTargetLang();
    if (name.isEmpty())
        return "";

    QDir dir(QDir(DATALOCATION).absoluteFilePath("usr/share/apertium/modes"));
    if (!dir.exists() || !dir.exists(name+".mode"))
        return "";

    QFile file(dir.absoluteFilePath(name+".mode"));
    if (file.open(QIODevice::ReadOnly) == false) {
        return "";
    }
    QString mode = file.readAll();
    file.close();

    mode = mode.trimmed();
    if (mode.isEmpty()) {
        return "";
    }
    mode.replace("$1", "-g");
    mode.remove("$2");
    if (mode.indexOf("'/usr/share") == -1) {
        mode.replace(QRegularExpression("(\\s*)(/usr/share/\\S+)(\\s*)"), "\\1\"\\2\"\\3");
    }
    mode.replace("/usr/share", QDir(DATALOCATION).absolutePath()+"/usr/share");
#ifdef Q_OS_WIN
    // Windows can't handle C:/ paths in ' quotes
    mode.replace("'", "\"");
#define OS_SEP ";"
#else
#define OS_SEP ":"
#endif

    auto env = QProcessEnvironment::systemEnvironment();
    env.insert("PATH", QDir(DATALOCATION).absoluteFilePath("apertium-all-dev/bin") + OS_SEP + env.value("PATH"));
    env.insert("LC_ALL", "en_US.UTF-8");

    auto run = new QProcess(this);
    run->setProcessEnvironment(env);
    run->setProcessChannelMode(QProcess::MergedChannels);
#ifdef Q_OS_WIN
    run->setNativeArguments(mode);
    run->start("cmd", QStringList() << "/D" << "/Q" << "/S" << "/C");
#else
    run->start("/bin/sh", QStringList() << "-c" << mode);
#endif
    run->waitForStarted();
    run->write(text.toUtf8()+"  ");
    run->closeWriteChannel();
    run->waitForFinished();
    run->deleteLater();
    return QString::fromUtf8(run->readAll());
}

void Translator::linuxTranslate(QNetworkRequest &request)
{
    QEventLoop loop;
    connect(parent->getManager()->get(request),&QNetworkReply::finished,&loop, &QEventLoop::quit);
    loop.exec();
}
