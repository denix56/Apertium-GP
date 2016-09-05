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

#ifndef TRAYWIDGET_H
#define TRAYWIDGET_H

#include <QWidget>
#include <QComboBox>

namespace Ui {
class TrayWidget;
}

class TrayWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TrayWidget(QWidget *parent = 0);

    QComboBox* inputComboBox() const;

    QComboBox* outputComboBox() const;

    ~TrayWidget();

signals:
    void prindEnded(QString text);

    void maximized();

public slots:
    void translationReceived(const QString &result);

    void setTitleBarEnabled(bool b);

    void setTransparentEnabled(bool b);

private:
    void enterEvent(QEvent *e);

    void leaveEvent(QEvent *e);

    Ui::TrayWidget *ui;

    const qreal HIDE = 0.3;
    const qreal SHOW = 1.0;
    bool transparentEnabled;
};

#endif // TRAYWIDGET_H
