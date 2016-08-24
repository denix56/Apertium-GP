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

#ifndef DRAGNDROPWIDGET_H
#define DRAGNDROPWIDGET_H

#include <QObject>
#include <QWidget>

class DragnDropWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DragnDropWidget(QWidget *parent = 0);
protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void dragLeaveEvent(QDragLeaveEvent*);
    void paintEvent(QPaintEvent *event);
signals:
    void documentDropped(QString path);
public slots:
private:
    QString fileType;
};

#endif // DRAGNDROPWIDGET_H
