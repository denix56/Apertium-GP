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

#pragma once

#include <QDialog>

#include "gpmainwindow.h"
#include "translator.h"
namespace Ui {
class FileDialog;
}

class DragnDropWidget;

class FileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FileDialog(GpMainWindow *parent = 0);

    virtual ~FileDialog() = 0;

    virtual QStringList getFileTypes() const = 0;

signals:
    void fileForTransChoosed(QString filePath);

protected slots:
    void on_browseBtn_clicked();

    void showPostFileTransDlg(QString trFilePath);

    void fileTranslateFailed();

protected:
    Ui::FileDialog *ui;
    QProgressDialog *fileTransWaitDlg = nullptr;

};

