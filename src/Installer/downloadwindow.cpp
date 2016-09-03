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

#include <QNetworkReply>
#include <QMessageBox>
#include <QEventLoop>
#include <QDebug>
#include <QFile>
#include <QProcess>
#include <QNetworkRequest>
#include <QDir>
#include <QAbstractButton>
#include <QPushButton>

#include "initializer.h"

#include "downloadwindow.h"
#include "ui_downloadwindow.h"

DownloadWindow::DownloadWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DownloadWindow)
{
    ui->setupUi(this);
    setModal(true);
    model = new DownloadModel(ui->view);
    delegate = new InstallerDelegate(ui->view);
    ui->view->setModel(model);
    ui->view->setItemDelegate(delegate);
    ui->view->verticalHeader()->sectionResizeMode(QHeaderView::ResizeToContents);
    ui->view->verticalHeader()->setDefaultSectionSize(37);
    ui->view->setMouseTracking(true);
    ui->view->setSortingEnabled(false);
    ui->view->viewport()->setAttribute(Qt::WA_Hover);
    connect(delegate,&InstallerDelegate::stateChanged,this,&DownloadWindow::chooseAction);
    setFixedSize(547, 582);
    setWindowFlags(Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint
                   | Qt::WindowTitleHint);
#ifdef Q_OS_LINUX
    auto applyButton = new QPushButton(tr("Apply"),this);
    connect(applyButton, &QPushButton::clicked,this, &DownloadWindow::applyChanges);
    ui->gridLayout_2->addWidget(applyButton,1,0,1,1,Qt::AlignRight);
    mngr = new ManagerHelper(this);
#else
    manager = new QNetworkAccessManager(this);
#endif
}

DownloadWindow::~DownloadWindow()
{
    delete ui;
}

