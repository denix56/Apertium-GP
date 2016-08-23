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

#include "dragndropwidget.h"
#include "docshandler.h"
#include <QDragMoveEvent>
#include <QMimeData>
#include <QDrag>
#include <QFileInfo>
#include <QPainter>
#include <QRect>
#include <QDebug>
DragnDropWidget::DragnDropWidget(QWidget *parent) : QWidget(parent)
{
    setAcceptDrops(true);
}

void DragnDropWidget::dragEnterEvent(QDragEnterEvent *event)
{
    QPalette Pal(palette());
    // устанавливаем цвет фона
    Pal.setColor(QPalette::Background,QColor(20,187,229,20));
    setPalette(Pal);
#ifdef Q_OS_WIN
    QFileInfo fileInfo(QString::fromUtf16((ushort*)event->mimeData()->
                                          data("application/x-qt-windows-mime;value=\"FileNameW\"").data()));
#elif defined(Q_OS_LINUX)
    QFileInfo fileInfo(QUrl::fromPercentEncoding(event->mimeData()->data("text/uri-list").trimmed()).remove("file:"));
#endif
    if (fileInfo.fileName().contains(QRegExp("("+DocsHandler::fileTypes.join('|')+")")))
        event->acceptProposedAction();
}

void DragnDropWidget::dropEvent(QDropEvent *event)
{
    QPalette Pal(palette());
    // устанавливаем цвет фона
    Pal.setColor(QPalette::Background,Qt::white);
    setPalette(Pal);
    event->acceptProposedAction();
    QString path;
#ifdef Q_OS_WIN
    path = QString::fromUtf16((ushort*)event->mimeData()->
                              data("application/x-qt-windows-mime;value=\"FileNameW\"").data());
#elif defined(Q_OS_LINUX)
    path = QUrl::fromPercentEncoding(event->mimeData()->data("text/uri-list").trimmed()).remove("file:");
#endif
    emit documentDropped(path);

}

void DragnDropWidget::dragLeaveEvent(QDragLeaveEvent *event)
{
    QPalette Pal(palette());
    // устанавливаем цвет фона
    Pal.setColor(QPalette::Background,Qt::white);
    setPalette(Pal);
}

void DragnDropWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setPen(Qt::DashLine);
    QRect rectangle(rect());
    const int offset = 1;
    painter.drawRect(rectangle.x()+offset,rectangle.y()+offset,
                     rectangle.width()-2*offset,rectangle.height()-2*offset);
    QWidget::paintEvent(event);
}
