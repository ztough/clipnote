#include "mainwindow.h"
#include "./ui_mainwindow.h"



bool MainWindow::clearKey(QKeySequenceEdit *key)
{
    if(key->keySequence().toString().contains("F")&&key->keySequence().toString().remove("F").toInt()>12){
        QKeySequence k;
        key->setKeySequence(k);
        return false;
    }
    return true;
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    ui->setupUi(this);

    setWindowIcon(QIcon(":/img/logo.png"));
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    setMaximumSize(size().width(),size().height());
    setMinimumSize(size().width(),size().height());
    server = new QWebSocketServer("ClipNote", QWebSocketServer::NonSecureMode, this);
    if (server->listen(QHostAddress::Any, 5488)) {
        connect(server, &QWebSocketServer::newConnection, this, &MainWindow::onNewConnection);
    }

    trayIcon = new QSystemTrayIcon(QIcon(":/img/logo.png"), this);
    trayMenu = new QMenu();
    actionShow = new QAction("显示", this);

    actionExit = new QAction("退出", this);
    trayMenu->addAction(actionShow);
    trayMenu->addSeparator();

    trayMenu->addAction(actionExit);
    trayMenu->addSeparator();


    trayIcon->setContextMenu(trayMenu);
    connect(actionShow, &QAction::triggered, [&]() {
        showNormal();
        activateWindow();
    });

    connect(actionExit, &QAction::triggered, [&]() {
        settings.remove("url");
        close();

        QCoreApplication::exit();
    });
    connect(trayIcon, &QSystemTrayIcon::activated, [&](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::DoubleClick) {
            showNormal();
            activateWindow();
        }
    });

    trayIcon->show();

    ui->tabWidget->setCurrentIndex(0);

    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString destinationPath = appDataPath + "/clipnote.db";
    QString proData=QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).replace("ClipNote","ClipNotePro")+ "/clipnote.db";

    if(!QDir(appDataPath).exists()){
        QDir().mkpath(appDataPath);
    }

    QString resourcePath = ":/db/clipnote.db";
    if(QFile::exists(proData)){
        resourcePath=proData;
    }
    if (!QFile::exists(destinationPath)) {
        if (QFile::copy(resourcePath, destinationPath)) {
            QFile::setPermissions(destinationPath, QFileDevice::ReadOwner | QFileDevice::WriteOwner);
        }
    }

    QList<QCheckBox*> checkboxes = ui->groupBox->findChildren<QCheckBox*>();
    foreach (QCheckBox *checkbox, checkboxes) {
        checkbox->setChecked(settings.value(checkbox->objectName()).toBool());
    }
    QList<QKeySequenceEdit*> keySequenceEdits = ui->groupBox_2->findChildren<QKeySequenceEdit*>();
    foreach (QKeySequenceEdit *keySequenceEdit, keySequenceEdits) {
        keySequenceEdit->setKeySequence(settings.value(keySequenceEdit->objectName()).toString());
    }
    ui->lineEdit_9->setText(settings.value("potplayer").toString());
    if( ui->lineEdit_9->text().isEmpty()){
        QSettings pp("HKEY_CLASSES_ROOT\\potplayer\\shell\\open\\command", QSettings::NativeFormat);
        QString defaultValue = pp.value(".").toString().replace(" \"%1\"","");
        ui->lineEdit_9->setText(defaultValue);
        settings.setValue("potplayer",defaultValue);
    }
    ui->lineEdit_11->setText(settings.value("database",destinationPath).toString());
    if( ui->lineEdit_11->text().isEmpty()){

        settings.setValue("database",destinationPath);
    }

    ui->lineEdit_4->setText(settings.value("interval",10).toString());


    ui->lineEdit->setText(settings.value("link_style","[$time]($url)").toString());



    if(QLineEdit *lineEdit = ui->keySequenceEdit_1->findChild<QLineEdit*>("qt_keysequenceedit_lineedit")){
        lineEdit->setPlaceholderText("跳转链接");
    }
    if(QLineEdit *lineEdit = ui->keySequenceEdit_2->findChild<QLineEdit*>("qt_keysequenceedit_lineedit")){
        lineEdit->setPlaceholderText("视频截图");
    }

    if(QLineEdit *lineEdit = ui->keySequenceEdit_6->findChild<QLineEdit*>("qt_keysequenceedit_lineedit")){
        lineEdit->setPlaceholderText("文本跳转");
    }

    if(QLineEdit *lineEdit = ui->keySequenceEdit_9->findChild<QLineEdit*>("qt_keysequenceedit_lineedit")){
        lineEdit->setPlaceholderText("播放暂停");
    }
    if(QLineEdit *lineEdit = ui->keySequenceEdit_10->findChild<QLineEdit*>("qt_keysequenceedit_lineedit")){
        lineEdit->setPlaceholderText("减速");
    }
    if(QLineEdit *lineEdit = ui->keySequenceEdit_11->findChild<QLineEdit*>("qt_keysequenceedit_lineedit")){
        lineEdit->setPlaceholderText("加速");
    }
    if(QLineEdit *lineEdit = ui->keySequenceEdit_12->findChild<QLineEdit*>("qt_keysequenceedit_lineedit")){
        lineEdit->setPlaceholderText("常速");
    }
    if(QLineEdit *lineEdit = ui->keySequenceEdit_13->findChild<QLineEdit*>("qt_keysequenceedit_lineedit")){
        lineEdit->setPlaceholderText("快退");
    }
    if(QLineEdit *lineEdit = ui->keySequenceEdit_14->findChild<QLineEdit*>("qt_keysequenceedit_lineedit")){
        lineEdit->setPlaceholderText("快进");
    }
    if(QLineEdit *lineEdit = ui->keySequenceEdit_15->findChild<QLineEdit*>("qt_keysequenceedit_lineedit")){
        lineEdit->setPlaceholderText("音量-");
    }
    if(QLineEdit *lineEdit = ui->keySequenceEdit_16->findChild<QLineEdit*>("qt_keysequenceedit_lineedit")){
        lineEdit->setPlaceholderText("音量+");
    }

    QIntValidator* intValidator = new QIntValidator(this);
    ui->lineEdit_4->setValidator(intValidator);
}


