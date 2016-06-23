<<<<<<< HEAD
#include "installerdelegate.h"
#include "downloadmodel.h"
#include <QApplication>
#include <QDebug>
#include <QPainter>
InstallerDelegate::InstallerDelegate(QObject *parent)
    :QStyledItemDelegate(parent)
{
}
void InstallerDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //FIXME: bug with sorting while downloading packages
    painter->save();
    auto model = qobject_cast<const DownloadModel *>(index.model());
#ifndef Q_OS_LINUX
    if (index.column() == 2 && model->item(index.row())->state>=DOWNLOADING){
        if (model->item(index.row())->state==DOWNLOADING)
        {
            int progress = model->item(index.row())->progress;
            QStyleOptionProgressBar progressBarOption;
            progressBarOption.rect = option.rect;
            progressBarOption.rect.setHeight(29);
            progressBarOption.rect.setY(option.rect.y()+(option.rect.height()-progressBarOption.rect.height())/2);
            progressBarOption.minimum = 0;
            progressBarOption.maximum = model->item(index.row())->size+1;
            progressBarOption.progress = progress;
            progressBarOption.textAlignment = Qt::AlignCenter;

            auto fsize = formatBytes(progress);
            fsize = fsize.left(fsize.lastIndexOf(' '));
            auto a = fsize.toDouble();
            auto b = index.data().toString().left(index.data().toString().lastIndexOf(' ')).toDouble();
            if (a > b)
                a /= 1024;
            progressBarOption.text = QString::number(a,'f',2) + " / " + index.data().toString();
            progressBarOption.textVisible = true;
            QApplication::style()->drawControl(QStyle::CE_ProgressBar,
                                               &progressBarOption, painter);
        }
        else
            if (model->item(index.row())->state==UNPACKING)
            {
                QTextOption text;
                text.setAlignment(Qt::AlignCenter);
                painter->drawText(option.rect,tr("Unpacking, please wait..."),text);
            }
    }
    else
#endif
        if (index.column()==3)
        {
#ifndef Q_OS_LINUX
            QStyleOptionButton button;
            button.rect=option.rect;
            button.rect.setHeight(30);
            button.rect.setY(option.rect.y()+(option.rect.height()-button.rect.height())/2);
            button.text = index.data().toString();
            button.features = QStyleOptionButton::DefaultButton;
            if (model->item(index.row())->state != UNPACKING)
                if (option.state & QStyle::State_Active)
                    button.state = option.state ^ QStyle::State_Active | QStyle::State_Enabled;
                else
                    button.state = option.state | QStyle::State_Enabled;
            QApplication::style()->drawControl( QStyle::CE_PushButton, &button, painter);
#else
            QStyleOptionButton checkBox;
            checkBox.rect = option.rect;
            //TODO: update from program
            checkBox.rect.setY(option.rect.y()+(option.rect.height()-checkBox.rect.height())/2);
            checkBox.rect.setX(option.rect.x()+option.rect.width()/2);
            if (model->item(index.row())->state == UNINSTALL)
                checkBox.state = QStyle::State_Enabled | QStyle::State_On;
            else
                checkBox.state = QStyle::State_Enabled | QStyle::State_Off;
            QApplication::style()->drawControl( QStyle::CE_CheckBox, &checkBox, painter);
#endif

        }
        else
        {
            QTextOption text;
            text.setAlignment(Qt::AlignCenter);

            if (index.column()==0 && model->item(index.row())->highlight)
                painter->setPen(Qt::red);
            painter->drawText(option.rect,index.data().toString(),text);
        }
    painter->restore();
}

bool InstallerDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    auto mod = qobject_cast<const DownloadModel *>(model);
    if( event->type() == QEvent::MouseButtonRelease )
    {
        if (index.column()==3 && mod->item(index.row())->state != UNPACKING)
        {
            emit stateChanged(index.row());
            return true;
        }
    }
    return false;
}

