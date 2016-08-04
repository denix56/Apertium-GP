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

    void trayResultReady(const QString &result);

    void docTranslated(QString trFilePath);

    void docTranslateRejected();
public slots:
    //translate on other OS
    void boxTranslate();

    void winTrayTranslate(QString text);

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
