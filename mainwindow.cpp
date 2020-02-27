#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QMenu>

#include <QShortcut>
#include <QTextEdit>
#include <QNetworkProxy>
#include <QDesktopWidget>
#include <QTimer>
#include <QPrintPreviewDialog>
#include <QLabel>
#include <QtWidgets>
#include <QtWebEngineWidgets>

#include <ctkPluginFrameworkLauncher.h>
#include <ctkPluginFrameworkFactory.h>
#include <ctkPluginFramework.h>
#include <ctkPluginException.h>
#include <ctkPluginContext.h>

#include "pluginservice.h"
#include "eventlistener.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose,true);
    ui->lineEditURL->installEventFilter(this);
    ui->pushButtonStop->setVisible(false);
    setStyleSheet("QTabBar:tab { width:150px; text-align:left; }"
                  "QPushButton::menu-indicator { width:0px; }");
    ui->pushButtonBack->setIcon(style()->standardIcon(QStyle::SP_ArrowLeft));
    ui->pushButtonForward->setIcon(style()->standardIcon(QStyle::SP_ArrowRight));
    ui->pushButtonStop->setIcon(style()->standardIcon(QStyle::SP_DialogCloseButton));
    setWindowState(Qt::WindowMaximized);    
    connect(ui->pushButtonBack, SIGNAL(pressed()), this, SLOT(goBack()));
    connect(ui->pushButtonForward, SIGNAL(pressed()), this, SLOT(goForward()));
    connect(ui->pushButtonGoto, SIGNAL(pressed()), this, SLOT(gotoURL()));
    connect(ui->lineEditURL,SIGNAL(returnPressed()),this,SLOT(gotoURL()));
    connect(ui->lineEditURL,SIGNAL(textEdited(QString)),this,SLOT(search(QString)));
    connect(ui->pushButtonStop, SIGNAL(pressed()), this, SLOT(stop()));

    action_lineEditURL_info = new QAction(this);
    action_lineEditURL_info->setIcon(QIcon(":/info.svg"));
    connect(action_lineEditURL_info,SIGNAL(triggered(bool)),this,SLOT(showInfo()));
    ui->lineEditURL->addAction(action_lineEditURL_info,QLineEdit::LeadingPosition);

    action_lineEditURL_bookmark = new QAction(this);
    action_lineEditURL_bookmark->setIcon(QIcon(":/bookmark_off.png"));
    connect(action_lineEditURL_bookmark,SIGNAL(triggered(bool)),this,SLOT(addBookmark()));
    ui->lineEditURL->addAction(action_lineEditURL_bookmark,QLineEdit::TrailingPosition);

    QMenu *menu = new QMenu;
    action_newtab = new QAction("新标签页",menu);
    action_newtab->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_T));
    action_open = new QAction("打开本地网页",menu);
    action_open->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_O));    
    action_bookmark = new QAction("书签",menu);
    action_bookmark->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_B));
    action_find = new QAction("查找",menu);
    action_find->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F));
    action_source = new QAction("查看网页源码",menu);
    action_source->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_U));
    action_history = new QAction("历史记录",menu);
    action_history->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_H));
    action_devtool = new QAction("开发者工具",menu);
    action_devtool->setShortcut(QKeySequence(Qt::Key_F12));
    action_loadJS = new QAction("重载JS",menu);
    action_print = new QAction("打印",menu);
    action_print->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_P));
    action_about = new QAction("关于",menu);
    action_about->setShortcut(QKeySequence(Qt::Key_F1));
    action_plugin = new QAction("插件",menu);
    action_plugin->setShortcut(QKeySequence(Qt::CTRL+ Qt::Key_L));
    menu->addAction(action_newtab);
    menu->addAction(action_open);
    menu->addAction(action_bookmark);
    menu->addAction(action_find);
    menu->addAction(action_source);
    menu->addAction(action_history);
    menu->addAction(action_devtool);
    menu->addAction(action_loadJS);
    menu->addAction(action_print);
    menu->addAction(action_about);
    menu->addAction(action_plugin);
    ui->pushButtonMenu->setMenu(menu);
    connect(action_newtab,SIGNAL(triggered(bool)),this,SLOT(newTab()));
    connect(action_open,SIGNAL(triggered(bool)),this,SLOT(openFile()));    
    connect(action_find,SIGNAL(triggered(bool)),this,SLOT(find()));
    connect(action_source,SIGNAL(triggered(bool)),this,SLOT(viewSource()));
    connect(action_history,SIGNAL(triggered(bool)),this,SLOT(history()));
    connect(action_devtool,SIGNAL(triggered(bool)),this,SLOT(inspector()));
    connect(action_loadJS,SIGNAL(triggered(bool)),this,SLOT(loadJS()));
    connect(action_print,SIGNAL(triggered(bool)),this,SLOT(print()));
    connect(action_about,SIGNAL(triggered(bool)),this,SLOT(about()));
    connect(action_plugin,SIGNAL(triggered(bool)),this,SLOT(openPluginTab()));
    connect(new QShortcut(QKeySequence(Qt::Key_Up),this), SIGNAL(activated()),this, SLOT(prevURL()));
    connect(new QShortcut(QKeySequence(Qt::Key_Down),this), SIGNAL(activated()),this, SLOT(nextURL()));
    connect(new QShortcut(QKeySequence(Qt::Key_Return),this), SIGNAL(activated()),this, SLOT(gotoURL()));
    connect(new QShortcut(QKeySequence(Qt::Key_Enter),this), SIGNAL(activated()),this, SLOT(gotoURL()));
    connect(new QShortcut(QKeySequence(Qt::Key_Escape),this), SIGNAL(activated()),this, SLOT(cancel()));
    connect(new QShortcut(QKeySequence(Qt::Key_F5),this), SIGNAL(activated()),this, SLOT(refresh()));    
    connect(new QShortcut(QKeySequence(Qt::ALT + Qt::Key_Left),this), SIGNAL(activated()),this, SLOT(goBack()));
    connect(new QShortcut(QKeySequence(Qt::ALT + Qt::Key_Right),this), SIGNAL(activated()),this, SLOT(goForward()));
    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Tab),this), SIGNAL(activated()),this, SLOT(switchTab()));    
    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Return),this), SIGNAL(activated()),this, SLOT(fillURL()));
    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Enter),this), SIGNAL(activated()),this, SLOT(fillURL()));
    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_W),this), SIGNAL(activated()),this, SLOT(closeCurrentTab()));
    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_D),this), SIGNAL(activated()),this, SLOT(addBookmark()));
    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Plus),this), SIGNAL(activated()),this, SLOT(zoomin()));
    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Minus),this), SIGNAL(activated()),this, SLOT(zoomout()));
    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_0),this), SIGNAL(activated()),this, SLOT(zoom1()));
    connect(new QShortcut(QKeySequence(Qt::Key_F11),this), SIGNAL(activated()),this, SLOT(fullScreen()));
    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_L),this), SIGNAL(activated()),this, SLOT(openPluginTab()));
    connect(ui->tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(currentChange(int)));
    connect(ui->tabWidget,SIGNAL(tabBarDoubleClicked(int)),this,SLOT(tabBarDoubleClick(int)));
    newTab();
    ui->lineEditURL->setText("http://www.baidu.com/");

    find_dialog = new QDialog(this);
    find_dialog->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->setMargin(2);
    lineEdit_find = new QLineEdit;
    hbox->addWidget(lineEdit_find);
    pushButton_findlast = new QPushButton;
    pushButton_findlast->setIcon(style()->standardIcon(QStyle::SP_ArrowLeft));
    pushButton_findlast->setFlat(true);
    pushButton_findlast->setFocusPolicy(Qt::NoFocus);
    hbox->addWidget(pushButton_findlast);
    pushButton_findnext = new QPushButton;
    pushButton_findnext->setIcon(style()->standardIcon(QStyle::SP_ArrowRight));
    pushButton_findnext->setFlat(true);
    pushButton_findnext->setFocusPolicy(Qt::NoFocus);
    hbox->addWidget(pushButton_findnext);
    pushButton_findclose = new QPushButton;
    pushButton_findclose->setIcon(style()->standardIcon(QStyle::SP_DialogCloseButton));
    pushButton_findclose->setFlat(true);
    pushButton_findclose->setFocusPolicy(Qt::NoFocus);
    hbox->addWidget(pushButton_findclose);
    find_dialog->setLayout(hbox);
    find_dialog->resize(250,20);
    connect(lineEdit_find,SIGNAL(returnPressed()),this,SLOT(findnext()));
    connect(pushButton_findlast,SIGNAL(pressed()),this,SLOT(findlast()));
    connect(pushButton_findnext,SIGNAL(pressed()),this,SLOT(findnext()));
    connect(pushButton_findclose,SIGNAL(pressed()),this,SLOT(hidefind()));

    loadBookmarks();
    loadHistory();
    loadJS();

    QString FileNameHistory = QDir::currentPath() + "/history";
    QFile *file = new QFile(FileNameHistory);
    if(!QFileInfo(FileNameHistory).isFile()){
        file->open(QIODevice::WriteOnly);
        file->close();
    }

    tableSearch = new QTableWidget(this);
    tableSearch->setShowGrid(false);
    tableSearch->setColumnCount(1);
    tableSearch->setSelectionMode(QAbstractItemView::SingleSelection);
    tableSearch->horizontalHeader()->setVisible(false);
    tableSearch->verticalHeader()->setVisible(false);    
    tableSearch->setVisible(false);
    connect(tableSearch,SIGNAL(cellClicked(int,int)),this,SLOT(cellClick(int,int)));

    QStringList Largs=QApplication::arguments();
    qDebug() << Largs;
    if(Largs.length()>1){
        ui->lineEditURL->setText("file://"+Largs.at(1));
        gotoURL();
    }


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::newTab()
{
    webview = new QWebEngineView;
    webview->load(QUrl("https://www.baidu.com/"));

    webview->show();
    ui->tabWidget->addTab(webview,QIcon("icon.png"),"新标签页");
    ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);
}

