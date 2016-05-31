
#include "initializer.h"
#include "apertiumgui.h"
#include "choosedialog.h"
#include "downloadwindow.h"
#include <QApplication>
#include <QProcess>
#include <QObject>
#include <QFile>
#include <QDebug>
#include <QMessageBox>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Initializer::getLangFullNames();
#ifdef Q_OS_LINUX
    if(!Initializer::conf->contains("path/langPath") ||
            Initializer::conf->value("path/langPath").toString().isEmpty())
        Initializer::conf->setValue("path/langPath", QVariant("/usr/share/apertium"));

    if(!QFile(Initializer::conf->value("path/serverPath").toString()+"/servlet.py").exists())
    {
        if(QFile(DATALOCATION+"/apertium-apy/apertium-apy/servlet.py").exists())
            Initializer::conf->setValue("path/serverPath",QVariant(DATALOCATION+"/apertium-apy/apertium-apy"));
        else {
            QMessageBox box;
            if(box.critical(nullptr,QObject::tr("Server not installed."),
                            QObject::tr("The program cannot find Apertium-APY. Please, press Ok and install it."),
                            QMessageBox::Ok,QMessageBox::Abort)==QMessageBox::Abort)
                return 0;
            auto dlg = new DownloadWindow;
            if (dlg->getData(false))
                qDebug() << dlg->exec();
        }
    }
#endif
    ApertiumGui w;
    if (!w.initialize())
        return 2;
    w.show();
    return a.exec();
}
