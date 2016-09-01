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
    connect(this, &ManagerHelper::canceled, cmd, &QProcess::kill);
}

int ManagerHelper::installRemove(const QStringList &packagesInstall, const QStringList &packagesRemove) const
{
    QStringList args;
    args <<"pkexec" << scriptPath;
    if (!packagesInstall.isEmpty())
        args << "--install" << packagesInstall;

    if (!packagesRemove.isEmpty())
        args << "--remove" << packagesRemove;
    cmd->start(args.join(' '));
    cmd->waitForStarted();
    while(cmd->state()==QProcess::Running)
        qApp->processEvents();
    cmd->waitForFinished();
    return cmd->exitCode();
}

int ManagerHelper::update() const
{  
    cmd->start("pkexec", QStringList() << scriptPath << "--update");
    cmd->waitForStarted();
    while(cmd->state()==QProcess::Running)
        qApp->processEvents();
    cmd->waitForFinished();
    return cmd->exitCode();
}

QString ManagerHelper::search(const QString &package) const
{
    cmd->start("pkexec", QStringList() << scriptPath << "--search" << package);
    cmd->waitForStarted();
    while(cmd->state()==QProcess::Running)
        qApp->processEvents();
    cmd->waitForFinished();
    return cmd->readAllStandardOutput();
}

//TODO: optimize for asking info about multiple packages
unsigned long long ManagerHelper::getSize(const QString &package) const
{
    cmd->start("pkexec", QStringList() << scriptPath << "--info" << package);
    cmd->waitForStarted();
    while(cmd->state()==QProcess::Running)
        qApp->processEvents();

    if(cmd->exitCode())
        return 0;

    return cmd->readAllStandardOutput().toULongLong();
}
