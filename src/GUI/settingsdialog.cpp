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

#include <QStackedLayout>
#include <QGridLayout>
#include <QListWidget>

#include "initializer.h"

#include "settingsdialog.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    auto fontPref = new QListWidgetItem(tr("Font"),ui->listWidget);
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
    ui->enableTrayWidget->setChecked(
                Initializer::conf->value("extra/traywidget",ui->enableTrayWidget->isChecked()).toBool());
    connect(ui->enableTrayWidget,&QCheckBox::clicked,[&](bool checked)
    {
        Initializer::conf->setValue("extra/traywidget",checked);
    });
    ui->listWidget->setCurrentRow(0);

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
    if (stdBtn==QDialogButtonBox::Apply
            || stdBtn==QDialogButtonBox::Ok) {
       GpMainWindow *parent = qobject_cast<GpMainWindow*>(this->parent());
       parent->setFontSize(ui->spinBox->value());
       parent->setTrayWidgetEnabled(ui->enableTrayWidget->isChecked());
    }
    if (stdBtn==QDialogButtonBox::Ok)
        this->accept();
    else
        if (stdBtn==QDialogButtonBox::Cancel)
            this->reject();
}
