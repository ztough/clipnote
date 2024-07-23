#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QApplication>
#include <QMainWindow>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QIntValidator>
#include <QSettings>
#include <QNetworkInterface>
#include <QList>
#include <QFileDialog>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QKeySequenceEdit>
#include <QInputDialog>
#include <QProcess>
#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include "QHotkey/qhotkey.h"
#include <QUrl>
#include <QRegularExpression>
#include <QThread>
#include <QClipboard>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>
#include <qt_windows.h>
#include "videocapturethread.h"
#include "webvideocapturethread.h"
#include <opencv2/opencv.hpp>
#include <QDialogButtonBox>
#include <QStandardPaths>
#include <QUrlQuery>
#include <QEvent>
#include <QCloseEvent>
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE
extern QSettings settings;

extern QString httpGet(QString link);
extern QString getLink(QString id);
extern bool isVideoLink(QString link);
extern int operation;
extern void simulateCtrlV();
extern void simulateCtrl2();
extern double convertTimeToSeconds(QString *timeStr);
extern QNetworkAccessManager *networkManager;
extern bool flag;
extern QString addLink(QString url,QString time);
extern void action(int);
extern void updateUrl(QString old_string,QString new_string);
extern HWND PotPlayer();

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    QWebSocketServer *server;
    QList<QWebSocket*> clients;
    QList<QHotkey*> keyList;
    QSystemTrayIcon* trayIcon;
    QMenu* trayMenu;
    QAction* actionShow;
    QAction* actionExit;
    QEventLoop loop;
    QString tempTime;
    QTimer* timer = new QTimer(this);
    QString srt;
    QString video;
    QString srtList;
    QStringList markdown;
    QString directory;
    QString text;
    QStringList textList;

public:
    MainWindow(QWidget *parent = nullptr);
    void onNewConnection();
    void onTextMessageReceived(QString);
    void onClientDisconnected();
    void broadcastMessage(QString);
     int frameCounter = 0;
     QString prefix;
    void hideForm();
    void showForm();
    void updateCheck();
    bool clearKey(QKeySequenceEdit* key);
    ~MainWindow();
protected:
    bool nativeEvent(const QByteArray &eventType, void *message,long long *result)
    {
        Q_UNUSED(eventType);
        MSG *msg = static_cast<MSG *>(message);
        if (msg->message == WM_COPYDATA)
        {
            COPYDATASTRUCT *cds = reinterpret_cast<COPYDATASTRUCT *>(msg->lParam);
            if (cds->dwData == 0x6020)
            {
                const char *data = reinterpret_cast<const char *>(cds->lpData);
                int dataSize = cds->cbData;
                QString filename = QString::fromUtf8(data, dataSize);
                settings.setValue("url",filename.replace("\\","/"));
                *result = 1;
                return true;
            }
        }
        return QWidget::nativeEvent(eventType, message, result);
    }

    void closeEvent(QCloseEvent *event) override {
        event->ignore();
        this->hide();
    }
private slots:
    void on_pushButton_4_clicked();

    void updateFrame(const Mat& frame,int total,int current,int second);

    void finish();
    void appendText(QString text);
    void appendLink(QString url,QString time);



    void on_checkBox_5_stateChanged(int arg1);

    void on_checkBox_6_stateChanged(int arg1);



    void on_keySequenceEdit_1_editingFinished();

    void on_keySequenceEdit_2_editingFinished();


    void on_keySequenceEdit_6_editingFinished();



    void on_keySequenceEdit_9_editingFinished();

    void on_keySequenceEdit_10_editingFinished();

    void on_keySequenceEdit_11_editingFinished();

    void on_keySequenceEdit_12_editingFinished();

    void on_pushButton_11_clicked();






    void on_pushButton_12_clicked();



    void on_keySequenceEdit_13_editingFinished();

    void on_keySequenceEdit_14_editingFinished();

    void on_keySequenceEdit_15_editingFinished();

    void on_pushButton_13_clicked();







    void on_lineEdit_textChanged(const QString &arg1);



    void on_pushButton_16_clicked();

    void on_lineEdit_9_textChanged(const QString &arg1);



    void on_lineEdit_11_textChanged(const QString &arg1);







    void on_keySequenceEdit_16_editingFinished();






    void on_pushButton_6_clicked();




    void on_pushButton_8_clicked();





    void on_lineEdit_4_textChanged(const QString &arg1);


public:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
