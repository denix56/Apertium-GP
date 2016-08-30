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

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMessageBox>
#include <QVariant>
#include <QLocale>
#include <QDir>
#include <QProcess>
#include <QDebug>
#include <QApplication>

#include "initializer.h"

QMap<QString, QString> Initializer::langNamesMap;
QSettings *Initializer::conf;
bool Initializer::initialize()
{
    conf = new QSettings(QSettings::NativeFormat,QSettings::UserScope,"Apertium","Apertium-GP");
#ifdef Q_OS_LINUX
    QDir path("/usr/share/apertium-gp");
#else
    qDebug() << DATALOCATION;
    QDir path(DATALOCATION);
#endif
    auto db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(path.absoluteFilePath("langNames.db"));
    if (!db.open()) {
        QMessageBox box;
        box.critical(0,"Database Error", "Database is not open.");
        return false;
    }
    QSqlQuery query;
    query.exec("SELECT * FROM languageNames WHERE lg = '"
               +QLocale::system().name().left(QLocale::system().name().indexOf('_'))+"'");
   if (!query.next()) {
        query.exec("SELECT * FROM languageNames WHERE lg = 'en'");
        query.next();
   }
    do {
       langNamesMap[query.value("inLg").toString()] = query.value("name").toString();
       langNamesMap[query.value("iso3").toString()] = query.value("name").toString();
   } while(query.next());
   db.close();
   return true;
}
