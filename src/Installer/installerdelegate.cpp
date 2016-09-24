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
    : QStyledItemDelegate(parent)
{
}
void InstallerDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    //FIXME: bug with sorting while downloading packages
    painter->save();
    auto model = qobject_cast<const DownloadModel *>(index.model());
#ifndef Q_OS_LINUX
    if (static_cast<Column>(index.column()) == Column::SIZE
            && model->item(index.row())->state >= State::DOWNLOADING) {
        if (model->item(index.row())->state == State::DOWNLOADING) {
            int progress = model->item(index.row())->progress;
            QStyleOptionProgressBar progressBarOption;
            progressBarOption.rect = option.rect;
            progressBarOption.rect.setHeight(29);
            progressBarOption.rect.setY(option.rect.y() + (option.rect.height() -
                                                           progressBarOption.rect.height()) / 2);
            progressBarOption.minimum = 0;
            progressBarOption.maximum = model->item(index.row())->size + 1;
            progressBarOption.progress = progress;
            progressBarOption.textAlignment = Qt::AlignCenter;

            auto fsize = DownloadModel::formatBytes(progress);
            fsize = fsize.left(fsize.lastIndexOf(' '));
            auto a = fsize.toDouble();
            auto b = index.data().toString().left(index.data().toString().lastIndexOf(' ')).toDouble();
            if (a > b)
                a /= 1024;
            progressBarOption.text = QString::number(a, 'f', 2) + " / " + index.data().toString();
            progressBarOption.textVisible = true;
            QApplication::style()->drawControl(QStyle::CE_ProgressBar,
                                               &progressBarOption, painter);
        } else if (model->item(index.row())->state == State::UNPACKING) {
            QTextOption text;
            text.setAlignment(Qt::AlignCenter);
            painter->drawText(option.rect, tr("Unpacking, please wait..."), text);
        }
    } else
#endif
        if (static_cast<Column>(index.column()) == Column::STATE) {
#ifndef Q_OS_LINUX
            QStyleOptionButton button;
            button.rect = option.rect;
            button.rect.setHeight(30);
            button.rect.setY(option.rect.y() + (option.rect.height() - button.rect.height()) / 2);
            button.text = index.data().toString();
            button.features = QStyleOptionButton::DefaultButton;

            if (model->item(index.row())->state != State::UNPACKING) {
                if (option.state & QStyle::State_Active)
                    button.state = (option.state ^ QStyle::State_Active) | QStyle::State_Enabled;
                else
                    button.state = option.state | QStyle::State_Enabled;
            }

            QApplication::style()->drawControl( QStyle::CE_PushButton, &button, painter);
#else
            QStyleOptionButton checkBox;
            checkBox.rect = option.rect;
            //TODO: update from program
            checkBox.rect.setY(option.rect.y() + (option.rect.height() - checkBox.rect.height()) / 2);
            checkBox.rect.setX(option.rect.x() + option.rect.width() / 2);
            if (model->item(index.row())->state == State::UNINSTALL)
                checkBox.state = QStyle::State_Enabled | QStyle::State_On;
            else
                checkBox.state = QStyle::State_Enabled | QStyle::State_Off;
            QApplication::style()->drawControl( QStyle::CE_CheckBox, &checkBox, painter);
#endif
        } else {
            QTextOption text;
            text.setAlignment(Qt::AlignCenter);
            QRect rect = option.rect;
            if (static_cast<Column>(index.column()) == Column::NAME) {
                text.setWrapMode(QTextOption::WordWrap);
                rect.setWidth(rect.width() - 5);

                if (model->item(index.row())->highlight)
                    painter->setPen(Qt::red);
            }
            painter->drawText(rect, index.data().toString(), text);
        }
    painter->restore();
}

bool InstallerDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
                                    const QStyleOptionViewItem &,
                                    const QModelIndex &index)
{
    auto mod = qobject_cast<const DownloadModel *>(model);
    if ( event->type() == QEvent::MouseButtonRelease ) {
        if (static_cast<Column>(index.column()) == Column::STATE
                && mod->item(index.row())->state != State::UNPACKING) {
            emit stateChanged(index.row());
            return true;
        }
    }
    return false;
}

QSize InstallerDelegate::sizeHint(const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const
{
    switch (static_cast<Column>(index.column())) {
    case Column::NAME:
        return QSize(205, option.rect.height());
    case Column::TYPE:
        return QSize(70, option.rect.height());
    case Column::SIZE:
        return QSize(150, option.rect.height());
    case Column::STATE:
        return QSize(80, option.rect.height());
    }
    return QSize();
}