bool DownloadWindow::getData(bool checked)
{
#ifndef Q_OS_LINUX
    if (manager->networkAccessible() != QNetworkAccessManager::Accessible)
    {
        QMessageBox::critical(this, tr("Network Inaccessible"),
                              tr("Can't check for updates as you appear to be offline."));
        return false;
    }
#else
    toInstall.clear();
    toUninstall.clear();
#endif
    model->reset();
    ui->view->setSortingEnabled(false);
    QProgressDialog wait(tr("Checking for availiable packages to download..."),tr("Cancel"),0,0);
    wait.setModal(true);
    wait.setMinimumDuration(0);
    wait.show();
    qApp->processEvents();
    //TODO: implement cancel option

#ifndef Q_OS_LINUX
    wait.setMaximum(100);
    actionCnt = 0;
#if defined(Q_OS_WIN)
#define OS_PATH "win32"
#elif defined(Q_OS_MAC)
#define OS_PATH "osx"
#else
#error "Not yet specialized for other OSs"
#endif
    //get headers of Core Tools
    auto reply = manager->head(
                QNetworkRequest(QUrl("http://apertium.projectjj.com/" OS_PATH "/nightly/apertium-all-dev.7z")));
    if (reply->error()!=QNetworkReply::NoError) {
        qDebug() << "An error occured: " << reply->errorString();
        return false;
    }
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    QString lm = reply->rawHeader("Last-Modified");
    QString cl = reply->rawHeader("Content-Length");
    auto state = INSTALL;
    if (QDir(DATALOCATION+"/apertium-all-dev").exists()) {
        state = UNINSTALL;
        if (Initializer::conf->value("files/apertium-all-dev").toString() != lm)
            state = UPDATE;
    }
    model->addItem(file("Required Core Tools",TOOLS, cl.toInt(),reply->request().url(),state,lm,true,0));
    wait.setValue(wait.maximum()/2);
    wait.setLabelText("Checking for new language pairs ...");

    //get information about langpairs
    reply = manager->get(QNetworkRequest(QUrl("http://apertium.projectjj.com/" OS_PATH "/nightly/data.php")));
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    connect(reply, &QNetworkReply::downloadProgress,[&wait](qint64 bytesReceived,qint64 bytesTotal)
    {wait.setValue(int(wait.maximum()/2*(1+1.*bytesReceived/bytesTotal)));});
    loop.exec();
    if (reply->error()!=QNetworkReply::NoError) {
        qDebug() << "An error occured: " << reply->errorString();
        return false;
    }
    QString body = reply->readAll();
    QRegularExpression rx("<tr><td>(apertium-\\w+-\\w+)</td><td>[^<]+</td><td>(\\d+)</td><td>([^<]+)</td>.*?</tr>");
    auto it = rx.globalMatch(body);
    while (it.hasNext()) {
        auto m = it.next();

        auto name = m.captured(1);
        auto state = INSTALL;
        if (QDir(DATALOCATION+"/usr/share/apertium/"+name).exists()) {
            state = UNINSTALL;
            if (Initializer::conf->value("files/"+name).toString() != m.captured(3))
                state = UPDATE;
        }
        model->addItem(file(name,LANGPAIRS, m.captured(2).toUInt(),
                            QUrl(QString("http://apertium.projectjj.com/" OS_PATH "/nightly/data.php?deb=")+name),
                            state, m.captured(3)));
    }
    ui->view->resizeColumnsToContents();
    ui->view->setSortingEnabled(true);
    ui->view->sortByColumn(TYPE,Qt::DescendingOrder);
    wait.close();
    if (state == INSTALL)
    {
        QMessageBox box;
        box.warning(this,tr("Required Core tools are not installed"),
                    tr("Please, install Required Core tools (they are highlighted red) to make translation work"));
    }
#else
    if(!checked){
        connect(&wait, &QProgressDialog::canceled, mngr, &ManagerHelper::canceled);
        mngr->update();
    }
    for(QString pair : mngr->getInfo(mngr->search("apertium")).split('\n')) {
        if (pair.isEmpty())
            continue;
        auto state = States::INSTALL;
        int i = pair.indexOf(' ');
        QString name = pair.left(i).remove("apertium-");
        int size = pair.mid(i+1).toInt();
        i = name.indexOf('-');
        QString lang1 = name.left(i);
        QString lang2 = name.mid(i+1);
        if (QDir("/usr/share/apertium/apertium-"+lang1+"-"+lang2).exists() ||
                QDir("/usr/share/apertium/apertium-"+lang2+"-"+lang1).exists())
            state = States::UNINSTALL;
        model->addItem(PkgInfo("apertium-" + name, Types::LANGPAIRS, size, QUrl(), state, ""));
    }
    auto state = States::INSTALL;
    if(QDir("/usr/share/apertium-apy").exists() || QDir("/usr/share/apertium-gp/apertium-apy").exists())
        state = States::UNINSTALL;
    model->addItem(PkgInfo("apertium-apy", Types::TOOLS, mngr->getInfo("apertium-apy").remove(QRegularExpression(".* ")).toInt(),
                        QUrl(), state, QString(), true));
    ui->view->resizeColumnsToContents();
    ui->view->setSortingEnabled(true);
    ui->view->sortByColumn(static_cast<int>(Columns::TYPE), Qt::DescendingOrder);
    wait.close();
#endif
    return true;
}

void DownloadWindow::chooseAction(int row)
{
    int pos;
    switch (model->item(row)->state) {
#ifndef Q_OS_LINUX
    case States::INSTALL:
    case States::UPDATE:
        installpkg(row);
        break;

    case States::UNINSTALL:
        removepkg(row);
        break;
#else
    case States::INSTALL:
        pos = toUninstall.indexOf(model->item(row)->name);
        if(pos!=-1)
            toUninstall.erase(toUninstall.begin()+pos);
        toInstall.push_back(model->item(row)->name);
        model->setData(model->index(row, static_cast<int>(Columns::STATE)), QVariant::fromValue(States::UNINSTALL));
        break;

    case States::UNINSTALL:
        pos = toInstall.indexOf(model->item(row)->name);
        if(pos!=-1)
            toInstall.erase(toInstall.begin()+pos);
        toUninstall.push_back(model->item(row)->name);
        model->setData(model->index(row, static_cast<int>(Columns::STATE)), QVariant::fromValue(States::INSTALL));
    default:
        break;
#endif
    }
}
#ifdef Q_OS_LINUX
void DownloadWindow::revert()
{
    int row;
    for (auto name : toInstall){
        row = model->find(name);
        model->setData(model->index(row,3), QVariant::fromValue(States::INSTALL));
    }
    toInstall.clear();
    for (auto name : toUninstall) {
        row = model->find(name);
        model->setData(model->index(row,3), QVariant::fromValue(States::UNINSTALL));
    }
    toUninstall.clear();
}


