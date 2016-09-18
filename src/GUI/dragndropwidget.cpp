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

/*!
  \class DragnDropWidget
  \ingroup gui
  \inmodule Apertium-GP

  \brief Class provides widget for Drag`n`Drop function

  It allows drop documents on itself for user. After dropping it emmits signal
  \l documentDropped(QString path) with path to dropped document.
 */

#include <QDragMoveEvent>
#include <QMimeData>
#include <QDrag>
#include <QFileInfo>
#include <QPainter>
#include <QRect>
#include <QDebug>


#include "dragndropwidget.h"

/*!
 * \brief Constructs \l DragnDropWidget with parent \a parent
 *
 * It also sets QWidget::acceptDrops() to true to enable Drag`n`Drop
 */
DragnDropWidget::DragnDropWidget(QWidget *parent)
    : QWidget(parent)
{
    this->parent = qobject_cast<FileDialog*>(parent);
    setAcceptDrops(true);
}

/*!
 * \brief Handles dragging document event \a event on this widget and checks document format
 * using \l FileDialog::fileTypes. The color of the widget is changed.
 */
void DragnDropWidget::dragEnterEvent(QDragEnterEvent *event)
{
    QPalette Pal(palette());
    // set background color
    Pal.setColor(QPalette::Background,QColor(20,187,229,20));
    setPalette(Pal);
#ifdef Q_OS_WIN
    QFileInfo fileInfo(QString::fromUtf16((ushort*)event->mimeData()->
                                          data("application/x-qt-windows-mime;value=\"FileNameW\"").data()));
#elif defined(Q_OS_LINUX)
    QFileInfo fileInfo(QUrl::fromPercentEncoding(
                           event->mimeData()->data("text/uri-list").trimmed()).remove("file:"));
#endif
    if (fileInfo.fileName().contains(QRegExp("("+parent->getFileTypes().join('|')+")")))
        event->acceptProposedAction();
}

/*!
 * \fn DragnDropWidget::documentDropped(QString path)
 *
 * This signal is emiited when the \l DragnDropWidget::dropEvent(QDropEvent *event)
 * finished successfully. It returns \a path to document that was dropped.
 */

/*!
 * \brief Gets path of dropped document from \a event and emits \l documentDropped(QString path)
 * The color of the widget is changed.
 */
void DragnDropWidget::dropEvent(QDropEvent *event)
{
    QPalette Pal(palette());
    // set background color
    Pal.setColor(QPalette::Background,Qt::white);
    setPalette(Pal);
    event->acceptProposedAction();
    QString path;
#ifdef Q_OS_WIN
    path = QString::fromUtf16((ushort*)event->mimeData()->
                              data("application/x-qt-windows-mime;value=\"FileNameW\"").data());
#elif defined(Q_OS_LINUX)
    path = QUrl::fromPercentEncoding(
               event->mimeData()->data("text/uri-list").trimmed()).remove("file:");
#endif
    emit fileDropped(path);
}

/*!
 * \brief Changes color to default if drag leaved this widget.
 */
void DragnDropWidget::dragLeaveEvent(QDragLeaveEvent *)
{
    QPalette Pal(palette());
    // set background color
    Pal.setColor(QPalette::Background, Qt::white);
    setPalette(Pal);
}

/*!
 * \brief Paint border around the widget and handles \a event.
 */
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
