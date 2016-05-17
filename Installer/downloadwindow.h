#ifndef DownloadWindow_H
#define DownloadWindow_H

#include "downloadmodel.h"
#include "installerdelegate.h"
#include <QDialog>
#include <QNetworkAccessManager>
#include <QProgressDialog>
namespace Ui {
class DownloadWindow;
}

class DownloadWindow : public QDialog
{
    Q_OBJECT

signals:
    void closed();
public:
    explicit DownloadWindow(QWidget *parent = 0);
    ~DownloadWindow();


private slots:
    void chooseAction(int row);
    void getData();

protected:
    void closeEvent(QCloseEvent *);

private:
    Ui::DownloadWindow *ui;
    QNetworkAccessManager *manager;
    QProgressDialog *wait;
    DownloadModel *model;
    InstallerDelegate *delegate;

    void installpkg(int row);
    void removepkg(int row);


};

#endif // DownloadWindow_H
