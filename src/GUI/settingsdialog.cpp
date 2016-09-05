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

#include <QListWidget>

#include "initializer.h"

#include "settingsdialog.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    auto fontPref = new QListWidgetItem(tr("Font"), ui->listWidget);
    QFont listFont(fontPref->font());
    listFont.setPointSize(14);
    fontPref->setFont(listFont);
    connect(ui->spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, &SettingsDialog::changeFont);
    ui->spinBox->setValue(Initializer::conf->value("interface/fontsize").toInt());

    auto quickTr = new QListWidgetItem(tr("Quick translation"),ui->listWidget);
    quickTr->setFont(listFont);
    connect(ui->listWidget,&QListWidget::currentRowChanged,ui->stackedWidget,
            &QStackedWidget::setCurrentIndex);
    connect(ui->enableTrayWidget, &QCheckBox::toggled, ui->screenWidget, &QWidget::setEnabled);
    connect(ui->enableTrayWidget, &QCheckBox::toggled, ui->titleBarEnabled, &QCheckBox::setEnabled);
    connect(ui->enableTrayWidget, &QCheckBox::toggled, ui->transparentEnabled, &QCheckBox::setEnabled);
    ui->enableTrayWidget->setChecked(
                Initializer::conf->value("extra/traywidget/enabled", false).toBool());
    ui->screenWidget->setEnabled(ui->enableTrayWidget->isChecked());
    ui->titleBarEnabled->setEnabled(ui->enableTrayWidget->isChecked());
    ui->transparentEnabled->setEnabled(ui->enableTrayWidget->isChecked());

     pos_checkbox = new QMap <Position, QCheckBox*> {
        { TopLeft, ui->Topleft },
        { TopRight, ui->TopRight },
        { BottomLeft, ui->BottomLeft },
        { BottomRight, ui->BottomRight }
    };

    Position pos = static_cast<Position>(Initializer::conf->value("extra/traywidget/position").toUInt());
    recheck_checkboxes(pos);

    //TODO: optimize
    connect(ui->Topleft, &QCheckBox::toggled, [&](bool b){ if(b) recheck_checkboxes(TopLeft);});
    connect(ui->TopRight, &QCheckBox::toggled, [&](bool b){ if(b) recheck_checkboxes(TopRight);});
    connect(ui->BottomLeft, &QCheckBox::toggled, [&](bool b){ if(b) recheck_checkboxes(BottomLeft);});
    connect(ui->BottomRight, &QCheckBox::toggled, [&](bool b){ if(b) recheck_checkboxes(BottomRight);});

    ui->titleBarEnabled->setChecked(Initializer::conf->value("extra/traywidget/titlebar", false).toBool());

    ui->transparentEnabled->setChecked(Initializer::conf->value("extra/traywidget/transparent", false).toBool());
    ui->listWidget->setCurrentRow(0);

}

void SettingsDialog::recheck_checkboxes(Position pos)
{
    for(auto key : pos_checkbox->keys())
        if (key == pos)
            pos_checkbox->value(key)->setChecked(true);
        else
            pos_checkbox->value(key)->setChecked(false);
}

void SettingsDialog::changeFont(int size)
{
    QFont font(ui->textEdit->font());
    font.setPointSize(size);
    ui->textEdit->setFont(font);
    ui->textEdit->setText(ui->textEdit->toPlainText());
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::on_buttonBox_clicked(QAbstractButton *button)
{
    auto stdBtn = ui->buttonBox->standardButton(button);
    if (stdBtn==QDialogButtonBox::Apply || stdBtn==QDialogButtonBox::Ok) {
       Initializer::conf->setValue("interface/fontsize", ui->spinBox->value());
       Initializer::conf->setValue("extra/traywidget/enabled", ui->enableTrayWidget->isChecked());
       for(auto key : pos_checkbox->keys())
           if(pos_checkbox->value(key)->isChecked())
               Initializer::conf->setValue("extra/traywidget/position", static_cast<unsigned>(key));
       Initializer::conf->setValue("extra/traywidget/titlebar", ui->titleBarEnabled->isChecked());
       Initializer::conf->setValue("extra/traywidget/transparent", ui->transparentEnabled->isChecked());
       }
    if (stdBtn==QDialogButtonBox::Ok)
        this->accept();
    else
        if (stdBtn==QDialogButtonBox::Cancel)
            this->reject();
}

