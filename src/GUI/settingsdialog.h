#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include "apertiumgui.h"

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();

private slots:
    void on_buttonBox_clicked(QAbstractButton *button);

    void changeFont(int size);

private:
    Ui::SettingsDialog *ui;
};

#endif // SETTINGSDIALOG_H
