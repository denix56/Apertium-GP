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
  \class InputTextEdit
  \ingroup gui
  \inmodule Apertium-GP
  \brief Class that provides special behavior for input text box.

  The program waits until the user stopped printing input text, because
  it is unnecessary to send translation each time.
  It is using \l QTimer to wait after the user has stopped printing. If the user will not print anymore,
  it will emit signal that the printing ended.
  */
/*!
 * \fn InputTextEdit::printEnded()
 * \brief This signal is emmited when the user stopped printing text for some time.
 */
#include <QKeyEvent>

#include "inputtextedit.h"

InputTextEdit::InputTextEdit(QWidget* parent)
    : QTextEdit(parent)
{
    connect(&timer,&QTimer::timeout,this,&InputTextEdit::printEnded);
    connect(this,&InputTextEdit::textChanged,[&]() {
        timer.start(250);
    });
    timer.setSingleShot(true);
}
