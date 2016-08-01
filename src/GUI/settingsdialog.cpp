#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "initializer.h"
#include <QStackedLayout>
#include <QGridLayout>
#include <QListWidget>

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
    connect(ui->listWidget,&QListWidget::currentRowChanged,ui->stackedWidget,
            &QStackedWidget::setCurrentIndex);
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
       ApertiumGui *parent = qobject_cast<ApertiumGui*>(this->parent());
       parent->setFontSize(ui->spinBox->value());
    }
    if (stdBtn==QDialogButtonBox::Ok)
        this->accept();
    else
        if (stdBtn==QDialogButtonBox::Cancel)
            this->reject();
}