void MainWindow::onNewConnection(){
    QWebSocket *client = server->nextPendingConnection();
    connect(client, &QWebSocket::textMessageReceived, this, &MainWindow::onTextMessageReceived);
    connect(client, &QWebSocket::disconnected, this, &MainWindow::onClientDisconnected);
    if (client) {
        clients.append(client);
    }
}
void MainWindow::onTextMessageReceived(QString message){
    QJsonDocument jsonDoc = QJsonDocument::fromJson(message.toUtf8());
    QJsonObject jsonObj=jsonDoc.object();
    jsonObj["url"]=jsonObj["url"].toString().replace("file:///","");
    if(jsonObj["action"].toString()=="pos"){
        settings.setValue("url", jsonObj["url"].toString());
    }

    if(jsonObj["action"].toString()=="finish"){
        finish();
    }
    if(jsonObj["action"].toString()=="start"){

        WebVideoCaptureThread *videoThread;
        videoThread = new WebVideoCaptureThread(textList,jsonObj["duration"].toInt());
        connect(videoThread, &WebVideoCaptureThread::broadcastMessage, this, &MainWindow::broadcastMessage);
        videoThread->start();
    }

    if(jsonObj["action"].toString()=="markdown"){
        video=jsonObj["url"].toString();
        if(jsonObj["data"].toString().startsWith("data")){
            QByteArray imageData = QByteArray::fromBase64(jsonObj["data"].toString().replace("data:image/png;base64,", "").toLocal8Bit());
            QImage qtImage;
            bool loaded = qtImage.loadFromData(imageData);
            if(loaded){

                if(jsonObj["mode"].toInt()==0){
                    cv::Mat cvImage(qtImage.height(), qtImage.width(), CV_8UC4, const_cast<uchar*>(qtImage.bits()), qtImage.bytesPerLine());
                    cv::cvtColor(cvImage, cvImage, cv::COLOR_BGRA2BGR);
                   updateFrame(cvImage, jsonObj["duration"].toInt(), jsonObj["time"].toInt() , jsonObj["time"].toInt());

                }
                if(jsonObj["mode"].toInt()==1){
                    cv::Mat cvImage(qtImage.height(), qtImage.width(), CV_8UC4, const_cast<uchar*>(qtImage.bits()), qtImage.bytesPerLine());
                    cv::cvtColor(cvImage, cvImage, cv::COLOR_BGRA2BGR);
                    updateFrame(cvImage,jsonObj["duration"].toInt(), jsonObj["time"].toInt() , jsonObj["time"].toInt());
                }
            }
        }else if(jsonObj.contains("time")) {
            QTime time(0,0,0);
            time= time.addSecs(jsonObj["time"].toInt());
            appendLink(video,time.toString("hh:mm:ss"));
        }

        if(!jsonObj["text"].toString().isEmpty()){
            appendText(jsonObj["text"].toString());
        }

    }
    if(jsonObj["action"].toString()=="cn"){

        QString url = jsonObj["url"].toString();
        QUrl cn(url);
        QUrlQuery query(cn);
        QString link = query.queryItemValue("url");
        if(!cn.fragment().isEmpty()){
            link+="#"+cn.fragment();
        }
        QString *time = new QString(query.queryItemValue("time"));
        int id=url.replace("cn:","").replace("/","").toInt();
        quint32 ipNumber = QHostAddress(url).toIPv4Address();
        if(ipNumber>0){
            id=ipNumber;
        }
        if(id>0){
            QJsonDocument jsonDoc = QJsonDocument::fromJson(getLink(QString::number(id)).toUtf8());
            QJsonObject jsonObj=jsonDoc.object();
            link=jsonObj["url"].toString();
            time=new QString(jsonObj["time"].toString());
        }
        bool isv=isVideoLink(link);
        timer->disconnect();
        timer->stop();
            if(!time->contains(".")){
                *time=*time+".000";
            }
            if(isv){
                if(PotPlayer()){
                    if(settings.value("url")==link){
                        SendMessage(PotPlayer(), 0x0400, 0x5005, QTime::fromString(*time,"hh:mm:ss.zzz").msecsSinceStartOfDay());
                        SendMessage(PotPlayer(), 0x0400, 0x5007, 2);
                    }else{
                        settings.setValue("url",link);
                        QString program = settings.value("potplayer").toString();
                        QString arguments =link + " /seek=" + *time;
                        QProcess::startDetached(program, QStringList() << arguments);

                    }
                }else{
                    settings.setValue("url",link);
                    QString program = settings.value("potplayer").toString();
                    QString arguments =link + " /seek=" + *time;
                    QProcess::startDetached(program, QStringList() << arguments);
                }
            } else{


                QJsonObject json;
                json["action"] = "seek";

                QUrl url(link);
                QUrlQuery query;
                query.setQuery(url.query());
                QString cnt =QString::number(convertTimeToSeconds(time));
                query.addQueryItem("cnt",cnt);
                url.setQuery(query);
                json["data"] =url.toString();
                json["cnt"] =cnt;
                json["url"] =link;
                json["text"]= jsonObj["text"].toBool();
                QString jsonString = QJsonDocument(json).toJson(QJsonDocument::Compact);
                onTextMessageReceived(jsonString);
            }


    }
    if(operation==1&&jsonObj["action"].toString()=="pos"){
        double result = jsonObj["data"].toDouble();
        QTime time(0, 0, 0, 0);
        time = time.addMSecs(int(result * 1000));
        QString formattedTime = time.toString("hh:mm:ss.zzz");
        QString encodedUrl = jsonObj["url"].toString();
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


    }

    if(jsonObj["action"].toString()=="seek"){

        if(clients.size()>1){
            broadcastMessage(message);

        }else if(jsonObj["text"].toBool()&&clients.size()>0){
            broadcastMessage(message);
        }
        else{
            QUrl url(jsonObj["data"].toString());
            QDesktopServices::openUrl(url);
        }

    }

    if(operation==2&&jsonObj["action"].toString()=="snapshots"){
        if(jsonObj["data"].toString().startsWith("data")){
            QByteArray imageData = QByteArray::fromBase64(jsonObj["data"].toString().replace("data:image/png;base64,", "").toLocal8Bit());
            QImage image;
            bool loaded = image.loadFromData(imageData);

            if(loaded){
                QApplication::clipboard()->setImage(image);
                simulateCtrlV();

            }
        }
        operation=0;
    }

    loop.quit();
}
void MainWindow::onClientDisconnected(){
    QWebSocket *client = qobject_cast<QWebSocket*>(sender());
    clients.removeOne(client);
    client->deleteLater();
}
void MainWindow::broadcastMessage(QString message){
    for (QWebSocket *client : clients) {
        if (client->state() == QAbstractSocket::ConnectedState) {
            client->sendTextMessage(message);
        }
    }
}


