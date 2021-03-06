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

#pragma once

#include <QMainWindow>
#include <QTextEdit>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QProcess>
#include <QMap>
#include <QStandardPaths>
#include <QThread>
#include <QTableView>
#include <QComboBox>
#include <QDir>
#include <QListWidgetItem>
#include <QProgressDialog>
#include <QSystemTrayIcon>

#include "headbutton.h"
#include "languagetablemodel.h"
#include "traywidget.h"
#include "initializer.h"

namespace Ui {
class GpMainWindow;
}

class Translator;
class DownloadWindow;

class GpMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit GpMainWindow(QWidget *parent = nullptr);

    ~GpMainWindow();

    bool initialize();

    int getFontSize() const;

    QString getText() const;


    Translator *getTranslator() const;

    QNetworkAccessManager *getManager() const;

    QString getCurrentSourceLang() const;

    QString getCurrentTargetLang() const;

    QString getCurrentSourceLang3() const;

    QString getCurrentTargetLang3() const;

protected:
    void closeEvent(QCloseEvent *event);

signals:
    void listOfLangsSet();

    void trayTitleBarEnableChecked(bool b);

    void transparentChecked(bool b);

    void ocrFailed();
public slots:
    void setFontSize(int size);

private slots:
    //get available language pairs
    void createListOfLangs(QNetworkReply *reply = nullptr);

    //update ComboBoxes when new source language, that is choosed
    void updateComboBox(QModelIndex);

    //update ComboBox with Tatget Languages when the new one is choosed
    void updateEndComboBox(QModelIndex);

    //Uncheck Other Source language buttons when the new one is checked
    void clearOtherSButtons();

    //Uncheck Other Target language buttons when the new one is checked
    void clearOtherEButtons();

    //update available Target languages
    void getResponseOfAvailLang(QNetworkReply *);

    //send translation request for each paragraph
    void createRequests(QString text = QString());

    //parse json response
    void getReplyFromAPY(QNetworkReply *);

    //add new langpairs to mru
    void saveMru();

    //NOT FOR LINUX
    void translateReceived(const QString &result);

    void on_boxInput_currentCharFormatChanged(const QTextCharFormat &);

    void dlAction_triggered();

    void fileTranslateAction_triggered();

    void ocrTranslateAction_triggered();

    void ocrReceived(QString text);

    void on_mru_itemClicked(QListWidgetItem *item);

    void on_swapBtn_clicked();

private:

    Ui::GpMainWindow *ui;
    QVector <HeadButton *> SourceLangBtns;
    QVector <HeadButton *> TargetLangBtns;
    QString currentSourceLang, currentTargetLang;
    QNetworkAccessManager *requestSender;
    HeadButton *currentSButton;
    QDialog *selectPathes;
    QProcess *apy;
    QDialog *fSizeBox;
    QTextDocument outputDoc;
    Translator *translator;

    TrayWidget *trayWidget;
    QSystemTrayIcon *trayIcon;
    QAction *showFullAction;
    QAction *exitAction;

    QAction *dlAction;
    QAction *documentTranslateAction;
    QAction *ocrTranslateAction;

    QThread thread;
    //not for Linux
    QDir *appdata;
    int lastBlockCount = 1;
    int currentFontSize;

    struct langpairUsed;
    //apt-get update completed
    bool checked = false;
    bool initRes = true;
    int serverStartedExitCode;

    const QString FONTSIZE = "interface/fontsize";
    const QString IDLANGTEXT = tr("Identify language...");
    const QString URL = "http://localhost:2737";
    const QString SERVERSTARTCMD = "/usr/share/apertium-gp/apertium-apy/apertium-apy/servlet.py "
                                   "/usr/share/apertium/modes";

    void loadConf();

    void setLangpair(QString source, QString target = QString());

    void setTrayWidgetPosition(TrayWidget::Position position);

    void setTrayWidgetEnabled(bool b);
};

