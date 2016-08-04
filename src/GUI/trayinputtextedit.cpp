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

#include "trayinputtextedit.h"
#include <QKeyEvent>
#include <QClipboard>
#include <QApplication>
TrayInputTextEdit::TrayInputTextEdit(QWidget* parent)
    :InputTextEdit(parent)
{
    connect(qobject_cast<InputTextEdit *>(this),&InputTextEdit::printEnded,[&]()
    {
        emit printEnded(this->toPlainText());
    });
}

void TrayInputTextEdit::keyPressEvent(QKeyEvent *e)
{
    if(e->key()==Qt::Key_Return || e->key()==Qt::Key_Enter)
        return;
    if (e->matches(QKeySequence::Paste)) {
        QString text = QApplication::clipboard()->text();
        setPlainText(text.left(text.indexOf('\n')));
        return;
    }
    InputTextEdit::keyPressEvent(e);
}
