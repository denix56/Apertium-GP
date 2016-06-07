#include "downloadwindow.h"
#include "ui_downloadwindow.h"
#include "initializer.h"
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
    setFixedSize(size());
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint | Qt::WindowTitleHint);
#ifdef Q_OS_LINUX
    auto applyButton = new QPushButton(tr("Apply"),this);
    connect(applyButton, &QPushButton::clicked,this, &DownloadWindow::applyChanges);
    ui->gridLayout_2->addWidget(applyButton,1,0,1,1,Qt::AlignRight);
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
        QMessageBox::critical(this, tr("Network Inaccessible"), tr("Can't check for updates as you appear to be offline."));
        return false;
    }
#endif
    model->reset();
    toInstall.clear();
    toUninstall.clear();
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
        qDebug() << name;
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
    ui->view->sortByColumn(TYPE,Qt::AscendingOrder);
    wait.close();
    if (state == INSTALL)
    {
        QMessageBox box;
        box.warning(this,tr("Required Core tools are not installed"),tr("Please, install Required Core tools (they are highlighted red) to make translation work"));
    }
#else
    QProcess cmd(this);
    QDir path(DATALOCATION);
    if(!checked){
        QFile refreshScript(DATALOCATION+"/refresh.sh");
        refreshScript.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
        refreshScript.write("#!/bin/bash\n"
                            "apt-get update");
        refreshScript.close();
        cmd.start("pkexec", QStringList() << path.absoluteFilePath("refresh.sh"));
        cmd.waitForStarted();
        while(cmd.state()==QProcess::Running)
            qApp->processEvents();
        refreshScript.remove();
    }
    if(cmd.exitCode()) {
        wait.close();
        return false;
    }
    cmd.start("apt-cache search apertium");
    cmd.waitForStarted();
    while(cmd.state()==QProcess::Running)
        qApp->processEvents();
    if(cmd.exitCode())
        return false;
    auto output = cmd.readAllStandardOutput();
    QRegularExpression reg("apertium(-[a-z]{2,3}){2} ");
    auto it = reg.globalMatch(output);
    while(it.hasNext()) {
        qApp->processEvents();
        auto m = it.next();
        auto name = m.captured();
        name = name.left(name.length()-1);
        if(name=="apertium-all-dev")
            continue;
        cmd.start("apt-cache show "+ name);
        cmd.waitForStarted();
        while(cmd.state()==QProcess::Running)
            qApp->processEvents();
        if(cmd.exitCode())
            return false;
        QString outpk = cmd.readAllStandardOutput();
        QRegularExpression sizeReg("\nSize: [0-9]+");
         unsigned long long size = 0;
         auto jt = sizeReg.globalMatch(outpk);
         while(jt.hasNext())
         {
             qApp->processEvents();
             auto curSize = jt.next().captured();
             size = qMax(size, curSize.mid(curSize.lastIndexOf(' ')).toULongLong());
         }
         auto state = INSTALL;
         if (QFile("/usr/share/apertium/"+name).exists())
             state = UNINSTALL;
         model->addItem(file(name,LANGPAIRS, size, QUrl(),state, ""));
     }
    auto state = INSTALL;
    if(QFile(DATALOCATION +"/apertium-apy/apertium-apy/servlet.py").exists())
        state = UNINSTALL;
    model->addItem(file("Apertium-APY",TOOLS,2202010,QUrl("https://svn.code.sf.net/p/apertium/svn/trunk/apertium-tools/apertium-apy"),
                        state,"",true));
    ui->view->resizeColumnsToContents();
    ui->view->setSortingEnabled(true);
    ui->view->sortByColumn(TYPE,Qt::AscendingOrder);
    wait.close();
    #endif
    return true;
}

void DownloadWindow::chooseAction(int row)
{
    switch (model->item(row)->state) {
#ifndef Q_OS_LINUX
    case INSTALL:
    case UPDATE:
        installpkg(row);
        break;
    case UNINSTALL:
        removepkg(row);
        break;
#else
    case INSTALL:
        if(toUninstall.indexOf(model->item(row)->name)==-1)
            toInstall.push_back(model->item(row)->name);
        model->setData(model->index(row,STATE),UNINSTALL);
        break;
    case UNINSTALL:
        if(toInstall.indexOf(model->item(row)->name)==-1)
            toUninstall.push_back(model->item(row)->name);
        model->setData(model->index(row,STATE),INSTALL);
        break;
#endif
    }
}
void DownloadWindow::revert()
{
#ifdef Q_OS_LINUX
    int row;
    for (auto name : toInstall){
        row = model->find(name);
        model->setData(model->index(row,3),INSTALL);
    }
    toInstall.clear();
    for (auto name : toUninstall) {
        row = model->find(name);
        model->setData(model->index(row,3),UNINSTALL);
    }
    toUninstall.clear();
#endif
}

