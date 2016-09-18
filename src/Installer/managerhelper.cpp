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
        args << "--install" << "\"" + packagesInstall.join(' ') + "\"";

    if (!packagesRemove.isEmpty())
        args << "--remove" << "\"" + packagesRemove.join(' ') + "\"";
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

QString ManagerHelper::search(const QStringList &names) const
{
    cmd->start("pkexec", QStringList() << scriptPath << "--search" << names.join('|'));
    cmd->waitForStarted();
    while(cmd->state()==QProcess::Running)
        qApp->processEvents();
    cmd->waitForFinished();
    return cmd->readAllStandardOutput();
}

QString ManagerHelper::getInfo(const QString &packages) const
{
    cmd->start("pkexec "+ scriptPath + " --info \"" + packages + "\"");
    cmd->waitForStarted();
    while(cmd->state()==QProcess::Running)
        qApp->processEvents();

    if(cmd->exitCode())
        return 0;
    return cmd->readAllStandardOutput();
}
