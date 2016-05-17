
#include "initializer.h"
#include "apertiumgui.h"
#include "choosedialog.h"
#include "downloadwindow.h"
#include <QApplication>
#include <QProcess>
#include <QObject>
#include <QFile>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Initializer::getLangFullNames();
#ifdef Q_OS_LINUX
   // ChooseDialog selectPathes;
    qDebug() << Initializer::conf->value("path/serverPath").toString()
             << QFile(Initializer::conf->value("path/serverPath").toString()+"/servlet.py").exists();

    if(!QFile(Initializer::conf->value("path/serverPath").toString()+"/servlet.py").exists())
    {
        DownloadWindow dlg;
        dlg.exec();
//        QEventLoop loop;
//        QObject::connect(&selectPathes, &ChooseDialog::onDialogClose, &loop, &QEventLoop::quit);
//        selectPathes.show();
//        loop.exec();
    }
//    if(selectPathes.canceled)
//        return 0;
#endif
    ApertiumGui w;
    if (w.initialize())
        w.show();
    else
        return 2;
    return a.exec();
}
