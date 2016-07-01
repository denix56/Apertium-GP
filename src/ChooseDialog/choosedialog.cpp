
#include "choosedialog.h"
#include "ui_choosedialog.h"
#include "initializer.h"
#include <QCloseEvent>
#include <QFileDialog>
#include <QApplication>
#include <QPushButton>
#include <QMessageBox>
ChooseDialog::ChooseDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChooseDialog)
{
    ui->setupUi(this);
    canceled=false;
    if (QFile(Initializer::conf->fileName()).exists())
    {
        ui->serverPathEdit->setText(Initializer::conf->value("path/serverPath").toString());
    }
}

ChooseDialog::~ChooseDialog()
{
    delete ui;
}
void ChooseDialog::closeEvent(QCloseEvent* event)
{
    if (ui->serverPathEdit->text().isEmpty())
        event->ignore();
    else
    {
        serverPath = ui->serverPathEdit->text();
        emit onDialogClose();
    }
}

//open QFileDialog to choose server path
void ChooseDialog::on_serverPathBtn_clicked()
{
    auto path = QFileDialog::getExistingDirectory(this, tr("Open Directory"),".\\",
                                                     QFileDialog::ShowDirsOnly
                                                     | QFileDialog::DontResolveSymlinks);
    if (!path.isEmpty())
        ui->serverPathEdit->setText(path);

    ui->okBox->button(QDialogButtonBox::Ok)->setFocus();
    ui->okBox->button(QDialogButtonBox::Ok)->setDefault(true);
    Initializer::conf->setValue("path/serverPath", QVariant(ui->serverPathEdit->text()));
}

//open QFileDialog to choose langpairs path
//void ChooseDialog::on_langPathBtn_clicked()
//{
//    auto path = QFileDialog::getExistingDirectory(this, tr("Open Directory"),".\\",
//                                                     QFileDialog::ShowDirsOnly
//                                                     | QFileDialog::DontResolveSymlinks);
//    if(!path.isEmpty())
//        ui->langPathEdit->setText(path);
//    ui->okBox->button(QDialogButtonBox::Ok)->setFocus();
//    ui->okBox->button(QDialogButtonBox::Ok)->setDefault(true);
//    Initializer::conf->setValue("path/langPath", QVariant(ui->langPathEdit->text()));
//}

void ChooseDialog::on_okBox_clicked(QAbstractButton *button)
{
    if (button==ui->okBox->button(QDialogButtonBox::Ok))
    {
        if (!QFile(ui->serverPathEdit->text()+"/servlet.py").exists())
        {
            QMessageBox box;
            box.critical(this,"Path error","Invalid server path. File servlet.py has not been found.");
            return;
        }
        close();
    }
    else
        //when the dialog is going to close
       if (button==ui->okBox->button(QDialogButtonBox::Cancel))
       {
           canceled = true;
           qApp->quit();
       }
}
