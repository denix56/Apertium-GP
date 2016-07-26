#ifndef NONLINUXTRANSLATOR_H
#define NONLINUXTRANSLATOR_H
#include <QObject>
#include <QNetworkRequest>
#include <QFileInfo>
#include <QProgressDialog>
class ApertiumGui;
//thread for nonlinuxtranslation
class Translator : public QObject
{
    Q_OBJECT
public:
    Translator(ApertiumGui* parent = 0);

    inline const QProgressDialog* getWaitDlg() const
    {
        return static_cast<const QProgressDialog*>(docTransWaitDlg);
    }

signals:
    void resultReady(const QString &result);

    void docTranslated(QString trFilePath);

    void docTranslateRejected();
public slots:
    //translate on other OS
    void boxTranslate();

    void docTranslate(QString filePath);

    //sent synchronous translation requests to APY on Linux
    void linuxTranslate(QNetworkRequest &request);

private:
    ApertiumGui *parent;
    QString notLinuxTranslate(QString text);
    QProgressDialog *docTransWaitDlg;

#ifndef Q_OS_LINUX
    //TODO: create one function for translating due to similar code
    void translateTxt(QString filePath, QDir &docDir);

    void translateDocx(QString filePath, QDir &docDir);

    void translatePptx(QString filePath, QDir &docDir);

    void translateHtml(QString filePath, QDir &docDir);

    void translateXlsx(QString filePath, QDir &docDir);

    void translateRtf(QString filePath, QDir &docDir);
#endif

};
#endif // NONLINUXTRANSLATOR_H