void MainWindow::gotoURL()
{
    tableSearch->setVisible(false);
    QString surl = ui->lineEditURL->text();
    if(surl.indexOf("://")==-1){
        surl = "http://" + surl;
        ui->lineEditURL->setText(surl);
    }
    webview->load(QUrl(ui->lineEditURL->text()));
}

void MainWindow::stop()
{
    webview->stop();
}

void MainWindow::onLinkClicked(const QUrl &url)
{
    newTab();
    ui->lineEditURL->setText(url.toString());
    webview->load(url);
}

void MainWindow::goBack()
{
    webview->page()->triggerAction(QWebEnginePage::Back);
}

void MainWindow::goForward()
{
    webview->page()->triggerAction(QWebEnginePage::Forward);
}

void MainWindow::about()
{
    newTab();
    webview->load(QUrl("https://www.163.com/"));
}
void MainWindow::openPluginTab()
{
    loadPlugin();
}

void MainWindow::loadStart()
{
    ui->pushButtonGoto->setVisible(false);
    ui->pushButtonStop->setVisible(true);
    QString title = webview->title();
    QString surl  = webview->url().toString();
    for (int i=0; i<SL_bookmark_url.size(); i++) {
        if (SL_bookmark_url.at(i) == surl) {
            action_lineEditURL_bookmark->setIcon(QIcon(":/bookmark_on.png"));
            isBookbarked = true;
            break;
        } else {
            action_lineEditURL_bookmark->setIcon(QIcon(":/bookmark_off.png"));
            isBookbarked = false;
        }
    }    
}