bool DownloadWindow::applyChanges()
{
    if(toInstall.isEmpty() && toUninstall.isEmpty()) {
        accept();
        return true;
    }
    if (model->countLangPairsInstalled() == 0)
    {
        QMessageBox box;
        box.critical(this, tr("Deleteing all lagpairs"),
                     tr("The program cannot work without any language pairs. "
                        "Please, leave at least one language pair"));
        revert();
        return false;
    }
    QProgressDialog dlg(this);
    dlg.setModal(true);
    dlg.setLabelText("Applying changes");
    dlg.setRange(0, 0);
    dlg.setCancelButton(nullptr);
    dlg.show();
    QStringList args;
    args << scriptPath;
    int pos = 0;
    if((pos = toUninstall.indexOf("apertium-apy"))!=-1){
        QMessageBox box;
        if(box.critical(this, tr("Uninstalling server."),
                        tr("You are going to remove Apertium-APY. "
                           "After that this program may stop working. Are you sure?"),
                        QMessageBox::Ok,QMessageBox::Abort)==QMessageBox::Ok) {
            toUninstall.erase(toUninstall.begin()+pos);
        }
        else{
            reject();
            return false;
        }
    }
    else
        if((pos = toInstall.indexOf("apertium-apy"))!=-1) {
            toInstall.erase(toInstall.begin()+pos);
        }
    int row;
    QStringList pkgsIn;

    if(!toInstall.isEmpty()) {
        for(auto name : toInstall) {
            qApp->processEvents();
            row = model->find(name);
            pkgsIn << model->item(row)->name;
            model->setData(model->index(row, static_cast<int>(Columns::STATE)),
                           QVariant::fromValue(States::UNINSTALL));
        }
    }

    QStringList pkgsRm;
    if(!toUninstall.isEmpty()) {
        for(auto name : toUninstall) {
            qApp->processEvents();
            row = model->find(name);
            pkgsRm << model->item(row)->name;
            model->setData(model->index(row, static_cast<int>(Columns::STATE)),
                           QVariant::fromValue(States::INSTALL));
        }
    }
    if(!toInstall.isEmpty() || !toUninstall.isEmpty() || pos != -1) {
        mngr->installRemove(pkgsIn, pkgsRm);
        toInstall.clear();
        toUninstall.clear();
    }
    dlg.close();
    accept();
    return true;

}
#else
void DownloadWindow::installpkg(int row)
{
    bool disabled = !ui->refreshButton->isEnabled();
    if (!disabled)
        ui->refreshButton->setEnabled(false);
    actionCnt++;
    auto name = model->item(row)->name;
    model->setData(model->index(row,STATE),DOWNLOADING);
    QNetworkRequest request;
    request.setUrl(QUrl(model->item(row)->link.toString()));
    auto reply = manager->get(request);
    reply->setReadBufferSize(model->item(row)->size*2);
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    auto sortConnection = connect(model, &DownloadModel::sorted, [&](){row = model->find(name);
        name = model->item(row)->name;});
    connect(reply, &QNetworkReply::downloadProgress, [&](qint64 bytesReceived,qint64)
    {model->setData(model->index(row,2),bytesReceived);});
    auto connection = connect(delegate, &InstallerDelegate::stateChanged,[&](int r)
    {if (r==row) reply->abort(); });
    loop.exec();
    disconnect(connection);
    if (reply->error()!=QNetworkReply::NoError)
    {
        model->setData(model->index(row,STATE),INSTALL);
        reply->deleteLater();
        return;
    }
    model->setData(model->index(row,STATE),UNPACKING);
    ui->view->update(model->index(row,SIZE));
    QString appdata_path = DATALOCATION;
    QDir().mkpath(appdata_path);
    QDir appdata(appdata_path);
    appdata.remove("data.tmp");
    appdata.remove("data.tar");
    appdata.remove("data.tar.xz");
    appdata.remove("debian-binary");
    appdata.remove("control.tar.gz");

    QFile file(appdata.filePath("data.tmp"));
    file.open(QIODevice::WriteOnly);
    file.write(reply->readAll());
    file.close();

    QDir exep(QCoreApplication::applicationDirPath());
    auto up = new QProcess(this);
    up->setWorkingDirectory(appdata_path);
#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
    up->start(exep.absoluteFilePath("7z"), QStringList() << "x" << "-y" << "data.tmp");
#else
    up->start("ar", QStringList() << "x" << "data.tmp");
#endif
    up->waitForStarted();
    up->waitForFinished();
    up->deleteLater();

    if (appdata.exists("data.tar")) {
        auto up = new QProcess(this);
        up->setWorkingDirectory(appdata_path);
        up->start(exep.absoluteFilePath("7z"), QStringList() << "x" << "-y" << "data.tar");
        up->waitForStarted();
        up->waitForFinished();
        up->deleteLater();
    }

    if (appdata.exists("data.tar.xz")) {
        auto up = new QProcess(this);
        up->setWorkingDirectory(appdata_path);
        up->start("tar", QStringList() << "-Jxf" << "data.tar.xz");
        up->waitForStarted();
        up->waitForFinished();
        up->deleteLater();
    }
    appdata.remove("data.tmp");
    appdata.remove("data.tar");
    appdata.remove("data.tar.xz");
    appdata.remove("debian-binary");
    appdata.remove("control.tar.gz");

    if (model->item(row)->link==QUrl("http://apertium.projectjj.com/"
                                     OS_PATH "/nightly/apertium-all-dev.7z"))
        Initializer::conf->setValue("files/apertium-all-dev", model->item(row)->lm);
    else
        Initializer::conf->setValue("files/"+name, model->item(row)->lm);
    ui->view->repaint();
    model->setData(model->index(row,STATE),UNINSTALL);
    ui->view->update(model->index(row,SIZE));
    actionCnt--;
    disconnect(sortConnection);
    if (!disabled)
        ui->refreshButton->setEnabled(true);
    reply->deleteLater();
}

