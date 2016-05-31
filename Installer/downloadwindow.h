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

public slots:
    bool getData(bool checked);
    void accept();
private slots:
    void chooseAction(int row);
    bool applyChanges();

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
    void revert();
    QVector <QString> toInstall, toUninstall;


};

#endif // DownloadWindow_H
