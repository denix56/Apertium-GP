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
#include <QtGlobal>
#include <QHash>
#include <memory>

#if QT_VERSION >= 0x050400
    #define DATALOCATION QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
#else
    #define DATALOCATION QStandardPaths::writableLocation(QStandardPaths::DataLocation)
#endif
const QString scriptPath = "/usr/share/apertium-gp/apertium-gp-helper.pl";

enum Position { TopLeft, TopRight, BottomLeft, BottomRight };
Q_DECLARE_METATYPE(Position)

class Initializer
{
public:
    static QHash<QString, QString> langNamesMap;
    static std::unique_ptr<QSettings> conf;
    static bool initialize();
};


#endif // INITIALIZER_H