void MainWindow::on_pushButton_4_clicked()
{
    QUrl url("https://ztough.cn/Buy/ClipNote");
    QDesktopServices::openUrl(url);

}
void MainWindow::finish()
{


    ui->progressBar->setValue(100);
    QApplication::clipboard()->setText(markdown.join("").trimmed());
    QMessageBox::information(this, tr("提示"), "图片已成功生成到指定文件夹，笔记内容已复制到剪贴板(请自行粘贴到笔记)");

}

void MainWindow::appendText(QString text)
{
    markdown+="- "+text+"\n";
}

void MainWindow::appendLink(QString url, QString time)
{

        QString id=addLink(url,time);
        QString link = ui->lineEdit->text()
                           .replace("$times",time)
                           .replace("$time",time.replace(QRegularExpression("(\\d\\d:\\d\\d:\\d\\d)\\.\\d\\d\\d"), "\\1"))
                           .replace("$url",id)
                           .replace("$name", QFileInfo(url).baseName())
                           .replace("$filename", QFileInfo(url).fileName())
                           .replace("$path", url);
        markdown+="- "+link+"\n";



}



void MainWindow::updateFrame(const Mat &frame, int total, int current,int second)
{

    ui->progressBar->setValue(static_cast<int>((static_cast<double>(current) / total) * 100));

        QString fileName = QString("%1%2.jpg").arg(prefix).arg(frameCounter++);

        QString filePath = directory  +"/"+ fileName;
        imwrite(filePath.toLocal8Bit().constData(), frame);

        markdown+="- "+QString("![](%1)").arg(filePath)+"\n";


    QTime time(0, 0);
    time= time.addSecs(second);
    QString timestr=time.toString("hh:mm:ss");

    appendLink(video,timestr);



}

