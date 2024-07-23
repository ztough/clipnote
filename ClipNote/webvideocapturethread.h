#include <QThread>
#include <opencv2/opencv.hpp>
#include <QDebug>
#include <QImage>
#include <QSettings>
#include <QTime>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

using namespace cv;

extern QSettings settings;
class WebVideoCaptureThread : public QThread {
    Q_OBJECT
signals:
    void frameCaptured(const Mat& frame,int total,int current,int second,QString timestr);
     void broadcastMessage(QString msg);
     void over(int time);
public:
    WebVideoCaptureThread(QStringList textList,int frameCount)
        : textList(textList), frameCount(frameCount)
    {}



protected:
    void run() override {
        int intervalSeconds;
        intervalSeconds=settings.value("interval").toInt();
        int includeImg= 0;
        QTime start=QTime(0,0,0);
        QTime end=QTime(23,59,59);
        int startFrame=(start.msecsSinceStartOfDay()/1000);
        int endFrame=(end.msecsSinceStartOfDay()/1000);
        if(endFrame>frameCount){endFrame=frameCount;}
        if(textList.count()>0){
            QJsonArray arr;
            static const QRegularExpression reg(R"(\d{2}:\d{2}:\d{2}\.\d{3})");
            static const QRegularExpression regex(R"(\d{2}:\d{2}:\d{2})");
            static const QRegularExpression regex2(R"(\d{2}:\d{2})");
            static const QRegularExpression regex3(R"(\d{2}:\d{2}:\d{2}-\d{2}:\d{2}:\d{2})");
            for(auto &text : textList){
                if(text.trimmed().isEmpty()){continue;}
                QString temp = text;
                temp.remove(" ").replace(",",".").replace("-->","-");
                temp=temp.trimmed();
                QTime time;
                QRegularExpressionMatch match = reg.match(temp);
                bool flag = false;
                if (match.hasMatch()) {
                    flag=true;
                    time=QTime::fromString(match.captured(0),"hh:mm:ss.zzz");
                }
                if(time.msecsSinceStartOfDay()==0){
                    match=regex.match(temp);
                    if (match.hasMatch()) {
                        flag=true;
                        time=QTime::fromString(match.captured(0),"hh:mm:ss");

                    }
                }
                if(time.msecsSinceStartOfDay()==0){
                    match=regex2.match(temp);
                    if (match.hasMatch()) {
                        flag=true;
                        if(!match.captured(0).length()<=5){
                            time=QTime::fromString("00:"+match.captured(0),"hh:mm:ss");
                        }else{
                            time=QTime::fromString(match.captured(0),"hh:mm:ss");
                        }
                    }
                }

                if(!flag){
                    arr.append(text);
                }else{
                    int i=time.msecsSinceStartOfDay()/1000;
                    if(i<=frameCount){
                        arr.append(i);

                    }else{

                        arr.append(text);

                    }
                }
            }
            QJsonObject json;
            json["action"]="seekandsnapshots";
            json["arr"]=arr;
            json["mode"]=1;
            json["img"]=includeImg;
            emit  broadcastMessage(QJsonDocument(json).toJson(QJsonDocument::Compact));

        }else{
            QJsonObject json;
            json["action"]="seekandsnapshots";
            json["interval"]=intervalSeconds;
            json["start"]=startFrame;
            json["end"]=endFrame;
            json["mode"]=0;
            json["img"]=includeImg;
            emit  broadcastMessage(QJsonDocument(json).toJson(QJsonDocument::Compact));

        }

    }
private:
    QString videoPath;
    QStringList textList;
    int frameCount;
};
