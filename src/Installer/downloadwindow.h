#ifndef DownloadWindow_H
#define DownloadWindow_H

#include "downloadmodel.h"
#include "installerdelegate.h"
#include "managerhelper.h"
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
    bool getData(bool checked = true);
    void accept();
private slots:
    void chooseAction(int row);

    void unpack(QNetworkReply *reply);
#ifdef Q_OS_LINUX
    bool applyChanges();
#endif
    void on_refreshButton_clicked();

protected:
    void closeEvent(QCloseEvent *);
private:
    Ui::DownloadWindow *ui;
    QNetworkAccessManager *manager;
    QProgressDialog *wait;
    DownloadModel *model;
    InstallerDelegate *delegate;
    ManagerHelper *mngr;

#ifndef Q_OS_LINUX
    void installpkg(int row);
    void removepkg(int row);
#else
    //cancel changes
    void revert();

    QVector <QString> toInstall, toUninstall;
#endif
    int actionCnt;



};

#endif // DownloadWindow_H