void MainWindow::loadFinish(bool b)
{
    ui->pushButtonGoto->setVisible(true);
    ui->pushButtonStop->setVisible(false);
    if (b) {
        ui->progressBar->setValue(0);
        ui->lineEditURL->setText(webview->url().toString());
    } else {
        if (webview->url().toString().contains(".")) {
            if (!(webview->url().toString().startsWith("http://"))) {
                webview->load(QUrl( "http://" + webview->url().toString()));
            }
        } else {
            webview->load(QUrl( "http://www.baidu.com/s?wd=" + ui->lineEditURL->text()));
        }
    }    
}

void MainWindow::openFile()
{
    if (filename=="") {
        filename = QFileDialog::getOpenFileName(this, "打开网页", ".");
    } else {
        filename = QFileDialog::getOpenFileName(this, "打开网页", filename);
    }
    if (!filename.isEmpty()) {
        webview->load(QUrl::fromLocalFile(filename));
        ui->lineEditURL->setText("file://" + filename);
    }
}

void MainWindow::downloadRequest(const QNetworkRequest &request)
{
    QString defaultFileName = QFileInfo(request.url().toString()).fileName();
    QString fileName = QFileDialog::getSaveFileName(this, tr("保存文件"), defaultFileName);
    if (fileName.isEmpty()) return;
    QNetworkRequest newRequest = request;
    newRequest.setAttribute(QNetworkRequest::User, fileName);
    QNetworkAccessManager *networkManager = new QNetworkAccessManager();
    QNetworkReply *reply = networkManager->get(newRequest);
    connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(downloadProgress(qint64, qint64)));
    connect(reply, SIGNAL(finished()), this, SLOT(downloadFinish()));
}

