
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


void Translator::nonLinuxTranslate() {
    auto name = parent->currentSourceLang + "-" + parent->currentTargetLang;
    if (name.isEmpty()) {
        return;
    }
    QDir dir(parent->appdata->absoluteFilePath("usr/share/apertium/modes"));
    if (!dir.exists() || !dir.exists(name+".mode")) {
        name = parent->currentSourceLang3 + "-" + parent->currentTargetLang3;
        if (!dir.exists() || !dir.exists(name+".mode"))
            return;
    }
    QFile file(dir.absoluteFilePath(name+".mode"));
    if (file.open(QIODevice::ReadOnly) == false) {
        return;
    }
    QString mode = file.readAll();
    file.close();

    mode = mode.trimmed();
    if (mode.isEmpty()) {
        return;
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
    run->write(parent->ui->boxInput->toPlainText().toUtf8()+"  ");
    run->closeWriteChannel();
    run->waitForFinished();
    auto result = QString::fromUtf8(run->readAll());
    emit resultReady(result.left(result.length()-2));
    run->deleteLater();
}

void Translator::linuxTranslate(QNetworkRequest &request)
{
    QEventLoop loop;
    connect(parent->requestSender->get(request),&QNetworkReply::finished,&loop, &QEventLoop::quit);
    loop.exec();
}
