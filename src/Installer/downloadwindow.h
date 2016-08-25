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

#ifndef DownloadWindow_H
#define DownloadWindow_H

#include "downloadmodel.h"
#include "installerdelegate.h"
#ifdef Q_OS_LINUX
#include "managerhelper.h"
#endif
#include <QDialog>
#include <QNetworkAccessManager>
#include <QProgressDialog>
namespace Ui {
class DownloadWindow;
}

class DownloadWindow : public QDialog
{
    Q_OBJECT

signals:
    void closed();
public:
    explicit DownloadWindow(QWidget *parent = 0);
    ~DownloadWindow();

public slots:
    bool getData(bool checked = true);
    void accept();
private slots:
    void chooseAction(int row);
#ifdef Q_OS_LINUX
    bool applyChanges();
#endif
    void on_refreshButton_clicked();

protected:
    void closeEvent(QCloseEvent *);
private:
    Ui::DownloadWindow *ui;
    QNetworkAccessManager *manager;
    QProgressDialog *wait;
    DownloadModel *model;
    InstallerDelegate *delegate;

#ifndef Q_OS_LINUX
    void installpkg(int row);
    void removepkg(int row);
#else
    ManagerHelper *mngr;

    //cancel changes
    void revert();
    QVector <QString> toInstall, toUninstall;
#endif
    int actionCnt;



};

#endif // DownloadWindow_H