MainWindow::~MainWindow()
{
    delete ui;
    delete trayIcon;
    delete trayMenu;
    delete actionShow;
    delete actionExit;
}






void MainWindow::on_checkBox_5_stateChanged(int arg1)
{

    settings.setValue(sender()->objectName(),arg1);

}
void MainWindow::on_checkBox_6_stateChanged(int arg1)
{

    settings.setValue(sender()->objectName(),arg1);
    if (arg1)
    {
        QSettings settings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
        settings.setValue("ClipNote", QCoreApplication::applicationFilePath().replace("/", "\\"));
    }
    else
    {
        QSettings settings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
        settings.remove("ClipNote");
    }

}


void MainWindow::on_keySequenceEdit_1_editingFinished()
{
    if(clearKey(qobject_cast<QKeySequenceEdit *>(sender()))){
        settings.setValue(sender()->objectName(),qobject_cast<QKeySequenceEdit *>(sender())->keySequence());
        keyList[0]->setRegistered(false);
        keyList[0]->setShortcut(qobject_cast<QKeySequenceEdit *>(sender())->keySequence());
        keyList[0]->setRegistered(true);
    }

}


void MainWindow::on_keySequenceEdit_2_editingFinished()
{
    if(clearKey(qobject_cast<QKeySequenceEdit *>(sender()))){
        settings.setValue(sender()->objectName(),qobject_cast<QKeySequenceEdit *>(sender())->keySequence());
        keyList[1]->setRegistered(false);
        keyList[1]->setShortcut(qobject_cast<QKeySequenceEdit *>(sender())->keySequence());
        keyList[1]->setRegistered(true);
    }

}



void MainWindow::on_keySequenceEdit_6_editingFinished()
{
    if(clearKey(qobject_cast<QKeySequenceEdit *>(sender()))){
        settings.setValue(sender()->objectName(),qobject_cast<QKeySequenceEdit *>(sender())->keySequence());
        keyList[5]->setRegistered(false);
        keyList[5]->setShortcut(qobject_cast<QKeySequenceEdit *>(sender())->keySequence());
        keyList[5]->setRegistered(true);
    }
}



