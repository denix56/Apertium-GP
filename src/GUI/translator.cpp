
#include "translator.h"
#include "apertiumgui.h"
#include "ui_apertiumgui.h"
#include <QDir>
#include <QProcess>
#include <QRegularExpression>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTextCodec>
#include <QDebug>
Translator::Translator(ApertiumGui* parent)
{
    this->parent = parent;
}

void Translator::boxTranslate()
{
    QString result = notLinuxTranslate(parent->ui->boxInput->toPlainText());
    emit resultReady(result.left(result.length()-2));
}

void Translator::translateTxt(QString filePath, QDir &docDir)
{
    QFile f(filePath);
    QFileInfo fileInfo(filePath);
    QFile outF(docDir.absoluteFilePath(fileInfo.baseName()+"_"+parent->currentSourceLang+"-"+
                                      parent->currentTargetLang+"."+fileInfo.suffix()));
    if (f.open(QIODevice::ReadOnly | QIODevice::Text) && outF.open(QIODevice::WriteOnly | QIODevice::Text)) {
        auto cmd = new QProcess(this);
        QTextStream stream(&outF);
#ifdef Q_OS_WIN
        cmd->setWorkingDirectory(parent->appdata->absoluteFilePath("apertium-all-dev/bin"));
#endif
        while (!f.atEnd()) {
            qApp->processEvents();
            //destxt
            QString line = f.readLine();
            cmd->start("cmd.exe", QStringList() << "/c" << "echo" << "\""+line+"\"" << "|" << "apertium-destxt");
            cmd->waitForReadyRead();
            QString output = cmd->readAllStandardOutput();
            //remove quotes
            output = output.mid(1,output.lastIndexOf('\"')-1);
            cmd->waitForFinished();
            //retxt
            cmd->start("cmd.exe", QStringList() << "/c" << "echo" << "\""+notLinuxTranslate(output)+"\"" << "|" << "apertium-retxt");
            cmd->waitForReadyRead();
            output = cmd->readAllStandardOutput();
            //remove quotes
            output = output.mid(1,output.lastIndexOf('\"')-1);
            stream << output+"\n";
            cmd->waitForFinished();
        }
        cmd->deleteLater();
        outF.close();
        f.close();
        emit docTranslated(outF.fileName());
    }
    else {
        emit docTranslateRejected();
        return;
    }
}

void Translator::translateDocx(QString filePath, QDir &docDir)
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
        return;
    }
    cmd->waitForFinished();
    cmd->start(exep.absoluteFilePath("7z"), QStringList() << "x" << "-y" << fileInfo.absoluteFilePath()
               << "-o"+dir.path());
    cmd->waitForReadyRead();
    cmd->waitForFinished();

    QTemporaryFile tmpDoc("document.xml.XXXXXX");
    QFile doc(dir.path()+"/word/document.xml");
    if(!tmpDoc.open() || !doc.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit docTranslateRejected();
        return;
    }
    tmpDoc.setTextModeEnabled(true);
    QTextStream stream(&tmpDoc);
    stream.setCodec(QTextCodec::codecForName("UTF-8"));
    cmd->setWorkingDirectory(parent->appdata->absoluteFilePath("apertium-all-dev/bin"));
    QStringList args;
    while (!doc.atEnd()) {
        qApp->processEvents();
        //deswxml
        QString line = doc.readLine();
        line = line.left(line.lastIndexOf(QRegExp("[\n\t\v\f\r]+$")));
        args.clear();
        args << "/c" << "echo" << "\""+line+"\"" << "|" << "apertium-deswxml";

        cmd->setNativeArguments(args.join(' '));
        cmd->start("cmd.exe");
        cmd->waitForReadyRead();
        QString output = cmd->readAllStandardOutput();
        cmd->waitForFinished();

        //remove quotes
        output = output.mid(1,output.lastIndexOf('\"')-1);
        //rewxml
        args.clear();
        args << "/c" << "echo" << "\""+notLinuxTranslate(output)+"\"" << "|" << "apertium-rewxml";
        cmd->setNativeArguments(args.join(" "));
        cmd->start("cmd.exe");
        cmd->waitForReadyRead();
        output = cmd->readAllStandardOutput();
        //remove quotes
        output = output.mid(1,output.lastIndexOf('\"')-1);
        stream << output << endl;
        cmd->waitForFinished();
    }
    doc.close();
    tmpDoc.close();
    doc.remove();
    tmpDoc.copy(doc.fileName());
    args.clear();
    cmd->setWorkingDirectory(dir.path());
    QString trFilePath = docDir.absoluteFilePath(fileInfo.baseName()+"_"+parent->currentSourceLang
                                                 +"-"+parent->currentTargetLang+"."+fileInfo.suffix());
    args << "a" << "-y"
               << "\""+trFilePath+"\""
               << "\""+dir.path()+"/*\"";
    cmd->setNativeArguments(args.join(' '));
    cmd->start(exep.absoluteFilePath("7z"));
    cmd->waitForFinished();
    emit docTranslated(trFilePath);
    cmd->deleteLater();
