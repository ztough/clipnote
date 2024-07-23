
#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QSharedMemory>
#include <QSettings>
#include <QHotkey>
#include <QUrl>
#include <QUrlQuery>
#include <QRegularExpression>
#include <QThread>
#include <QClipboard>
#include <QJsonObject>
#include <QJsonDocument>
#include <QtConcurrent>
#include <QUuid>
#include <QProcess>
#include <QSqlQuery>
#include <QSqlError>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEvent>
#include <QObject>
#include <qt_windows.h>
int operation=0;
HWND mainHwnd;
MainWindow *w;

QNetworkAccessManager *networkManager;


bool flag=false;

QString before;
QSettings settings("ClipNote","Settings");
QString dbstr=QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+ "/ClipNote/clipnote.db";
void action(int);

HWND PotPlayer();

QString httpGet(QString link);
void simulateCtrlV();
void simulateCtrlC();
void simulateCtrl2();
double convertTimeToSeconds(QString*);


void play();
void pause();


bool isVideoLink(QString link){
    if(!link.startsWith("http")){
        QFile file(link);

        if (file.exists()) {
            return true;
        } else {
            return false;
        }
    }else{
        return false;
        /*QUrl url(link);
        QNetworkRequest request(url);
        QString userAgent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/122.0.0.0 Safari/537.36 Edg/122.0.0.0";
        request.setRawHeader("User-Agent", userAgent.toUtf8());
        QNetworkReply *reply = networkManager->head(request);
        QEventLoop loop;
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();
        QString ct = reply->header(QNetworkRequest::ContentTypeHeader).toString();
        reply->deleteLater();
        if(ct.contains("video")||ct.contains("audio")){
            return true;
        }else{
            return false;
        }*/

    }

}

QString httpGet(QString link){
    QUrl url(link);
    QNetworkRequest request(url);
    QNetworkReply *reply = networkManager->get(request);
    QByteArray responseData;
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    if (reply->error() == QNetworkReply::NoError) {
        responseData = reply->readAll();
    }
    reply->deleteLater();
    return QString::fromUtf8(responseData);
}

QString addLink(QString url,QString time){
    if (QSqlDatabase::contains("qt_sql_default_connection")) {
        QSqlDatabase::removeDatabase("qt_sql_default_connection");
    }
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    QString dbstring=settings.value("database",dbstr).toString();
    db.setDatabaseName(dbstring);
    db.open();
    QSqlQuery query;
    query.prepare("INSERT INTO links (url,time) VALUES (:url, :time)");
    query.bindValue(":url", url);
    query.bindValue(":time", time);
    query.exec();
    QString id="cn://"+QString::number(query.lastInsertId().toInt());
    db.close();
    return id;
}
QString getLink(QString id){
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    QString dbstring=settings.value("database",dbstr).toString();
    db.setDatabaseName(dbstring);
    db.open();
    QSqlQuery query;
    query.prepare("SELECT * FROM links WHERE id=:id");
    query.bindValue(":id", id);
    query.exec();
    query.next();
    QString url = query.value(1).toString();
    QString time = query.value(2).toString();
    db.close();
    return QString("{\"url\":\"%1\",\"time\":\"%2\"}").arg(url).arg(time);
}
void updateUrl(QString old_string,QString new_string){
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    QString dbstring=settings.value("database",dbstr).toString();
    db.setDatabaseName(dbstring);
    db.open();
    QSqlQuery query;
    query.prepare("UPDATE links SET url = REPLACE(url, :old_string, :new_string) WHERE url LIKE :pattern");
    query.bindValue(":old_string", old_string);
    query.bindValue(":new_string", new_string);
    query.bindValue(":pattern", "%" + old_string + "%");
    query.exec();
    db.close();

}
std::string qstr2str(const QString qstr)
{
    QByteArray cdata = qstr.toLocal8Bit();
    return std::string(cdata);
}
void seek(int value){
    QJsonObject json;
    json["action"] = "pos";
    json["cnt"] =QString::number(value);
    QString jsonString = QJsonDocument(json).toJson(QJsonDocument::Compact);
    w->broadcastMessage(jsonString);
}

