/*
* Copyright (C) 2016, Denys Senkin <denisx9.0c@gmail.com>
*
* This file is part of apertium-gp
*
* apertium-gp is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* apertium-gp is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with apertium-gp.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef INITIALIZER_H
#define INITIALIZER_H
#include <QMap>
#include <QStandardPaths>
#include <QSettings>
#include <QDir>

#define DATALOCATION  QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)

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
