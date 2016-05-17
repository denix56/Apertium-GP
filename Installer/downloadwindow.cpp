#include "downloadwindow.h"
#include "ui_downloadwindow.h"
#include "initializer.h"
#include <KAuth>
#include <QNetworkReply>
#include <QMessageBox>
#include <QEventLoop>
#include <QDebug>
#include <QFile>
#include <QProcess>
#include <QNetworkRequest>
#include <QDir>
#include <QAbstractButton>

DownloadWindow::DownloadWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DownloadWindow)
{
    ui->setupUi(this);
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
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint | Qt::WindowTitleHint | Qt::CustomizeWindowHint);


    manager = new QNetworkAccessManager(this);
    getData();
}

DownloadWindow::~DownloadWindow()
{
    delete ui;
}

void DownloadWindow::getData()
{
    if (manager->networkAccessible() != QNetworkAccessManager::Accessible)
    {
        QMessageBox::critical(this, tr("Network Inaccessible"), tr("Can't check for updates as you appear to be offline."));
        return;
    }

    QProgressDialog wait(tr("Checking availiable packages to download..."),tr("Cancel"),0,100,this);
    wait.setModal(true);
    wait.show();


    //TODO: implement cancel option

#ifndef Q_OS_LINUX
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
    ui->view->sortByColumn(TYPE,Qt::AscendingOrder);
    wait.close();
    if (state == INSTALL)
    {
        QMessageBox box;
        box.warning(this,tr("Required Core tools are not installed"),tr("Please, install Required Core tools (they are highlighted red) to make translation work"));
    }
#else
    auto getCmd = new QProcess(this);
    //getCmd->setWorkingDirectory(DATALOCATION);
    getCmd->execute("apt-cache search apertium-all-dev");
    QStringList l = QString(getCmd->readAllStandardOutput()).split('\n');
    for(auto s:l)
        qDebug() << s;
    ;
    if (!l.contains("apertium-all-dev"))
    {
        QMessageBox box;
        if (box.warning(nullptr,tr("Required packages are not installed"),
                    tr("The program is missing some of packages. Press Ok to install them.")
                        ,QMessageBox::Cancel,QMessageBox::Ok)==QMessageBox::Cancel)
            return;
        //ApertiumHelper helper(this);
        //helper.initAction();

        KAuth::Action initAction("org.kde.auth.apertium-gp.init");
    }
#endif
}

void DownloadWindow::chooseAction(int row)
{
    switch (model->item(row)->state) {
    case INSTALL:
    case UPDATE:
        installpkg(row);
        break;
    case UNINSTALL:
        removepkg(row);
        break;
    default:
        break;
    }
}

void DownloadWindow::installpkg(int row)
{
#ifndef Q_OS_LINUX
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
    reply->deleteLater();
    #endif
}

void DownloadWindow::removepkg(int row)
{
    auto name = model->item(row)->name;
    QDir dir;
    if(name == "Required Core Tools")
    {
        QMessageBox box;
        if (box.warning(this,tr("Deleteing Core tools"),
                    tr("After deleteing this package the translation stops working. Are you sure?"),
                    QMessageBox::Ok, QMessageBox::Cancel)==QMessageBox::Cancel)
            return;
        dir.setPath(DATALOCATION+"/apertium-all-dev");
    }
    else
        dir.setPath(DATALOCATION+"/usr/share/apertium/"+name);

    if (Initializer::conf->contains("files/"+name) && dir.exists())
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
    {
        QMessageBox box;
        box.critical(this,tr("An error occurs while deleteing"),tr("Cannot locate this package"));
    }
}

void DownloadWindow::closeEvent(QCloseEvent *e)
{
    emit closed();
    QWidget::closeEvent(e);
}