QSize InstallerDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    switch(index.column())
    {
    case 0:
        return QSize(160,option.rect.height());
    case 1:
        return QSize(70,option.rect.height());
    case 2:
        return QSize(150,option.rect.height());
    case 3:
        return QSize(80,option.rect.height());
    }
    return QSize();
}
//void InstallerDelegate::drawFocus(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect) const;
=======
#include "installerdelegate.h"
#include "downloadmodel.h"
#include <QApplication>
#include <QDebug>
#include <QPainter>
InstallerDelegate::InstallerDelegate(QObject *parent)
    :QStyledItemDelegate(parent)
{
}
void InstallerDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    auto model = qobject_cast<const DownloadModel *>(index.model());
#ifndef Q_OS_LINUX
    if (index.column() == 2 && model->item(index.row())->state>=DOWNLOADING){
        if (model->item(index.row())->state==DOWNLOADING)
        {
            int progress = model->item(index.row())->progress;
            QStyleOptionProgressBar progressBarOption;
            progressBarOption.rect = option.rect;
            progressBarOption.rect.setHeight(29);
            progressBarOption.rect.setY(option.rect.y()+(option.rect.height()-progressBarOption.rect.height())/2);
            progressBarOption.minimum = 0;
            progressBarOption.maximum = model->item(index.row())->size+1;
            progressBarOption.progress = progress;
            progressBarOption.textAlignment = Qt::AlignCenter;

            auto fsize = formatBytes(progress);
            fsize = fsize.left(fsize.lastIndexOf(' '));
            auto a = fsize.toDouble();
            auto b = index.data().toString().left(index.data().toString().lastIndexOf(' ')).toDouble();
            if (a > b)
                a /= 1024;
            progressBarOption.text = QString::number(a,'f',2) + " / " + index.data().toString();
            progressBarOption.textVisible = true;
            QApplication::style()->drawControl(QStyle::CE_ProgressBar,
                                               &progressBarOption, painter);
        }
        else
            if (model->item(index.row())->state==UNPACKING)
            {
                QTextOption text;
                text.setAlignment(Qt::AlignCenter);
                painter->drawText(option.rect,tr("Unpacking, please wait..."),text);
            }
    }
    else
#endif
        if (index.column()==3)
        {
#ifndef Q_OS_LINUX
            QStyleOptionButton button;
            button.rect=option.rect;
            button.rect.setHeight(30);
            button.rect.setY(option.rect.y()+(option.rect.height()-button.rect.height())/2);
            button.text = index.data().toString();
            button.features = QStyleOptionButton::DefaultButton;
            if (model->item(index.row())->state != UNPACKING)
                if (option.state & QStyle::State_Active)
                    button.state = option.state ^ QStyle::State_Active | QStyle::State_Enabled;
                else
                    button.state = option.state | QStyle::State_Enabled;
            QApplication::style()->drawControl( QStyle::CE_PushButton, &button, painter);
#else
            QStyleOptionButton checkBox;
            checkBox.rect = option.rect;
            //TODO: update from program
            checkBox.rect.setY(option.rect.y()+(option.rect.height()-checkBox.rect.height())/2);
            checkBox.rect.setX(option.rect.x()+option.rect.width()/2);
            if (model->item(index.row())->state == UNINSTALL)
                checkBox.state = QStyle::State_Enabled | QStyle::State_On;
            else
                checkBox.state = QStyle::State_Enabled | QStyle::State_Off;
            QApplication::style()->drawControl( QStyle::CE_CheckBox, &checkBox, painter);
#endif

        }
        else
        {
            QTextOption text;
            text.setAlignment(Qt::AlignCenter);

            if (index.column()==0 && model->item(index.row())->highlight)
                painter->setPen(Qt::red);
            painter->drawText(option.rect,index.data().toString(),text);
        }
    painter->restore();
}

bool InstallerDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    auto mod = qobject_cast<const DownloadModel *>(model);
    if( event->type() == QEvent::MouseButtonRelease )
    {
        if (index.column()==3 && mod->item(index.row())->state != UNPACKING)
        {
            emit stateChanged(index.row());
            return true;
        }
    }
    return false;
}

QSize InstallerDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    switch(index.column())
    {
    case 0:
        return QSize(160,option.rect.height());
    case 1:
        return QSize(70,option.rect.height());
    case 2:
        return QSize(150,option.rect.height());
    case 3:
        return QSize(80,option.rect.height());
    }
    return QSize();
}
//void InstallerDelegate::drawFocus(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect) const;
>>>>>>> a80b0491c6c41ea9ecc8150152ed6c393b815b83
