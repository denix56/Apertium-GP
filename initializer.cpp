#include "initializer.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMessageBox>
#include <QVariant>
#include <QLocale>
#include <QDir>

Initializer::Initializer()
{

}

QMap<QString, QString> Initializer::langNamesMap;
QSettings *Initializer::conf;
void Initializer::getLangFullNames()
{
    conf = new QSettings(QSettings::NativeFormat,QSettings::UserScope,"Apertium","Apertium-GP");
    auto db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(QDir::currentPath()+"/langNames.db");
    if (!db.open())
    {
        QMessageBox box;
        box.critical(0,"Database Error", "Database is not open.");
        return;
    }
    QSqlQuery query;
    query.exec("SELECT * FROM languageNames WHERE lg = '"
               +QLocale::system().name().left(QLocale::system().name().indexOf('_'))+"'");
   if (!query.next())
   {
        query.exec("SELECT * FROM languageNames WHERE lg = 'en'");
        query.next();
   }
    do
    {
#ifndef Q_OS_LINUX
        langNamesMap[query.value("inLg").toString()] = query.value("name").toString();
#endif
        langNamesMap[query.value("iso3").toString()] = query.value("name").toString();
    } while(query.next());
    db.close();
}
