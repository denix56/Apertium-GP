#ifndef NONLINUXTRANSLATOR_H
#define NONLINUXTRANSLATOR_H
#include <QObject>
#include <QNetworkRequest>
class ApertiumGui;
//thread for nonlinuxtranslation
class Translator : public QObject
{
    Q_OBJECT
public:
    Translator(ApertiumGui* parent = 0);

signals:
    void resultReady(const QString &result);
public slots:
    //translate on other OS
    void nonLinuxTranslate();
    //sent synchronuous translation requests to APY on Linux
    void linuxTranslate(QNetworkRequest &request);
private:
    ApertiumGui* parent;
};
#endif // NONLINUXTRANSLATOR_H
