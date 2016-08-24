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

#include "initializer.h"
#include "gpmainwindow.h"
#include "downloadwindow.h"
#include <QApplication>
#include <QProcess>
#include <QObject>
#include <QFile>
#include <QMessageBox>
#include <QDebug>
int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    if (!Initializer::initialize())
        return 4;
#ifdef Q_OS_LINUX
    if(!QDir("/usr/share/apertium-apy").exists()) {
        QMessageBox box;
        if(box.critical(nullptr,QObject::tr("Server not installed."),
                        QObject::tr("The program cannot find Apertium-APY. Please, press Ok and install it."),
                        QMessageBox::Ok,QMessageBox::Abort)==QMessageBox::Abort)
            return 0;
        auto dlg = new DownloadWindow;
        if (dlg->getData(false))
            qDebug() << dlg->exec();
        else
            return 5;
    }
#endif
    GpMainWindow w;
    if (!w.initialize())
        return 2;
    w.show();
    return a.exec();
}
