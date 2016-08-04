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

#include "managerhelper.h"
#include <QProcess>
#include <QMessageBox>
#include <QApplication>
#include <QRegularExpression>
#include <QDebug>
ManagerHelper::ManagerHelper(QObject* parent)
    : QObject(parent)
{
    
}
 //find out current package manager
void ManagerHelper::chooseManager()
{
    QProcess cmd;
    cmd.start("which", QStringList() << "apt-get"
              << "aptitude" << "yum" << "packman" << "zypper" << "emerge");
    cmd.waitForStarted();
    while(cmd.state()==QProcess::Running)
        qApp->processEvents();
    QString answer = cmd.readAllStandardOutput();
    if (!answer.isEmpty()) {
        QString tmp = answer.split('\n')[0];
        mngr = tmp.mid(tmp.lastIndexOf('/')+1);
    }
    else {
        QMessageBox box;
        box.critical(nullptr,tr("No package manager"), tr("The program cannot find any supportted package manager. "
                                                       "Please install packages manually or contact developer "
                                                       "for adding support of your package manager. The program "
                                                       "will be closed now."));
        qApp->exit(3);      
    }
}

QString ManagerHelper::install(QStringList packages) const
{
    QStringList args;
    args << mngr;
    if (mngr == "apt-get" || mngr == "aptitude")
        args << "-y";
    else
        if (mngr == "zypper")
            args << "--non-interactive";
    else
           if (mngr == "yum" || mngr == "dnf")
               args << "--y";
    args << "install" << packages;
    return args.join(' ');
}
QString ManagerHelper::remove(QStringList packages) const
{
    QStringList args;
    args << mngr;
    if (mngr == "apt-get" || mngr == "aptitude")
        args << "-y";
    else
        if (mngr == "zypper")
            args << "--non-interactive";
    else
           if (mngr == "yum" || mngr == "dnf")
               args << "--y";
    args << "remove" << packages;
    return args.join(' ');
}

QString ManagerHelper::update() const
{
    QStringList args;
    args << mngr;
    if (mngr == "apt-get" || mngr == "aptitude")
        args << "update";
    else
        if (mngr == "zypper")
            args << "refresh";
        else
            if (mngr == "yum" || mngr == "dnf")
                args << "check-update";
    return args.join(' ');
}

QString ManagerHelper::search(QString package) const
{
    QStringList args;
    args << (mngr=="apt-get" ? "apt-cache" : mngr) << "search" << package;
    return args.join(' ');
}

QString ManagerHelper::getManager() const
{
    return mngr;
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
    QStringList args;
    args << (mngr=="apt-get" ? "apt-cache" : mngr);
    QProcess cmd;
    unsigned long long size = 0;
    if (mngr == "apt-get" || mngr == "aptitude") {
        args << "show" << package;
        cmd.start(args.join(' '));
        cmd.waitForStarted();
        while(cmd.state()==QProcess::Running)
            qApp->processEvents();
        if(cmd.exitCode())
            return 0;
        auto outpk = QString(cmd.readAllStandardOutput()).split('\n');
        for(auto str: outpk) {
            if(str.contains(QRegularExpression("^Size"))) {
                QRegularExpression sizeReg("[0-9]+");
                auto jt = sizeReg.match(str);
                if(jt.hasMatch())
                    size = jt.captured().toULongLong();
            }
        }
    }
    else
        if (mngr == "zypper") {
            args << "info" << package;
            cmd.start(args.join(' '));
            cmd.waitForStarted();
            while(cmd.state()==QProcess::Running)
                qApp->processEvents();
            if(cmd.exitCode())
                return 0;
            auto outpk = QString(cmd.readAllStandardOutput()).split('\n');
            for(auto str : outpk) {
                if (str.contains(QRegularExpression("^Size"))) {
                    QRegularExpression sizeReg("[0-9]+\\.?[0-9]+");
                    unsigned long long size = 0;
                    auto jt = sizeReg.match(str);
                    if(jt.hasMatch()) {
                        size = (unsigned long long)(jt.captured().toDouble() * 1024ULL);
                        if(str.right(3)=="MiB")
                            size *= 1024ULL;
                    }
                }
            }
        }
        else
            //TODO: finish
            if (mngr == "yum" || mngr == "dnf")
                args << "check-update";
    return size;
}