double convertTimeToSeconds(QString *timeStr) {
    QTime time = QTime::fromString(*timeStr, "hh:mm:ss.zzz");
    int hoursToSeconds = time.hour() * 3600;
    int minutesToSeconds = time.minute() * 60;
    int seconds = time.second();
    int milliseconds = time.msec();
    return hoursToSeconds + minutesToSeconds + seconds + milliseconds / 1000.0;
}
int main(int argc, char *argv[])
{

    QApplication a(argc, argv);

    networkManager=new QNetworkAccessManager;
    QSharedMemory shared("ClipNote");

    if(shared.attach())
    {
        QMessageBox::warning(nullptr,"提示",QObject::tr("请勿重复打开ClipNote，请查看系统托盘！"));
        return 0;
    }
    shared.create(1);

    QSettings reg("HKEY_CURRENT_USER\\Software\\Classes\\cn", QSettings::NativeFormat);
    reg.setValue("Default","URL:cn");
    reg.setValue("URL Protocol","");
    QString currentDir = QCoreApplication::applicationDirPath();
    QString filePath = QDir(currentDir).filePath("ClipNoteBridge.exe");

    reg.setValue("shell/open/command/Default",filePath.replace("/","\\")+" %1");



    w = new MainWindow();
    w->showNormal();
    w->activateWindow();
    if(settings.value("checkBox_5").toBool()){
        w->close();
    }
    for(int i=1;i<=16;i++){

        QHotkey *key=new QHotkey(QKeySequence(settings.value("keySequenceEdit_"+QString::number(i)).toString()), true,&a);
        QObject::connect(key, &QHotkey::activated,  [i]() {
            action(i);
        });
        w->keyList.append(key);
    }
#ifdef Q_OS_WIN
    mainHwnd= reinterpret_cast<HWND>(w->winId());
#endif

    return a.exec();
}


HWND PotPlayer(){
    HWND hwnd = FindWindow(L"PotPlayer64", NULL);
    if (hwnd == Q_NULLPTR) {
        hwnd=FindWindow(L"PotPlayer32", NULL);
    }
    return hwnd;
}


void releaseKey()
{

    INPUT releaseInputs[8] = {};
    int index = 0;


    releaseInputs[index].type = INPUT_KEYBOARD;
    releaseInputs[index].ki.wVk = VK_CONTROL;
    releaseInputs[index].ki.dwFlags = KEYEVENTF_KEYUP;
    index++;
    releaseInputs[index].type = INPUT_KEYBOARD;
    releaseInputs[index].ki.wVk = VK_LCONTROL;
    releaseInputs[index].ki.dwFlags = KEYEVENTF_KEYUP;
    index++;
    releaseInputs[index].type = INPUT_KEYBOARD;
    releaseInputs[index].ki.wVk = VK_RCONTROL;
    releaseInputs[index].ki.dwFlags = KEYEVENTF_KEYUP;
    index++;


    releaseInputs[index].type = INPUT_KEYBOARD;
    releaseInputs[index].ki.wVk = VK_SHIFT;
    releaseInputs[index].ki.dwFlags = KEYEVENTF_KEYUP;
    index++;
    releaseInputs[index].type = INPUT_KEYBOARD;
    releaseInputs[index].ki.wVk = VK_LSHIFT;
    releaseInputs[index].ki.dwFlags = KEYEVENTF_KEYUP;
    index++;
    releaseInputs[index].type = INPUT_KEYBOARD;
    releaseInputs[index].ki.wVk = VK_RSHIFT;
    releaseInputs[index].ki.dwFlags = KEYEVENTF_KEYUP;
    index++;

    releaseInputs[index].type = INPUT_KEYBOARD;
    releaseInputs[index].ki.wVk = VK_MENU;
    releaseInputs[index].ki.dwFlags = KEYEVENTF_KEYUP;
    index++;

    releaseInputs[index].type = INPUT_KEYBOARD;
    releaseInputs[index].ki.wVk = VK_LWIN;
    releaseInputs[index].ki.dwFlags = KEYEVENTF_KEYUP;
    index++;

    SendInput(index, releaseInputs, sizeof(INPUT));

}
void simulateCtrlV()
{
    releaseKey();
    INPUT inputs[4] = {};
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_CONTROL;
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = 'V';
    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = 'V';
    inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;
    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = VK_CONTROL;
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(4, inputs, sizeof(INPUT));
}
void simulateCtrlC()
{

    INPUT inputs[4] = {};
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_RCONTROL;
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = 'C';
    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = 'C';
    inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;
    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = VK_RCONTROL;
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(4, inputs, sizeof(INPUT));

}
void simulateCtrl2()
{

    INPUT inputs[4] = {};
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_RCONTROL;
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = '2';
    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk ='2';
    inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;
    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = VK_RCONTROL;
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(4, inputs, sizeof(INPUT));

}

