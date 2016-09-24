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


#include <QFrame>
#include <QDebug>


#include "traywidget.h"
#include "ui_traywidget.h"

TrayWidget::TrayWidget(QWidget *parent) :
    QWidget(parent, Qt::ToolTip | Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint),
    ui(new Ui::TrayWidget)
{
    ui->setupUi(this);
    connect(ui->textEdit, &TrayInputTextEdit::printEnded, this, &TrayWidget::prindEnded);
    connect(ui->closeBtn_3, &QPushButton::clicked, qApp, &QApplication::quit);
    connect(ui->maxBtn_3, &QPushButton::clicked, this, &TrayWidget::maximized);
}


void TrayWidget::translationReceived(const QString &result)
{
    ui->textEdit_2->clear();
    auto cursor = ui->textEdit_2->textCursor();
    cursor.movePosition(QTextCursor::Start);
    auto format = cursor.charFormat();
    format.setForeground(Qt::black);
    cursor.insertText(result, format);
    cursor.movePosition(QTextCursor::Start);
    while (!cursor.atEnd()) {
        auto cursor1 = cursor.document()->
                       find(QRegExp("[\\*#]\\w+\\W?"), cursor);
        if (cursor1.isNull())
            break;
        cursor = cursor1;
        auto format = cursor.charFormat();
        format.setForeground(Qt::red);
        cursor.insertText(cursor.selectedText().mid(1), format);
    }
    ui->textEdit_2->setTextCursor(cursor);
}

QComboBox *TrayWidget::inputComboBox() const
{
    return ui->comboBox;
}

QComboBox *TrayWidget::outputComboBox() const
{
    return ui->comboBox_2;
}
TrayWidget::~TrayWidget()
{
    delete ui;
}

void TrayWidget::setTitleBarEnabled(bool b)
{
    if (b) {
        ui->closeBtn_3->show();
        ui->maxBtn_3->show();
        setFixedHeight(151);
    } else {
        ui->closeBtn_3->hide();
        ui->maxBtn_3->hide();
        setFixedHeight(131);
    }
}

void TrayWidget::setTransparentEnabled(bool b)
{
    transparentEnabled = b;
    setWindowOpacity(b ? HIDE : SHOW);
}

void TrayWidget::enterEvent(QEvent *e)
{
    if (transparentEnabled)
        setWindowOpacity(SHOW);
    QWidget::enterEvent(e);
}

void TrayWidget::leaveEvent(QEvent *e)
{
    if (transparentEnabled)
        setWindowOpacity(HIDE);
    QWidget::leaveEvent(e);
}