//#else
//    // cmd->start("ar", QStringList() << "x" << "data.tmp");
//#endif
}

QString Translator::replaceWrongEncodings(QString src, QString tr)
{
    int iSrc1 = -1;
    int iSrc2 = -1;
    int iTr1 = -1;
    int iTr2 = -1;
    QString tmp1, tmp2;
    QRegExp regExp("\\?+");
    while(true) {
        iSrc1 = src.indexOf('\"',iSrc2+1);
        iSrc2 = src.indexOf('\"',iSrc1+1);
        iTr1 = tr.indexOf('\"',iTr2+1);
        iTr2 = tr.indexOf('\"',iTr1+1);
        if(iSrc1==-1)
            return tr;
        tmp1 = src.mid(iSrc1+1,iSrc2-iSrc1-1);
        tmp2 = tr.mid(iTr1+1,iTr2-iTr1-1);
        if (tmp2.contains(regExp) && !tmp1.contains(regExp))
            tr.replace(iTr1+1,tmp2.length(),tmp1);
    }
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
        QTemporaryFile tmpSlide(slide.fileName()+".XXXXXX");
        if(!tmpSlide.open() ||
                !slide.open(QIODevice::ReadOnly | QIODevice::Text)) {
            emit docTranslateRejected();
            return;
        }
        tmpSlide.setTextModeEnabled(true);

        QTextStream stream(&tmpSlide);
        stream.setCodec(QTextCodec::codecForName("UTF-8"));
        cmd->setWorkingDirectory(parent->appdata->absoluteFilePath("apertium-all-dev/bin"));
        while (!slide.atEnd()) {
            qApp->processEvents();
            //despptx
            QString line = slide.readLine().trimmed();
            args.clear();
            args << "/c" << "echo" << "\""+line+"\"" << "|" << "apertium-despptx";
            cmd->setNativeArguments(args.join(' '));
            cmd->start("cmd.exe");
            cmd->waitForReadyRead();
            QString output = cmd->readAllStandardOutput();
            cmd->waitForFinished();

            //remove quotes
            output = output.mid(1,output.lastIndexOf('\"')-1);
            //repptx
            args.clear();
            args << "/c" << "echo" << "\""+notLinuxTranslate(output)+"\"" << "|" << "apertium-repptx";
            cmd->setNativeArguments(args.join(" "));
            cmd->start("cmd.exe");
            cmd->waitForReadyRead();
            output = cmd->readAllStandardOutput();
            //remove quotes
            output = output.mid(1,output.lastIndexOf('\"')-1);
            stream << replaceWrongEncodings(line, output).trimmed();
            if (!slide.atEnd())
                stream << endl;
            cmd->waitForFinished();
        }
        tmpSlide.close();
        slide.close();
        slide.remove();
        tmpSlide.copy(slide.fileName());
    }
    args.clear();
    cmd->setWorkingDirectory(dir.path());
    QString trFilePath = docDir.absoluteFilePath(fileInfo.baseName()+"_"+parent->currentSourceLang
                                                 +"-"+parent->currentTargetLang+"."+fileInfo.suffix());
    QFile(trFilePath).remove();
    args << "a" << "-tzip" << "-y"
               << "\""+trFilePath+"\""
               << "\""+dir.path()+"/*\"";
    cmd->setNativeArguments(args.join(' '));
    cmd->start(exep.absoluteFilePath("7z"));
    cmd->waitForFinished();
    emit docTranslated(trFilePath);
    cmd->deleteLater();
//#else
//    // cmd->start("ar", QStringList() << "x" << "data.tmp");
//#endif
}

void Translator::docTranslate(QString filePath)
{
    QDir docDir(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    docDir.mkdir(qApp->applicationName()+"-docs");
    if (!docDir.cd(qApp->applicationName()+"-docs")) {
        qDebug() << docDir.absolutePath();
        emit docTranslateRejected();
        return;
    }
    QFileInfo fileInfo(filePath);
    if (fileInfo.suffix()=="txt")
        translateTxt(filePath, docDir);
    else
        if (fileInfo.suffix()=="docx")
            translateDocx(filePath, docDir);
        else
            if (fileInfo.suffix()=="pptx")
                translatePptx(filePath, docDir);
}

QString Translator::notLinuxTranslate(QString text)
{
    QString name = parent->currentSourceLang + "-" + parent->currentTargetLang;
    if (name.isEmpty())
        return "";

    QDir dir(parent->appdata->absoluteFilePath("usr/share/apertium/modes"));
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
    mode.replace("/usr/share", parent->appdata->absolutePath()+"/usr/share");
#ifdef Q_OS_WIN
    // Windows can't handle C:/ paths in ' quotes
    mode.replace("'", "\"");
#define OS_SEP ";"
#else
#define OS_SEP ":"
#endif

    auto env = QProcessEnvironment::systemEnvironment();
    env.insert("PATH", parent->appdata->absoluteFilePath("apertium-all-dev/bin") + OS_SEP + env.value("PATH"));
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
    connect(parent->requestSender->get(request),&QNetworkReply::finished,&loop, &QEventLoop::quit);
    loop.exec();
}
