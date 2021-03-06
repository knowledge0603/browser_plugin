#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkReply>
#include <QPushButton>
#include <QLineEdit>
#include <QFileDialog>
#include <QTableWidget>
#include <QPrinter>
QT_BEGIN_NAMESPACE
class QWebEngineView;
class QLineEdit;
QT_END_NAMESPACE
namespace Ui {
class MainWindow;
}
class ctkPluginContext;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


protected:
    void resizeEvent(QResizeEvent*);

private:
    Ui::MainWindow *ui;
    QAction *action_newtab;
    QAction *action_open;
    QAction *action_bookmark;
    QAction *action_source;
    QAction *action_history;
    QAction *action_about;
    QAction *action_find;
    QAction *action_loadJS;
    QAction *action_devtool;
    QAction *action_print;
    QAction *action_lineEditURL_info;
    QAction *action_lineEditURL_bookmark;
    QAction *action_plugin;
    QString filename,js;
    QDialog *find_dialog;
    QLineEdit *lineEdit_find;
    QPushButton *pushButton_findlast, *pushButton_findnext, *pushButton_findclose;
    QStringList SL_bookmark_title, SL_bookmark_url, SL_history_time, SL_history_title, SL_history_url;
    void saveBookmarks();
    void fillBookmarkMenu();
    bool isBookbarked;
    struct History {
        QString time;
        QString title;
        QString url;
    };
    void appendHistory(QString stime, QString title, QString url);
    //QWebInspector* WI;
    QNetworkAccessManager *NAM;
    QTableWidget *tableSearch;
    void loadHistory();
    QDialog *dialogInfo;
    //------
    QWebEngineView *webview;
    QLineEdit *locationEdit;
    int progress;
    QString jQuery;
    QAction *rotateAction;
    void loadPlugin();
    ctkPluginContext *context;
    QList<QFileInfo> *fileInfo;
private slots:
    void gotoURL();
    void onLinkClicked(const QUrl&);
    void goBack();
    void goForward();
    void about();
    void loadStart();
    void loadFinish(bool);
    void openFile();
    void downloadRequest(const QNetworkRequest &request);
    void downloadFinish();
    void downloadProgress(qint64, qint64);
    void linkHover(const QString&, const QString&, const QString&);
    void newTab();
    void closeTab(int);
    void currentChange(int);
    void titleChange(QString);
    void iconChange();
    void switchTab();
    void fillURL();
    void closeCurrentTab();
    void stop();
    void viewSource();
    void loadProgress(int i);
    void history();
    void refresh();
    void loadBookmarks();
    void gotoBookmarkURL(bool);
    void find();
    void hidefind();
    void findlast();
    void findnext();
    void addBookmark();
    void loadJS();
    void zoomin();
    void zoomout();
    void zoom1();
    void inspector();
    void tabBarDoubleClick(int);
    void search(QString);
    void cancel();
    void cellClick(int,int);
    bool eventFilter(QObject *,QEvent *);
    void prevURL();
    void nextURL();
    void fullScreen();
    void showInfo();
    void print();
    void printDocument(QPrinter *printer);
    void closeInfo();
    //------
    void adjustLocation();
    void adjustTitle();
    void setProgress(int p);
    void finishLoading(bool);
    void rotateImages(bool invert);
    void changeLocation();
    void highlightAllLinks();
    void removeGifImages();
    void removeInlineFrames();
    void removeObjectElements();
    void removeEmbeddedElements();
    void openPluginTab();
public slots:
    void doubleClickedSlot(int row);
};

#endif // MAINWINDOW_H
