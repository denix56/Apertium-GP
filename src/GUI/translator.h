#ifndef NONLINUXTRANSLATOR_H
#define NONLINUXTRANSLATOR_H
#include <QObject>
#include <QNetworkRequest>
#include <QFileInfo>
class ApertiumGui;
//thread for nonlinuxtranslation
class Translator : public QObject
{
    Q_OBJECT
public:
    Translator(ApertiumGui* parent = 0);

signals:
    void resultReady(const QString &result);
    void docTranslated(QString trFilePath);
    void docTranslateRejected();
public slots:
    //translate on other OS
    void boxTranslate();
    void docTranslate(QString filePath);
    //sent synchronuous translation requests to APY on Linux
    void linuxTranslate(QNetworkRequest &request);
private:
    ApertiumGui* parent;
    QString notLinuxTranslate(QString text);

    void translateTxt(QString filePath, QDir &docDir);

    void translateDocx(QString filePath, QDir &docDir);

    void translatePptx(QString filePath, QDir &docDir);

    void translateHtml(QString filePath, QDir &docDir);

    void translateXlsx(QString filePath, QDir &docDir);

};
#endif // NONLINUXTRANSLATOR_H
