
#ifndef INITIALIZER_H
#define INITIALIZER_H
#include <QMap>
#include <QStandardPaths>
#include <QSettings>
#define DATALOCATION QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)

class Initializer
{
public:
    Initializer();
    static QMap<QString, QString> langNamesMap;
    static void getLangFullNames();
    static QSettings *conf;
};

inline QString nameToFull(const QString &s)
{
    auto pair = s.mid(s.indexOf(QRegExp("(-[a-z]{2,3}){2}"))).mid(1);
    auto sourceLang = Initializer::langNamesMap[pair.left(pair.indexOf('-'))];
    auto targetLang = Initializer::langNamesMap[pair.mid(pair.indexOf('-')+1)];
    if (sourceLang.isEmpty())
        sourceLang = pair.left(pair.indexOf('-'));
    if (targetLang.isEmpty())
        targetLang = pair.mid(pair.indexOf('-')+1);
    return sourceLang+" - "+targetLang;
}

#endif // INITIALIZER_H