void MainWindow::downloadFinish()
{
    QNetworkReply *reply = ((QNetworkReply*)sender());
    QNetworkRequest request = reply->request();
    QVariant v = request.attribute(QNetworkRequest::User);
    QString fileName = v.toString();
    QFile file(fileName);
    if (file.open(QFile::ReadWrite))
        file.write(reply->readAll());
}

QString sbytes(qint64 bytes){
    QString unit="B";
    double dbytes=bytes*1.0;
    if(bytes>999999999){
        dbytes/=(1024*1024*1024);
        unit="GB";
    }else{
        if(bytes>999999){
            dbytes/=(1024*1024);
            unit="MB";
        }else{
            if(bytes>999){
                dbytes/=1024;
                unit="KB";
            }
        }
    }
    return QString("%1%2").arg(QString::number(dbytes,'f',2)).arg(unit);
}

void MainWindow::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    statusBar()->showMessage(QString("%1/%2").arg(sbytes(bytesReceived)).arg(sbytes(bytesTotal)));
}

void MainWindow::titleChange(QString title)
{
    ui->tabWidget->setTabText(ui->tabWidget->currentIndex(), title);
    ui->tabWidget->setTabToolTip(ui->tabWidget->currentIndex(),title);
}

void MainWindow::linkHover(const QString &link, const QString &title, const QString &textContent)
{
    Q_UNUSED(title);
    Q_UNUSED(textContent);
    statusBar()->showMessage(link);
}

void MainWindow::closeTab(int i)
{
    ui->tabWidget->removeTab(i);
    if(ui->tabWidget->count()<1){
        newTab();
    }
}

void MainWindow::currentChange(int i)
{
    qDebug() << i;
    if(i!=-1)
        ui->lineEditURL->setText(webview->url().toString());
}

void MainWindow::iconChange()
{
    ui->tabWidget->setTabIcon(ui->tabWidget->currentIndex(),webview->icon());
}

void MainWindow::switchTab()
{
    int index = ui->tabWidget->currentIndex();
    if(index == ui->tabWidget->count()-1){
        ui->tabWidget->setCurrentIndex(0);
        return;
    }
    if(index < ui->tabWidget->count()-1){
        ui->tabWidget->setCurrentIndex(index+1);        
    }
}

void MainWindow::fillURL()
{
    ui->lineEditURL->setText("http://www."+ui->lineEditURL->text()+".com");
    gotoURL();
}

