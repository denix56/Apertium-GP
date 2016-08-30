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

#include <QMessageBox>
#include <QApplication>
#include <QDebug>

#include "initializer.h"

#include "managerhelper.h"

ManagerHelper::ManagerHelper(QObject* parent)
    : QObject(parent)
{
    cmd = new QProcess(this);
    connect(this, &ManagerHelper::canceled, cmd, &QProcess::terminate);
}
// //find out current package manager
//void ManagerHelper::chooseManager()
//{
//
//    cmd->start("which", QStringList() << "apt-get"
//              << "aptitude" << "yum" << "packman" << "zypper" << "emerge");
//    cmd->waitForStarted();
//    while(cmd->state()==QProcess::Running)
//        qApp->processEvents();
//    QString answer = cmd->readAllStandardOutput();
//    if (!answer.isEmpty()) {
//        QString tmp = answer.split('\n')[0];
//        mngr = tmp.mid(tmp.lastIndexOf('/')+1);
//    }
//    else {
//        QMessageBox box;
//        box.critical(nullptr,tr("No package manager"), tr("The program cannot find any supportted package manager. "
//                                                       "Please install packages manually or contact developer "
//                                                       "for adding support of your package manager. The program "
//                                                       "will be closed now."));
//        qApp->exit(3);
//    }
//}

int ManagerHelper::installRemove(const QStringList &packagesInstall, const QStringList &packagesRemove) const
{
    QStringList args;
    args << scriptPath;
    if (!packagesInstall.isEmpty())
        args << "-i" << "\"" + packagesInstall.join(' ') + "\"";

    if (!packagesRemove.isEmpty())
        args << "-r" << "\"" + packagesRemove.join(' ') + "\"";
    cmd->start("pkexec "+args.join(' '));
    cmd->waitForStarted();
    while(cmd->state()==QProcess::Running)
        qApp->processEvents();
    cmd->waitForFinished();
    return cmd->exitCode();
}
//int ManagerHelper::remove(QStringList packages) const
//{
//    cmd->start("pkexec", QStringList() << scriptPath << "-r" << "\""+packages.join(' ')+"\"");
//    cmd->waitForStarted();
//    while(cmd->state()==QProcess::Running)
//        qApp->processEvents();
//    cmd->waitForFinished();
//    return cmd->exitCode();
//}

int ManagerHelper::update() const
{  
    cmd->start("pkexec", QStringList() << scriptPath << "-u");
    cmd->waitForStarted();
    while(cmd->state()==QProcess::Running)
        qApp->processEvents();
    cmd->waitForFinished();
    return cmd->exitCode();
}

QString ManagerHelper::search(const QString &package) const
{
    cmd->start("pkexec", QStringList() << scriptPath << "-S" << package);
    cmd->waitForStarted();
    while(cmd->state()==QProcess::Running)
        qApp->processEvents();
    cmd->waitForFinished();
    return cmd->readAllStandardOutput();
}

//QStringList ManagerHelper::info(QString package) const
//{
//    QStringList args;
//    args << (mngr=="apt-get" ? "apt-cache" : mngr);
//    if (mngr == "apt-get" || mngr == "aptitude") {
//        args << "show" << package << " | grep \"Package|^Size\"";
//    }
//    else
//        if (mngr == "zypper" || mngr == "yum" || mngr == "dnf")
//            args << "info";
//    return args.join(' ');
//}

//TODO: optimize for asking info about multiple packages
unsigned long long ManagerHelper::getSize(const QString &package) const
{

    cmd->start("pkexec", QStringList() << scriptPath << "-I" << package);
    cmd->waitForStarted();
    while(cmd->state()==QProcess::Running)
        qApp->processEvents();
    if(cmd->exitCode())
        return 0;

    QString x = cmd->readAllStandardOutput();
    return (unsigned long long)x.left(x.indexOf('\n')).toDouble();
}
