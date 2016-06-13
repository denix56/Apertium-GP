#ifndef ChooseDialog_H
#define ChooseDialog_H

#include <QDialog>
#include<QDialogButtonBox>
namespace Ui {
class ChooseDialog;
}

//Dialog to choose right pathes to server and langpairs
class ChooseDialog : public QDialog
{
    Q_OBJECT

protected:
    void closeEvent(QCloseEvent* event);
public:
    explicit ChooseDialog(QWidget *parent = 0);
    ~ChooseDialog();
    bool canceled;
    QString serverPath, langPath;
signals:
    //when the dialog is going to close
    void onDialogClose();
private slots:
    void on_okBox_clicked(QAbstractButton *button);

    void on_serverPathBtn_clicked();

    //void on_langPathBtn_clicked();

private:
    Ui::ChooseDialog *ui;
};
#endif // ChooseDialog_H