void MainWindow::closeCurrentTab()
{
    ui->tabWidget->removeTab(ui->tabWidget->currentIndex());
    if(ui->tabWidget->count()<1)newTab();
}

void MainWindow::viewSource()
{
    ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);
    ui->statusBar->showMessage("");
}

void MainWindow::loadProgress(int i)
{
    ui->progressBar->setValue(i);
}

void MainWindow::refresh()
{

}

void MainWindow::find()
{    
    find_dialog->move(x() + width() - find_dialog->width(), y()+60);
    find_dialog->show();
    find_dialog->raise();
}

void MainWindow::hidefind()
{
    find_dialog->hide();
}

void MainWindow::findlast()
{
}

void MainWindow::findnext()
{
}

void MainWindow::loadBookmarks()
{
    QString fileNameBookmark = QDir::currentPath() + "/bookmark";
    QFile *file = new QFile(fileNameBookmark);
    if (!QFileInfo(fileNameBookmark).isFile()) {
        file->open(QIODevice::WriteOnly);
        file->close();
    }
    if (file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream ts(file);
        QString s = ts.readAll();
        file->close();
        QStringList line = s.split("\n");
        for (int i=0; i<line.size(); i++) {
            if (line.at(i).contains("#")) {
                QStringList strlist=line.at(i).split("#");
                SL_bookmark_title.append(strlist.at(0));
                SL_bookmark_url.append(strlist.at(1));
            }
        }
    }
    fillBookmarkMenu();
}

void MainWindow::fillBookmarkMenu()
{
    QMenu *submenu = new QMenu;
    for (int i=0; i<SL_bookmark_title.size(); i++) {
        QAction *action_bookmark_title = new QAction(submenu);
        action_bookmark_title->setText(SL_bookmark_title.at(i));
        action_bookmark_title->setToolTip(SL_bookmark_url.at(i));
        submenu->addAction(action_bookmark_title);
        connect(action_bookmark_title,SIGNAL(triggered(bool)),this,SLOT(gotoBookmarkURL(bool)));
    }
    action_bookmark->setMenu(submenu);
}

void MainWindow::addBookmark()
{
    QString title = webview->title();
    QString surl =webview->url().toString();
    if (title!="" && surl!="") {
        if (isBookbarked) {
            action_lineEditURL_bookmark->setIcon(QIcon(":/bookmark_off.png"));
            SL_bookmark_title.removeOne(title);
            SL_bookmark_url.removeOne(surl);
        } else {
            action_lineEditURL_bookmark->setIcon(QIcon(":/bookmark_on.png"));
            SL_bookmark_title.append(title);
            SL_bookmark_url.append(surl);
        }
        isBookbarked = !isBookbarked;
        fillBookmarkMenu();
        saveBookmarks();
    }
}

void MainWindow::saveBookmarks()
{
    QString fileNameBookmark = QDir::currentPath() + "/bookmark";
    QFile file(fileNameBookmark);
    if (file.open(QFile::WriteOnly)) {
        QTextStream ts(&file);
        QString s;
        for(int i=0; i<SL_bookmark_title.size(); i++) {
            s = s+ SL_bookmark_title.at(i) + "#" + SL_bookmark_url.at(i);
            if (i < SL_bookmark_title.size()-1)
                s = s + "\n";
        }
        ts << s;
        file.close();
    }
}

void MainWindow::gotoBookmarkURL(bool)
{
    QAction *action = qobject_cast<QAction*>(sender()); // 获取发出信号的对象
    qDebug() << action->toolTip();
    ui->lineEditURL->setText(action->toolTip());
}

void MainWindow::loadHistory()
{
    SL_history_time.clear();
    SL_history_title.clear();
    SL_history_url.clear();
    QString fileNameHistory = QDir::currentPath() + "/history";
    QFile *file = new QFile(fileNameHistory);
    if (file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream ts(file);
        QString s = ts.readAll();
        file->close();
        QStringList line = s.split("\n");
        for(int i=0; i<line.size(); i++){
            if (line.at(i).contains("#")) {
                QStringList strlist = line.at(i).split("#");
                SL_history_time.append(strlist.at(0));
                SL_history_title.append(strlist.at(1));
                SL_history_url.append(strlist.at(2));
            }
        }
    }
}

