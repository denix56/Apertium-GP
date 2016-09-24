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
  \class SettingsDialog
  \ingroup gui
  \inmodule Apertium-GP
 */
#include <QListWidget>
#include <QFileDialog>

#include "initializer.h"

#include "settingsdialog.h"
#include "ui_settingsdialog.h"

/*!
 * Constructs \l SettingsDialog and initializes fileds with vlaues from
 * configuration file.
 */
SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    connect(ui->listWidget, &QListWidget::currentRowChanged, ui->stackedWidget,
            &QStackedWidget::setCurrentIndex);

    //Font settings
    auto fontPref = new QListWidgetItem(tr("Font"), ui->listWidget);
    QFont listFont(fontPref->font());
    listFont.setPointSize(14);
    fontPref->setFont(listFont);
    connect(ui->spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, &SettingsDialog::changeFont);
    ui->spinBox->setValue(Initializer::conf->value("interface/fontsize").toInt());

    //Tray widget settings
    auto quickTr = new QListWidgetItem(tr("Quick translation"), ui->listWidget);
    quickTr->setFont(listFont);
    connect(ui->enableTrayWidget, &QCheckBox::toggled, ui->screenWidget, &QWidget::setEnabled);
    connect(ui->enableTrayWidget, &QCheckBox::toggled, ui->titleBarEnabled, &QCheckBox::setEnabled);
    connect(ui->enableTrayWidget, &QCheckBox::toggled, ui->transparentEnabled, &QCheckBox::setEnabled);

    ui->enableTrayWidget->setChecked(
        Initializer::conf->value("extra/traywidget/enabled", false).toBool());
    ui->screenWidget->setEnabled(ui->enableTrayWidget->isChecked());
    ui->titleBarEnabled->setEnabled(ui->enableTrayWidget->isChecked());
    ui->transparentEnabled->setEnabled(ui->enableTrayWidget->isChecked());

    pos_checkbox = new QMap <TrayWidget::Position, QCheckBox *> {
        { TrayWidget::TopLeft, ui->Topleft },
        { TrayWidget::TopRight, ui->TopRight },
        { TrayWidget::BottomLeft, ui->BottomLeft },
        { TrayWidget::BottomRight, ui->BottomRight }
    };

    TrayWidget::Position pos = static_cast<TrayWidget::Position>
                               (Initializer::conf->value("extra/traywidget/position").toUInt());
    recheck_checkboxes(pos);

    //TODO: optimize
    connect(ui->Topleft, &QCheckBox::toggled, [&](bool b) {
        if (b) recheck_checkboxes(TrayWidget::TopLeft);
    });
    connect(ui->TopRight, &QCheckBox::toggled, [&](bool b) {
        if (b) recheck_checkboxes(TrayWidget::TopRight);
    });
    connect(ui->BottomLeft, &QCheckBox::toggled, [&](bool b) {
        if (b) recheck_checkboxes(TrayWidget::BottomLeft);
    });
    connect(ui->BottomRight, &QCheckBox::toggled, [&](bool b) {
        if (b) recheck_checkboxes(TrayWidget::BottomRight);
    });

    ui->titleBarEnabled->setChecked(Initializer::conf->
                                    value("extra/traywidget/titlebar", false).toBool());

    ui->transparentEnabled->setChecked(Initializer::conf->
                                       value("extra/traywidget/transparent", false).toBool());

    //OCR settings
    auto ocr = new QListWidgetItem(tr("OCR"), ui->listWidget);
    ocr->setFont(listFont);
    ui->pathEdit->setText(Initializer::conf->value("extra/ocr/tessdata_path", DATALOCATION +
                                                   "/tessdata").toString());
    connect(ui->pathBtn, &QToolButton::clicked, [&]() {
        QString path = QFileDialog::getExistingDirectory(this, QString(), DATALOCATION,
                                                         QFileDialog::ShowDirsOnly);
        if (!path.isEmpty())
            ui->pathEdit->setText(path);
    });

    ui->listWidget->setCurrentRow(0);

}

/*!
 * This slot is executed when the user choosed the new position
 * for the tray widget.
 */
void SettingsDialog::recheck_checkboxes(TrayWidget::Position pos)
{
    for (auto key : pos_checkbox->keys())
        if (key == pos)
            pos_checkbox->value(key)->setChecked(true);
        else
            pos_checkbox->value(key)->setChecked(false);
}

/*!
 * This slot is executed when the user changed the font size for text boxes.
 * This functiion shows user how the new font will look like.
 */
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
    if (stdBtn == QDialogButtonBox::Apply || stdBtn == QDialogButtonBox::Ok) {
        Initializer::conf->setValue("interface/fontsize", ui->spinBox->value());
        Initializer::conf->setValue("extra/traywidget/enabled", ui->enableTrayWidget->isChecked());
        for (auto key : pos_checkbox->keys())
            if (pos_checkbox->value(key)->isChecked())
                Initializer::conf->setValue("extra/traywidget/position", static_cast<unsigned>(key));

        Initializer::conf->setValue("extra/traywidget/titlebar", ui->titleBarEnabled->isChecked());
        Initializer::conf->setValue("extra/traywidget/transparent", ui->transparentEnabled->isChecked());
        Initializer::conf->setValue("extra/ocr/tessdata_path", ui->pathEdit->text());
    }
    if (stdBtn == QDialogButtonBox::Ok)
        this->accept();
    else if (stdBtn == QDialogButtonBox::Cancel)
        this->reject();
}