void MainWindow::on_keySequenceEdit_9_editingFinished()
{
    if(clearKey(qobject_cast<QKeySequenceEdit *>(sender()))){
        settings.setValue(sender()->objectName(),qobject_cast<QKeySequenceEdit *>(sender())->keySequence());
        keyList[8]->setRegistered(false);
        keyList[8]->setShortcut(qobject_cast<QKeySequenceEdit *>(sender())->keySequence());
        keyList[8]->setRegistered(true);
    }
}


void MainWindow::on_keySequenceEdit_10_editingFinished()
{
    if(clearKey(qobject_cast<QKeySequenceEdit *>(sender()))){
        settings.setValue(sender()->objectName(),qobject_cast<QKeySequenceEdit *>(sender())->keySequence());
        keyList[9]->setRegistered(false);
        keyList[9]->setShortcut(qobject_cast<QKeySequenceEdit *>(sender())->keySequence());
        keyList[9]->setRegistered(true);
    }
}


void MainWindow::on_keySequenceEdit_11_editingFinished()
{
    if(clearKey(qobject_cast<QKeySequenceEdit *>(sender()))){
        settings.setValue(sender()->objectName(),qobject_cast<QKeySequenceEdit *>(sender())->keySequence());
        keyList[10]->setRegistered(false);
        keyList[10]->setShortcut(qobject_cast<QKeySequenceEdit *>(sender())->keySequence());
        keyList[10]->setRegistered(true);
    }
}


void MainWindow::on_keySequenceEdit_12_editingFinished()
{
    if(clearKey(qobject_cast<QKeySequenceEdit *>(sender()))){
        settings.setValue(sender()->objectName(),qobject_cast<QKeySequenceEdit *>(sender())->keySequence());
        keyList[11]->setRegistered(false);
        keyList[11]->setShortcut(qobject_cast<QKeySequenceEdit *>(sender())->keySequence());
        keyList[11]->setRegistered(true);
    }
}

void MainWindow::on_keySequenceEdit_13_editingFinished()
{
    if(clearKey(qobject_cast<QKeySequenceEdit *>(sender()))){
        settings.setValue(sender()->objectName(),qobject_cast<QKeySequenceEdit *>(sender())->keySequence());
        keyList[12]->setRegistered(false);
        keyList[12]->setShortcut(qobject_cast<QKeySequenceEdit *>(sender())->keySequence());
        keyList[12]->setRegistered(true);
    }
}


void MainWindow::on_keySequenceEdit_14_editingFinished()
{
    if(clearKey(qobject_cast<QKeySequenceEdit *>(sender()))){
        settings.setValue(sender()->objectName(),qobject_cast<QKeySequenceEdit *>(sender())->keySequence());
        keyList[13]->setRegistered(false);
        keyList[13]->setShortcut(qobject_cast<QKeySequenceEdit *>(sender())->keySequence());
        keyList[13]->setRegistered(true);
    }
}


void MainWindow::on_keySequenceEdit_15_editingFinished()
{
    if(clearKey(qobject_cast<QKeySequenceEdit *>(sender()))){
        settings.setValue(sender()->objectName(),qobject_cast<QKeySequenceEdit *>(sender())->keySequence());
        keyList[14]->setRegistered(false);
        keyList[14]->setShortcut(qobject_cast<QKeySequenceEdit *>(sender())->keySequence());
        keyList[14]->setRegistered(true);
    }
}
void MainWindow::on_keySequenceEdit_16_editingFinished()
{
    if(clearKey(qobject_cast<QKeySequenceEdit *>(sender()))){
        settings.setValue(sender()->objectName(),qobject_cast<QKeySequenceEdit *>(sender())->keySequence());
        keyList[15]->setRegistered(false);
        keyList[15]->setShortcut(qobject_cast<QKeySequenceEdit *>(sender())->keySequence());
        keyList[15]->setRegistered(true);
    }
}

void MainWindow::on_pushButton_11_clicked()
{
    QUrl url("https://tkjpydwgii.feishu.cn/wiki/Y58ZwuubmiOd28kQqgPcHJ8HnLd?from=from_copylink");
    QDesktopServices::openUrl(url);
}