void MainWindow::history()
{
    loadHistory();
    QString s = "<html><head><title>历史记录</title><style>a{text-decoration:none;color:black;} table{margin:10 auto;} td{padding:5px;white-space:nowrap;overflow:hidden;text-overflow:ellipsis;} td:first-child{color:gray;} td:nth-child(2){max-width:500px;} td:nth-child(3){color:gray;max-width:500px;}</style></head><body><table>";
    for (int i=SL_history_time.size()-1; i>=0; i--) {
        s += "<tr><td>" + SL_history_time.at(i) + "</td><td><a href=" + SL_history_url.at(i) + ">" + SL_history_title.at(i) + "</td><td>" + SL_history_url.at(i) + "</td></tr>";
    }
    s += "</table></body></html>";
    newTab();
    QUrl url("HTYBrowser://history");
    ui->tabWidget->setTabIcon(ui->tabWidget->currentIndex(),QIcon(":/history.ico"));
}

void MainWindow::appendHistory(QString stime, QString title, QString surl)
{
    QString fileNameHistory = QDir::currentPath() + "/history";
    QFile file(fileNameHistory);
    if (file.open(QFile::WriteOnly | QIODevice::Append)) {
        QTextStream ts(&file);
        QString s = stime + "#" + title + "#" + surl + "\n";
        ts << s;
        file.close();
    }
}

void MainWindow::loadJS()
{
    QString fileNameJS = QDir::currentPath() + "/js.js";
    QFile *file = new QFile(fileNameJS);
    if (!QFileInfo(fileNameJS).isFile()) {
        file->open(QIODevice::WriteOnly);
        file->close();
    } else {
        if (file->open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream ts(file);
            js = ts.readAll();
            file->close();
        }
    }
}

void MainWindow::zoomin()
{
}

void MainWindow::zoomout()
{
}

void MainWindow::zoom1()
{
}

void MainWindow::inspector()
{
}

void MainWindow::tabBarDoubleClick(int index)
{
    qDebug() << "tab" << index;
    if (index == -1) {
        newTab();
    }
}

void MainWindow::search(QString key)
{
    if (key != "") {
        tableSearch->setRowCount(0);
        tableSearch->setVisible(true);        
        QStringList SLURL;
        SLURL.append(SL_history_url);
        SLURL.append(SL_bookmark_url);        
        QStringList result = SLURL.filter(key);
        result.removeDuplicates();
        if (result.size() == 0) {
            tableSearch->setVisible(false);
        } else {
            for(int i=0; i<result.size(); i++){
                tableSearch->insertRow(i);
                tableSearch->setItem(i,0,new QTableWidgetItem(result.at(i)));
            }
        }
    }
}

void MainWindow::cancel()
{   
    tableSearch->hide();
    stop();
}

void MainWindow::cellClick(int r,int c)
{
    ui->lineEditURL->setText(tableSearch->item(r,c)->text());
    gotoURL();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    tableSearch->resize(ui->lineEditURL->width(),220);
    tableSearch->setColumnWidth(0,ui->lineEditURL->width());
    tableSearch->move(ui->lineEditURL->x(), ui->lineEditURL->y() + ui->lineEditURL->height());
    if (find_dialog->isVisible()) {
        find_dialog->move(x() + width() - find_dialog->width(), y()+60);
    }
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->lineEditURL) {
        if (event->type() == QEvent::FocusIn) {
            QTimer::singleShot(0,ui->lineEditURL,SLOT(selectAll()));
        }
    }
    return QWidget::eventFilter(watched,event);
}

void MainWindow::prevURL()
{
    if (tableSearch->isVisible() && tableSearch->currentRow()>0) {
        tableSearch->setCurrentCell(tableSearch->currentRow()-1,0);
        ui->lineEditURL->setText(tableSearch->item(tableSearch->currentRow(),0)->text());
    }
}

