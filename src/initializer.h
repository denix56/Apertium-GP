
#ifndef INITIALIZER_H
#define INITIALIZER_H
#include <QMap>
#include <QStandardPaths>
#include <QSettings>
#include <QDir>
//#ifndef DATALOCATION
//    QDir tmp;
//    tmp.cd("~");
//    QString pth;
//    if ((tmp(".local").exists()))
//        pth = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
//    else
//        pth = QStandardPaths::writableLocation(QStandardPaths::HomeLocation)+".apertium-gp";
// DATALOCATION pth;
//    qDebug() << DATALOCATION;
//#endif

#define DATALOCATION  QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).isEmpty() \
    ? QStandardPaths::writableLocation(QStandardPaths::HomeLocation)+".apertium-gp" \
    : QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)


class Initializer
{
public:
    static QMap<QString, QString> langNamesMap;
    static QSettings *conf;
    static bool initialize();
};

inline QString nameToFull(const QString &s)
{
    auto pair = s.mid(s.indexOf(QRegExp("(-[a-z]{2,3}){2}"))).mid(1);
    auto sourceLang = Initializer::langNamesMap[pair.left(pair.indexOf('-'))];
    auto targetLang = Initializer::langNamesMap[pair.mid(pair.indexOf('-')+1)];
    //qDebug() << sourceLang << targetLang;
    if (sourceLang.isEmpty())
        sourceLang = pair.left(pair.indexOf('-'));
    if (targetLang.isEmpty())
        targetLang = pair.mid(pair.indexOf('-')+1);
    return sourceLang+" - "+targetLang;
}

#endif // INITIALIZER_H