void DownloadWindow::removepkg(int row)
{
    auto name = model->item(row)->name;
    QDir dir;
    if(name == "Required Core Tools")
    {
        QMessageBox box;
        if (box.warning(this,tr("Deleting Core tools"),
                        tr("After deleteing this package the translation stops working. Are you sure?"),
                        QMessageBox::Ok, QMessageBox::Cancel)==QMessageBox::Cancel)
            return;
        dir.setPath(DATALOCATION+"/apertium-all-dev");
    }
    else
        dir.setPath(DATALOCATION+"/usr/share/apertium/"+name);
    if (dir.exists())
        if (Initializer::conf->contains("files/"+name))
            if (dir.removeRecursively()) {
                auto pair = model->item(row)->name.mid
                        (model->item(row)->name.indexOf(QRegExp("(-[a-z]{2,3}){2}$"))).mid(1);
                dir.setPath(DATALOCATION+"/usr/share/apertium/modes");
                auto sourceLang = pair.left(pair.indexOf("-"));
                auto targetLang = pair.mid(pair.indexOf("-")+1);
                QRegExp expr(sourceLang+"|"+targetLang+"\\D*"+targetLang+"|"+sourceLang+"\\D*");
                for (QString filename : dir.entryList())
                    if (expr.indexIn(filename)!=-1)
                        dir.remove(filename);
                
                model->setData(model->index(row,3),INSTALL);
                Initializer::conf->remove("files/"+name);
            }
            else {
                QMessageBox box;
                box.critical(this, tr("An error occurs while deleteing"),
                             tr("Cannot delete this package"));
            }
        else
            if (name == "Required Core Tools" &&
                    Initializer::conf->contains("files/apertium-all-dev")) {
                dir.setPath(DATALOCATION+"/apertium-all-dev");
                dir.removeRecursively();
                model->setData(model->index(row,3),INSTALL);
                Initializer::conf->remove("files/apertium-all-dev");
            }
            else {
                QMessageBox box;
                box.critical(this,tr("An error occurs while deleteing"),
                             tr("Cannot locate this package"));
            }
}
#endif
void DownloadWindow::closeEvent(QCloseEvent *)
{
    accept();
}

void DownloadWindow::accept()
{
#ifdef Q_OS_LINUX
    if(!toInstall.isEmpty() || !toUninstall.isEmpty())
    {
        QMessageBox box;
        if(box.warning(this, tr("Unsaved changes"),
                       tr("You have unsaved changes. "
                          "To save them press Apply, to abort changes press Cancel"),
                       QMessageBox::Apply,QMessageBox::Cancel)==QMessageBox::Apply) {
            if(applyChanges()) {
                QDialog::accept();
                return;
            }
        }
        else
            revert();
    }
#endif
    QDialog::accept();

}

void DownloadWindow::on_refreshButton_clicked()
{
    getData();
}