bool DownloadWindow::applyChanges()
{
#ifdef Q_OS_LINUX
    if(toInstall.isEmpty() && toUninstall.isEmpty()) {
        accept();
        return true;
    }
    if (model->countLangPairsInstalled() == 0)
    {
        QMessageBox box;
        box.critical(this, tr("Deleteing all lagpairs"),
                     tr("The program cannot work without any language pairs. Please, leave at least one language pair"));
        revert();
        reject();
        return false;
    }
    QProgressDialog dlg(this);
    dlg.setLabelText("Applying changes");
    dlg.setRange(0,0);
    dlg.setCancelButton(nullptr);
    dlg.show();
    QDir dir("/home/denys/.local/share");
    if(!QDir(DATALOCATION).exists())
        dir.mkdir("Apertium-GP");
    dir.setPath(DATALOCATION);
    QFile script(dir.absoluteFilePath("install-packages.sh"));
    script.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);

    //Generating bash script
    script.write("#!/bin/bash\n"
                 "#This script is generated automatically, do not modify it manually.\n"
                 "#install packages\n"
                 "#Argument -apy means that the Apertium-APY should be installed, -rapy means to remove Apertium-APY\n\n"
                 "DIR=\"$( cd \"$( dirname \"${BASH_SOURCE[0]}\" )\" && pwd )\"\n"
                 "cd \"$DIR\"\n"
                 "if [ \"$1\" = \"-apy\" ]; then\n"
                 "apt-get -y install build-essential python3-dev python3-pip zlib1g-dev subversion\n"
                 "pip3 install --upgrade tornado\n"
                 "svn co https://svn.code.sf.net/p/apertium/svn/trunk/apertium-tools/apertium-apy\n"
                 "elif [ \"$1\" = \"-rapy\" ]; then\n"
                 "rm -rf \"$DIR/apertium-apy\"\n"
                 "fi\n\n");
    QByteArray pcks;
    QStringList args;
    args << script.fileName();
    int pos;
    if((pos = toUninstall.indexOf("Apertium-APY"))!=-1){
        QMessageBox box;
        if(box.critical(this, tr("Uninstalling server."),
                     tr("You are going to remove Apertium-APY. "
                        "After that this program may stop working. Are you sure?"),
                     QMessageBox::Ok,QMessageBox::Abort)==QMessageBox::Ok) {
        toUninstall.erase(toUninstall.begin()+pos);
        args <<"-rapy";
        }
        else{
            script.close();
            reject();
            return false;
        }
    }
    else
    if((pos = toInstall.indexOf("Apertium-APY"))!=-1) {
        toInstall.erase(toInstall.begin()+pos);
        args <<"-apy";
    }

    if(!toInstall.isEmpty()) {
        script.write("apt-get -y install");
        for(auto name : toInstall) {
            qApp->processEvents();
            int row = model->find(name);
            pcks += " " + model->item(row)->name.toLatin1();

            model->setData(model->index(row,STATE),UNINSTALL);
        }
        script.write(pcks);
        script.write("\n");
        pcks.clear();
    }

    if(!toUninstall.isEmpty()) {
        script.write("apt-get -y remove");
        for(auto name : toUninstall) {
            qApp->processEvents();
            int row = model->find(name);
            pcks += " " + model->item(row)->name.toLatin1();
            model->setData(model->index(row,STATE),INSTALL);
        }
        script.write(pcks);
    }
    script.close();
    script.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
    if(!toInstall.isEmpty() || !toUninstall.isEmpty() || pos!=-1)
    {
        QProcess cmd;
        cmd.start("pkexec", args);
        qDebug() << cmd.arguments();
        cmd.waitForStarted();
        while(cmd.state()==QProcess::Running)
            qApp->processEvents();
        script.remove();
        toInstall.clear();
        toUninstall.clear();
    }
    dlg.close();
    accept();
    return true;
#endif
}

void DownloadWindow::installpkg(int row)
{
#ifndef Q_OS_LINUX
    actionCnt++;
    auto url = model->item(row)->link.toString();
    auto bsize = model->item(row)->size;
    auto lastState = model->item(row)->state;
    model->setData(model->index(row,STATE),DOWNLOADING);
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    auto reply = manager->get(request);
    reply->setReadBufferSize(bsize*2);
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    connect(reply, &QNetworkReply::downloadProgress, [&](qint64 bytesReceived,qint64)
    {model->setData(model->index(row,2),bytesReceived);});
    auto connection = connect(delegate, &InstallerDelegate::stateChanged,[&](int r)
    {if (r==row) reply->abort();});
    loop.exec();
    disconnect(connection);
    if (reply->error()!=QNetworkReply::NoError)
    {
        model->setData(model->index(row,STATE),lastState);
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

    if (model->item(row)->link==QUrl("http://apertium.projectjj.com/" OS_PATH "/nightly/apertium-all-dev.7z"))
        Initializer::conf->setValue("files/apertium-all-dev", model->item(row)->lm);
    else
        Initializer::conf->setValue("files/"+model->item(row)->name, model->item(row)->lm);
    ui->view->repaint();
    model->setData(model->index(row,STATE),UNINSTALL);
    ui->view->update(model->index(row,SIZE));
    actionCnt--;
    reply->deleteLater();
#endif
}

void DownloadWindow::removepkg(int row)
{
#ifndef Q_OS_LINUX
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
    qDebug() << Initializer::conf->contains("files/"+name);
    if (dir.exists())
    if (Initializer::conf->contains("files/"+name))
        if (dir.removeRecursively())
        {
            auto pair = model->item(row)->name.mid(model->item(row)->name.indexOf(QRegExp("(-[a-z]{2,3}){2}$"))).mid(1);
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
        else
        {
            QMessageBox box;
            box.critical(this, tr("An error occurs while deleteing"),tr("Cannot delete this package"));
        }
    else
        if (name == "Required Core Tools" &&  Initializer::conf->contains("files/apertium-all-dev")) {
            dir.setPath(DATALOCATION+"/apertium-all-dev");
            dir.removeRecursively();
            model->setData(model->index(row,3),INSTALL);
            Initializer::conf->remove("files/apertium-all-dev");
        }
        else
        {
            QMessageBox box;
            box.critical(this,tr("An error occurs while deleteing"),tr("Cannot locate this package"));
        }
#endif
}

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
        if(box.warning(this, tr("Unsaved changes"), tr("You have unsaved changes. To save them press Apply, to abort changes press Cancel"),
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