void MainWindow::nextURL()
{
    if (tableSearch->isVisible() && tableSearch->currentRow()<tableSearch->rowCount()-1) {
        tableSearch->setCurrentCell(tableSearch->currentRow()+1,0);
        ui->lineEditURL->setText(tableSearch->item(tableSearch->currentRow(),0)->text());
    }
}

void MainWindow::fullScreen()
{
    if (isFullScreen()) {
        showMaximized();
        ui->statusBar->show();
        ui->navbar->show();
        ui->tabWidget->tabBar()->show();
        ui->progressBar->show();
    } else {
        showFullScreen();
        ui->statusBar->hide();
        ui->navbar->hide();
        ui->tabWidget->tabBar()->hide();
        ui->progressBar->hide();
        find_dialog->hide();
    }
}

void MainWindow::showInfo()
{
    dialogInfo = new QDialog;
    dialogInfo->setWindowFlags(Qt::FramelessWindowHint);
    dialogInfo->setAttribute(Qt::WA_QuitOnClose,false);
    dialogInfo->move(ui->lineEditURL->x(),ui->lineEditURL->y()+60);
    QVBoxLayout *vbox = new QVBoxLayout;
    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addStretch();
    QPushButton *pushButton_closeInfo = new QPushButton;
    pushButton_closeInfo->setFixedSize(24,24);
    pushButton_closeInfo->setIcon(style()->standardIcon(QStyle::SP_DialogCloseButton));
    pushButton_closeInfo->setFlat(true);
    pushButton_closeInfo->setFocusPolicy(Qt::NoFocus);
    connect(pushButton_closeInfo,SIGNAL(released()),this,SLOT(closeInfo()));
    hbox->addWidget(pushButton_closeInfo);
    vbox->addLayout(hbox);
    hbox = new QHBoxLayout;
    QLabel *label = new QLabel;
    label->setText("连接是安全的");
    label->setStyleSheet("color:green;");
    hbox->addWidget(label);
    vbox->addWidget(label);
    dialogInfo->setLayout(vbox);
    dialogInfo->show();
}

void MainWindow::closeInfo()
{
    dialogInfo->close();
}

void MainWindow::print()
{
    QPrinter printer(QPrinter::HighResolution);
    QPrintPreviewDialog preview(&printer, this);
    connect(&preview, SIGNAL(paintRequested(QPrinter*)), this, SLOT(printDocument(QPrinter*)));
    preview.exec();
}

void MainWindow::printDocument(QPrinter *printer)
{
}
void MainWindow::adjustLocation()
{
    locationEdit->setText(webview->url().toString());
}
void MainWindow::adjustTitle()
{
    if (progress <= 0 || progress >= 100)
        setWindowTitle(webview->title());
    else
        setWindowTitle(QStringLiteral("%1 (%2%)").arg(webview->title()).arg(progress));
}
void MainWindow::setProgress(int p)
{
    progress = p;
    adjustTitle();
}
void MainWindow::finishLoading(bool)
{
    progress = 100;
    adjustTitle();
    webview->page()->runJavaScript(jQuery);

    rotateImages(rotateAction->isChecked());
}
void MainWindow::rotateImages(bool invert)
{
    QString code;

    if (invert)
        code = QStringLiteral("qt.jQuery('img').each( function () { qt.jQuery(this).css('transition', 'transform 2s'); qt.jQuery(this).css('transform', 'rotate(180deg)') } )");
    else
        code = QStringLiteral("qt.jQuery('img').each( function () { qt.jQuery(this).css('transition', 'transform 2s'); qt.jQuery(this).css('transform', 'rotate(0deg)') } )");
    webview->page()->runJavaScript(code);
}

void MainWindow::changeLocation()
{
    QUrl url = QUrl::fromUserInput(locationEdit->text());
    webview->load(url);
    webview->setFocus();
}

