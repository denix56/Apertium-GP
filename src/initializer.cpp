#include "initializer.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMessageBox>
#include <QVariant>
#include <QLocale>
#include <QDir>
#include <QProcess>
#include <QDebug>
#include <QApplication>

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
    if (!db.open())
    {
        QMessageBox box;
        box.critical(0,"Database Error", "Database is not open.");
        return false;
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
       langNamesMap[query.value("inLg").toString()] = query.value("name").toString();
       langNamesMap[query.value("iso3").toString()] = query.value("name").toString();
   } while(query.next());
   db.close();
   return true;
}
