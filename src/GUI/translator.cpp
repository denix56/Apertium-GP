
#include "translator.h"
#include "apertiumgui.h"
#include "ui_apertiumgui.h"
#include <QDir>
#include <QProcess>
#include <QRegularExpression>
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

void Translator::docTranslate(QString filePath)
{
    QFileInfo fileInfo(filePath);
    QDir docDir(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    docDir.mkdir(qApp->applicationName());
    if (!docDir.cd(qApp->applicationName())) {
        qDebug() << docDir.absolutePath();
        emit docTranslateRejected();
        return;
    }
    QFile outF(docDir.absoluteFilePath(fileInfo.baseName()+"_"+parent->currentSourceLang+"-"+
                                      parent->currentTargetLang+"."+fileInfo.suffix()));
    if (fileInfo.suffix()=="txt") {
        QFile f(filePath);

        if (f.open(QIODevice::ReadOnly | QIODevice::Text) && outF.open(QIODevice::WriteOnly | QIODevice::Text)) {
            auto cmd = new QProcess(this);
#ifdef Q_OS_WIN
            cmd->setWorkingDirectory(parent->appdata->absoluteFilePath("apertium-all-dev/bin"));
#endif
            while (!f.atEnd()) {
                qApp->processEvents();
                QString line = f.readLine();
                cmd->start("echo", QStringList() << "\""+line+"\"" << "|" << "apertium-destxt");
                cmd->waitForReadyRead();
                QString output = cmd->readAllStandardOutput();
                output = output.mid(1,output.lastIndexOf('\"')-1);
                qDebug() << output;
                cmd->waitForFinished();
                cmd->start("echo", QStringList() << "\""+notLinuxTranslate(output)+"\"" << "|" << "apertium-retxt");
                cmd->waitForReadyRead();
                output = cmd->readAllStandardOutput();
                output = output.mid(1,output.lastIndexOf('\"')-1);
                outF.write(output.toUtf8()+"\n");
                cmd->waitForFinished();
            }
            cmd->deleteLater();
            outF.close();
            f.close();
        }
        else {
            emit docTranslateRejected();
            return;
        }
    }
    emit docTranslated(outF.fileName());
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