void MainWindow::highlightAllLinks()
{
    QString code = QStringLiteral("qt.jQuery('a').each( function () { qt.jQuery(this).css('background-color', 'yellow') } )");
    webview->page()->runJavaScript(code);
}

void MainWindow::removeGifImages()
{
    QString code = QStringLiteral("qt.jQuery('[src*=gif]').remove()");
    webview->page()->runJavaScript(code);
}

void MainWindow::removeInlineFrames()
{
    QString code = QStringLiteral("qt.jQuery('iframe').remove()");
    webview->page()->runJavaScript(code);
}

void MainWindow::removeObjectElements()
{
    QString code = QStringLiteral("qt.jQuery('object').remove()");
    webview->page()->runJavaScript(code);
}

void MainWindow::removeEmbeddedElements()
{
    QString code = QStringLiteral("qt.jQuery('embed').remove()");
    webview->page()->runJavaScript(code);
}
void MainWindow::loadPlugin()
{
    // get plugin location path
    //QString path = QApplication::applicationDirPath() + "/plugins";
    QString path ="/home/grj/Desktop/Qt_HTYBrowser-master/plugins";
    qDebug() << "--------------get plugin location path:"<<path;

    QDir *dir=new QDir(path);
    QStringList filter;
    filter<< "*.dll" << "*.so";
    fileInfo=new QList<QFileInfo>(dir->entryInfoList(filter));

    QTableWidget *tableWidget = new QTableWidget;
    int count=fileInfo->count();
    tableWidget->setRowCount(count);
    tableWidget->setColumnCount(1);
    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    for(int k=0;k<fileInfo->count();k++) {
        QString strPlugin = fileInfo->at(k).absoluteFilePath();
        qDebug() << "--------------strPlugin:"<<strPlugin;
        tableWidget->setItem(k, 0, new QTableWidgetItem(QIcon(":/icon.png"), strPlugin));
    }

    connect(tableWidget,SIGNAL(cellDoubleClicked(int,int)),this,SLOT(doubleClickedSlot(int)));
    ui->tabWidget->addTab(tableWidget,QIcon(":/bookmark_on.png"),"plugin标签");
    ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);
}
void MainWindow::doubleClickedSlot(int row)
{
    QString path ="/home/grj/Desktop/Qt_HTYBrowser-master/plugins";
    ctkPluginFrameworkLauncher::addSearchPath(path);
    //set and start CTK Plugin System
    ctkPluginFrameworkLauncher::start("org.commontk.eventadmin");
    //get plugin context
    context = ctkPluginFrameworkLauncher::getPluginContext();
    // event listener
    EventListener listener;
    context->connectFrameworkListener(&listener, SLOT(onFrameworkEvent(ctkPluginFrameworkEvent)));
    context->connectPluginListener(&listener, SLOT(onPluginEvent(ctkPluginEvent)));
    context->connectServiceListener(&listener, "onServiceEvent"); //, filter);
    try {
        QSharedPointer<ctkPlugin> plugin = context->installPlugin(QUrl::fromLocalFile(fileInfo->at(row).absoluteFilePath()));
        // 获取清单头和值
        QHash<QString, QString> headers = plugin->getHeaders();
        qDebug() << "Headers:" << headers << "\r\n";
        // 获取符号名
        QString symb = plugin->getSymbolicName();
        qDebug() << "Symbolic Name:" << symb << "\r\n";
        plugin->start(ctkPlugin::START_TRANSIENT);
        qDebug() << "Plugin start ...";
        ctkServiceReference reference = context->getServiceReference<PluginService>();
        if (reference) {
            PluginService* service = qobject_cast<PluginService *>(context->getService(reference));
            if(service != Q_NULLPTR) {
                service->doSomeThing();
                QWidget* w = service->getWidget();
                ui->tabWidget->addTab(w,QIcon(":/bookmark_on.png"),"plugin");
                ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);
            }
        }
    } catch(const ctkPluginException &e) {
        qDebug() << "Failed to install plugin" << e.what();
        return;
    }
}
