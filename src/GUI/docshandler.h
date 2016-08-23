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

#ifndef DOCSHANDLER_H
#define DOCSHANDLER_H
#include "gpmainwindow.h"
#include "dragndropwidget.h"
#include <QWidget>
namespace Ui {
class DocsHandler;
}

class DocsHandler : public QWidget
{
    Q_OBJECT

public:
    explicit DocsHandler(GpMainWindow *parent = 0);
    ~DocsHandler();
    static const QStringList fileTypes;
signals:
    void docForTransChoosed(QString filePath);
private slots:
    void on_browseBtn_clicked();
    void showPostDocTransDlg(QString trFilePath);
    void docTranslateFailed();

private:
    Ui::DocsHandler *ui;
    GpMainWindow *parent;
    QProgressDialog *docTransWaitDlg;

};

#endif // DOCTRANSLATE_H