void MainWindow::on_pushButton_12_clicked()
{

        directory = QFileDialog::getExistingDirectory(this,
                                                      QObject::tr("选择保存图片的文件夹"),
                                                      settings.value("imagepath").toString(),
                                                      QFileDialog::ShowDirsOnly
                                                          | QFileDialog::DontResolveSymlinks);

        if(directory.isEmpty()){return;}else{
            settings.setValue("imagepath",directory);}
        prefix=QInputDialog::getText(this,"请输入图片前缀" ,"可为空", QLineEdit::Normal,"");


    frameCounter=0;
    markdown.clear();

    QMessageBox msgBox;
    msgBox.setWindowTitle("选择本地视频，还是网络视频");
    msgBox.setText("是代表本地视频，否代表网络视频(保持网页处于前台状态(不能切换标签页),若无效请更新浏览器扩展，若无图片则网页跨域限制属于正常)。");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);

    int ret = msgBox.exec();

    switch (ret) {
    case QMessageBox::Yes:
        video = QFileDialog::getOpenFileName(this,"选择视频文件",settings.value("history").toString(),"视频文件 (*.*)");
        if (!video.isEmpty()) {

            QString lastDirectory = QFileInfo(video).absolutePath();
            settings.setValue("history",lastDirectory);
            VideoCaptureThread *videoThread;
            videoThread = new VideoCaptureThread(video,textList);
            connect(videoThread, &VideoCaptureThread::frameCaptured, this, &MainWindow::updateFrame);
            connect(videoThread, &VideoCaptureThread::finish, this, &MainWindow::finish);
            connect(videoThread, &VideoCaptureThread::appendText, this, &MainWindow::appendText);
            connect(videoThread, &VideoCaptureThread::appendLink, this, &MainWindow::appendLink);
            videoThread->start();


        }

        break;
    case QMessageBox::No:


        QJsonObject json;
        json["action"]="start";
        broadcastMessage(QJsonDocument(json).toJson(QJsonDocument::Compact));

        break;
    }





}







void MainWindow::on_pushButton_13_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "选择PotPlayer.exe",
                                                    "",
                                                    "可执行程序 (*.exe)");

    if (!fileName.isEmpty()) {
        if(!fileName.toLower().contains("potplayer")){
            QMessageBox::critical(this, tr("错误"), "所选的不是PotPlayer.exe", QMessageBox::Ok);
            return;
        }else{
            settings.setValue("potplayer",fileName);
            ui->lineEdit_9->setText(fileName);
        }
    }
}



void MainWindow::on_lineEdit_textChanged(const QString &arg1)
{
    settings.setValue("link_style",arg1);
}




void MainWindow::on_pushButton_16_clicked()
{

    QString fileName = QFileDialog::getOpenFileName(this,"选择clipnote.db","","数据库 (*.db)");

    if (!fileName.isEmpty()) {
        if(!fileName.toLower().contains("clipnote.db")){
            QMessageBox::critical(this, tr("错误"), "所选的不是clipnote.db", QMessageBox::Ok);
            return;
        }else{
            settings.setValue("database",fileName);
            ui->lineEdit_11->setText(fileName);
        }
    }
}


void MainWindow::on_lineEdit_9_textChanged(const QString &arg1)
{
    if(arg1.isEmpty()){
        settings.remove("potplayer");
    }
}





void MainWindow::on_lineEdit_11_textChanged(const QString &arg1)
{
    if(arg1.isEmpty()){
        settings.remove("database");
    }

}








void MainWindow::on_pushButton_6_clicked()
{
    QUrl url("https://ztough.cn");
    QDesktopServices::openUrl(url);
}






void MainWindow::on_pushButton_8_clicked()
{
    text = QInputDialog::getMultiLineText(this, "请输入自定义生成内容", "请输入内容：", text);
    if(text.trimmed().isEmpty()){
        textList.clear();
        ui->pushButton_8->setText("自定义");
    }else{
        textList=text.split("\n");
        ui->pushButton_8->setText("已自定义");
    }

}








void MainWindow::on_lineEdit_4_textChanged(const QString &arg1)
{
    settings.setValue("interval",arg1);
}