void play(){
    SendMessage(PotPlayer(), 0x0400, 0x5007, 2);
    QJsonObject json;
    json["action"] = "play";
    QString jsonString = QJsonDocument(json).toJson(QJsonDocument::Compact);
    w->broadcastMessage(jsonString);
}
void pause(){
    SendMessage(PotPlayer(), 0x0400, 0x5007, 1);
    QJsonObject json;
    json["action"] = "pause";
    QString jsonString = QJsonDocument(json).toJson(QJsonDocument::Compact);
    w->broadcastMessage(jsonString);
}

void action(int index){
    if(index==1){

        if(PotPlayer()){
            int milliseconds =SendMessage(PotPlayer(), 0x0400, 0x5004, 1);
            QTime time(0, 0);
            time = time.addMSecs(milliseconds);
            QString formattedTime = time.toString("hh:mm:ss.zzz");
            LRESULT result = SendMessage(PotPlayer(), 0x0400,0x6020,reinterpret_cast<LPARAM>(mainHwnd));
            QString encodedUrl = settings.value("url").toString();
            QString id=addLink(encodedUrl,formattedTime);
            QString link = settings.value("link_style","[$time]($url)").toString()
                               .replace("$times",formattedTime)
                               .replace("$time",formattedTime.replace(QRegularExpression("(\\d\\d:\\d\\d:\\d\\d)\\.\\d\\d\\d"), "\\1"))
                               .replace("$url",id)
                               .replace("$name", QFileInfo(encodedUrl).baseName())
                               .replace("$filename", QFileInfo(encodedUrl).fileName())
                               .replace("$path", encodedUrl);
            QApplication::clipboard()->setText(link);
            simulateCtrlV();


        }else{
            operation=1;
            QJsonObject json;
            json["action"] = "pos";
            QString jsonString = QJsonDocument(json).toJson(QJsonDocument::Compact);
            w->broadcastMessage(jsonString);
            w->loop.exec();
        }

    }
    if(index==2){

        if(PotPlayer()){

            keybd_event(VK_CONTROL, 0, KEYEVENTF_EXTENDEDKEY | 0, 0);
            SendMessage(PotPlayer(), 0x0400, 0x5010, 0x43);
            keybd_event(VK_CONTROL, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
            QThread::msleep(1000);
            simulateCtrlV();

        }

        else{

            QJsonObject json;
            json["action"] = "pause";
            QString jsonString = QJsonDocument(json).toJson(QJsonDocument::Compact);

            operation=2;
            json["action"] = "snapshots";
            jsonString = QJsonDocument(json).toJson(QJsonDocument::Compact);
            w->broadcastMessage(jsonString);
            w->loop.exec();


        }

    }



    if(index==6){

        simulateCtrlC();

        QTimer::singleShot(100, [&]() {
            QString text = QApplication::clipboard()->text();
            QRegularExpression regex("cn://(\\d+)");
            QRegularExpressionMatch match = regex.match(text);
            if (match.hasMatch()) {

                QString data = match.captured(1);
                QString msg="{\"action\":\"cn\",\"url\":\"%1\",\"text\":true}";
                w->onTextMessageReceived(msg.arg(data));
            }
        });
    }



    if(index==9){
        if(PotPlayer()){
            int code = SendMessage(PotPlayer(), 0x0400, 0x5006, 0);
            if (code == 2) { SendMessage(PotPlayer(), 0x0400, 0x5007, 1); } else { SendMessage(PotPlayer(), 0x0400, 0x5007, 2); }
        }else{
            QJsonObject json;
            json["action"] = "playorpause";
            QString jsonString = QJsonDocument(json).toJson(QJsonDocument::Compact);
            w->broadcastMessage(jsonString);
        }
        return;
    }
    if(index==10){
        if(PotPlayer()){
            keybd_event(VK_CONTROL, 0,  KEYEVENTF_KEYUP, 0);
            SendMessage(PotPlayer(), 0x0400, 0x5010, 0x58);

        }else{
            QJsonObject json;
            json["action"] = "jiansu";
            QString jsonString = QJsonDocument(json).toJson(QJsonDocument::Compact);
            w->broadcastMessage(jsonString);
        }
        return;
    }

    if(index==11){

        if(PotPlayer()){
            keybd_event(VK_CONTROL, 0,  KEYEVENTF_KEYUP, 0);
            SendMessage(PotPlayer(), 0x0400, 0x5010, 0x43);


        }else{
            QJsonObject json;
            json["action"] = "jiasu";
            QString jsonString = QJsonDocument(json).toJson(QJsonDocument::Compact);
            w->broadcastMessage(jsonString);
        }
        return;
    }
    if(index==12){
        if(PotPlayer()){
            SendMessage(PotPlayer(), 0x0400, 0x5010, 0x5A);
        }else{
            QJsonObject json;
            json["action"] = "changsu";
            QString jsonString = QJsonDocument(json).toJson(QJsonDocument::Compact);
            w->broadcastMessage(jsonString);
        }
        return;
    }


    if(index==13){
        if(PotPlayer()){
            int ms = SendMessage(PotPlayer(), 0x0400, 0x5004, 1);
            SendMessage(PotPlayer(), 0x0400, 0x5005, ms-=5000);
            SendMessage(PotPlayer(), 0x0400, 0x5007, 2);
        }else{
            QJsonObject json;
            json["action"] = "kt";
            QString jsonString = QJsonDocument(json).toJson(QJsonDocument::Compact);
            w->broadcastMessage(jsonString);
        }
        return;
    }

    if(index==14){
        if(PotPlayer()){
            int ms = SendMessage(PotPlayer(), 0x0400, 0x5004, 1);
            SendMessage(PotPlayer(), 0x0400, 0x5005, ms+=5000);
            SendMessage(PotPlayer(), 0x0400, 0x5007, 2);
        }else{
            QJsonObject json;
            json["action"] = "kj";
            QString jsonString = QJsonDocument(json).toJson(QJsonDocument::Compact);
            w->broadcastMessage(jsonString);
        }
        return;
    }


    if(index==15){
        if(PotPlayer()){
            int vol = SendMessage(PotPlayer(), 0x0400, 0x5000, 1);
            SendMessage(PotPlayer(), 0x0400, 0x5001, vol-=10);
        }else{
            QJsonObject json;
            json["action"] = "yljian";
            QString jsonString = QJsonDocument(json).toJson(QJsonDocument::Compact);
            w->broadcastMessage(jsonString);
        }
        return;
    }

    if(index==16){

        if(PotPlayer()){

            int vol = SendMessage(PotPlayer(), 0x0400, 0x5000, 1);
            SendMessage(PotPlayer(), 0x0400, 0x5001, vol+=10);
        }else{
            QJsonObject json;
            json["action"] = "yljia";
            QString jsonString = QJsonDocument(json).toJson(QJsonDocument::Compact);
            w->broadcastMessage(jsonString);
        }
        return;
    }




}

