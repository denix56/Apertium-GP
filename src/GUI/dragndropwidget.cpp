#include "dragndropwidget.h"
#include "doctranslate.h"
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
    QFileInfo fileInfo(QString::fromUtf16((ushort*)event->mimeData()->
                                          data("application/x-qt-windows-mime;value=\"FileNameW\"").data()));
    if (fileInfo.fileName().contains(QRegExp("("+DocTranslate::fileTypes.join('|')+")")))
        event->acceptProposedAction();
}

void DragnDropWidget::dropEvent(QDropEvent *event)
{
    QPalette Pal(palette());
    // устанавливаем цвет фона
    Pal.setColor(QPalette::Background,Qt::white);
    setPalette(Pal);
    event->acceptProposedAction();
    emit documentDropped(QString::fromUtf16((ushort*)event->mimeData()->
                                            data("application/x-qt-windows-mime;value=\"FileNameW\"").data()));

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