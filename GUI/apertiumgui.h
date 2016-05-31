#ifndef ApertiumGui_H
#define ApertiumGui_H
#include "headbutton.h"
#include "languagetablemodel.h"
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
namespace Ui {
class ApertiumGui;
}

class Translator;
class DownloadWindow;

class ApertiumGui : public QMainWindow
{
    Q_OBJECT

friend class Translator;
friend class DownloadWindow;

public:
    explicit ApertiumGui(QWidget *parent = 0);
    bool initialize();
    ~ApertiumGui();

protected:
    void resizeEvent(QResizeEvent*);
signals:
    void listOfLangsSet();
    void failedToStart();
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
    void createRequests();

    void createListOfLangs(QNetworkReply *reply = nullptr);
    //Uncheck Other Target language buttons when the new one is checked
    void clearOtherEButtons();


    void fontSizeBox();
    void changeFontSize(int size);

    void fontSizeCancel();

    void on_mru_currentTextChanged(const QString &currentText);

    //Not for Linux
    void saveMru();
    void translateReceived(const QString &result);

    void on_boxInput_currentCharFormatChanged(const QTextCharFormat &format);
    void dlAction_triggered();


private:

    Ui::ApertiumGui *ui;
    QVector <HeadButton*> SourceLangBtns;
    QVector <HeadButton*> TargetLangBtns;
    QString currentSourceLang, currentTargetLang, currentSourceLang3, currentTargetLang3;
    QNetworkAccessManager* requestSender;
    QUrl url;
    HeadButton* currentSButton;
    QDialog* selectPathes;
    QString serverPath;
    QString langPairsPath;
    QProcess* apy;
    QDialog* fSizeBox;
    QTextDocument outputDoc;
    Translator *translator;
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

    void loadConf();

};

#endif // ApertiumGui_H
