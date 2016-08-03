#ifndef ApertiumGui_H
#define ApertiumGui_H
#include "headbutton.h"
#include "languagetablemodel.h"
#include "traywidget.h"
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
namespace Ui {

class DocTranslate;

class ApertiumGui;
}

class Translator;
class DownloadWindow;

class ApertiumGui : public QMainWindow
{
    Q_OBJECT

public:
    explicit ApertiumGui(QWidget *parent = 0);

    bool initialize();

    inline Translator *const getTranslator() const
    {
        return translator;
    }

    inline QNetworkAccessManager *const getManager() const
    {
        return requestSender;
    }

    inline QString getCurrentSourceLang() const
    {
        return currentSourceLang;
    }

    inline QString getCurrentTargetLang() const
    {
        return currentTargetLang;
    }

    int getFontSize() const;

    QString getText() const;
    void setTrayWidgetEnabled(bool b);
    ~ApertiumGui();

protected:
    void resizeEvent(QResizeEvent*);

    void closeEvent(QCloseEvent *event);

signals:
    void listOfLangsSet();

    void failedToStart();

public slots:
    void setFontSize(int size);

private slots:
    //update ComboBoxes when new source language, that is choosed
    void updateComboBox(QModelIndex);

    //update ComboBox with Tatget Languages when the new one is choosed
    void updateEndComboBox(QModelIndex);

    //Uncheck Other Source language buttons when the new one is checked
    void clearOtherSButtons();



    //get available language pairs
    //update available Target languages
    void getResponseOfAvailLang(QNetworkReply*);

    //parse json response
    void getReplyFromAPY(QNetworkReply*);

     //send translation request for each paragraph
    void createRequests(QString text = QString());

    void createListOfLangs(QNetworkReply *reply = nullptr);

    //Uncheck Other Target language buttons when the new one is checked
    void clearOtherEButtons();

    //box for changing font size
    //void fontSizeBox();

    //Not for Linux
    void saveMru();

    void translateReceived(const QString &result);

    void on_boxInput_currentCharFormatChanged(const QTextCharFormat &format);

    void dlAction_triggered();

    void on_mru_itemClicked(QListWidgetItem *item);

    void on_swapBtn_clicked();

    void on_docTranslateBtn_clicked();

private:

    Ui::ApertiumGui *ui;
    QVector <HeadButton *> SourceLangBtns;
    QVector <HeadButton *> TargetLangBtns;
    QString currentSourceLang, currentTargetLang;
    QNetworkAccessManager *requestSender;
    HeadButton *currentSButton;
    QDialog* selectPathes;
    QString serverPath;
    QString langPairsPath;
    QProcess *apy;
    QDialog *fSizeBox;
    QTextDocument outputDoc;
    Translator *translator;

    TrayWidget *trayWidget;
    QSystemTrayIcon *trayIcon;
    QAction *showFullAction;
    QAction * exitAction;

    QThread thread;
    //not for Linux
    QDir *appdata;
    int lastBlockCount = 1;
    int currentFontSize;

    struct langpairUsed;
    //apt-get update completed
    bool checked = false;
    bool initRes = true;
    const QString FONTSIZE = "interface/fontsize";
    const QString SERVERPATH = "path/serverPath";
    const QString LANGPATH = "path/langPath";
    const QString idLangText = tr("Identify language...");
    const QString url = "http://localhost:2737";

    void loadConf();
    void setLangpair(QString source, QString target = QString());
};

#endif // ApertiumGui_H
