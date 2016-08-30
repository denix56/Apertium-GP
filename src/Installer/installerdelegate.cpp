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

#include <QApplication>
#include <QDebug>
#include <QPainter>

#include "downloadmodel.h"

#include "installerdelegate.h"

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
    if (index.column() == SIZE && model->item(index.row())->state>=DOWNLOADING){
        if (model->item(index.row())->state==DOWNLOADING) {
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
        if (static_cast<Columns>(index.column()) == Columns::STATE) {
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
            if (model->item(index.row())->state == States::UNINSTALL)
                checkBox.state = QStyle::State_Enabled | QStyle::State_On;
            else
                checkBox.state = QStyle::State_Enabled | QStyle::State_Off;
            QApplication::style()->drawControl( QStyle::CE_CheckBox, &checkBox, painter);
#endif

        }
        else {
            QTextOption text;
            text.setAlignment(Qt::AlignCenter);

            if (static_cast<Columns>(index.column()) == Columns::NAME && model->item(index.row())->highlight)
                painter->setPen(Qt::red);
            painter->drawText(option.rect,index.data().toString(),text);
        }
    painter->restore();
}

bool InstallerDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &,
                                    const QModelIndex &index)
{
    auto mod = qobject_cast<const DownloadModel *>(model);
    if( event->type() == QEvent::MouseButtonRelease ) {
        if (static_cast<Columns>(index.column()) == Columns::STATE && mod->item(index.row())->state != States::UNPACKING) {
            emit stateChanged(index.row());
            return true;
        }
    }
    return false;
}

QSize InstallerDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    switch(static_cast<Columns>(index.column())) {
    case Columns::NAME:
        return QSize(160,option.rect.height());
    case Columns::TYPE:
        return QSize(70,option.rect.height());
    case Columns::SIZE:
        return QSize(150,option.rect.height());
    case Columns::STATE:
        return QSize(80,option.rect.height());
    }
    return QSize();
}
