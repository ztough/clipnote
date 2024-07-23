#include <QThread>
#include <opencv2/opencv.hpp>
#include <QDebug>
#include <QImage>
#include <QSettings>
#include <QTime>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

using namespace cv;
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}
extern QTime Total;
extern QSettings settings;
extern void broadcastMessage(QString msg);
class VideoCaptureThread : public QThread {
    Q_OBJECT
signals:
    void frameCaptured(const Mat& frame,int total,int current,int second);
    void finish();
    void appendText(QString text);
    void appendLink(QString url,QString time);
public:
    VideoCaptureThread(const QString& videoPath,QStringList textList)
        : videoPath(videoPath), textList(textList)
    {}

protected:
    void run() override {
        int intervalSeconds;
        intervalSeconds=settings.value("interval",10).toInt();
        AVFormatContext *formatContext = avformat_alloc_context();
        if (avformat_open_input(&formatContext, videoPath.toStdString().c_str(), nullptr, nullptr) != 0) {
            qWarning() << "Error opening video file:" << videoPath;
            return;
        }

        if (avformat_find_stream_info(formatContext, nullptr) < 0) {
            qWarning() << "Could not find stream information";
            avformat_close_input(&formatContext);
            return;
        }

        int videoStreamIndex = -1;
        for (unsigned int i = 0; i < formatContext->nb_streams; i++) {
            if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                videoStreamIndex = i;
                break;
            }
        }

        if (videoStreamIndex == -1) {
            qWarning() << "Could not find a video stream";
            avformat_close_input(&formatContext);
            return;
        }

        AVCodecParameters *codecParameters = formatContext->streams[videoStreamIndex]->codecpar;
        const AVCodec *codec = avcodec_find_decoder(codecParameters->codec_id);
        if (!codec) {
            qWarning() << "Unsupported codec";
            avformat_close_input(&formatContext);
            return;
        }

        AVCodecContext *codecContext = avcodec_alloc_context3(codec);
        if (avcodec_parameters_to_context(codecContext, codecParameters) < 0) {
            qWarning() << "Could not copy codec context";
            avcodec_free_context(&codecContext);
            avformat_close_input(&formatContext);
            return;
        }

        if (avcodec_open2(codecContext, codec, nullptr) < 0) {
            qWarning() << "Could not open codec";
            avcodec_free_context(&codecContext);
            avformat_close_input(&formatContext);
            return;
        }

        AVFrame *frame = av_frame_alloc();
        AVPacket *packet = av_packet_alloc();
        struct SwsContext *swsContext = sws_getContext(
            codecContext->width, codecContext->height, codecContext->pix_fmt,
            codecContext->width, codecContext->height, AV_PIX_FMT_BGR24,
            SWS_BILINEAR, nullptr, nullptr, nullptr);

        double fps = av_q2d(formatContext->streams[videoStreamIndex]->avg_frame_rate);
        int64_t duration = formatContext->duration / (double)AV_TIME_BASE;
        int64_t frameCount = static_cast<int64_t>(duration * fps);
        int intervalFrames = static_cast<int>(fps * intervalSeconds);


        if(textList.count()>0){
            static const QRegularExpression reg(R"(\d{2}:\d{2}:\d{2}\.\d{3})");
            static const QRegularExpression regex(R"(\d{2}:\d{2}:\d{2})");
            static const QRegularExpression regex2(R"(\d{2}:\d{2})");

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
                   emit appendText(text);
                }else{
                    int i=(time.msecsSinceStartOfDay()/1000)*fps;
                    if(i<=frameCount){
                            int64_t targetTimestamp = av_rescale_q(i, av_inv_q(av_d2q(fps, 1000000)), formatContext->streams[videoStreamIndex]->time_base);

                            if (av_seek_frame(formatContext, videoStreamIndex,targetTimestamp, AVSEEK_FLAG_BACKWARD) < 0) {
                                std::cerr << "Error seeking to target PTS." << std::endl;
                                break;
                            }
                            avcodec_flush_buffers(codecContext);
                            while (av_read_frame(formatContext, packet) >= 0) {

                                if (packet->stream_index == videoStreamIndex) {

                                    if (avcodec_send_packet(codecContext, packet) == 0) {

                                        if (avcodec_receive_frame(codecContext, frame) == 0) {

                                            cv::Mat img(codecContext->height, codecContext->width, CV_8UC3);
                                            uint8_t *data[1] = { img.data };
                                            int linesize[1] = { static_cast<int>(img.step[0]) };
                                            sws_scale(swsContext, frame->data, frame->linesize, 0, codecContext->height, data, linesize);
                                            emit frameCaptured(img, frameCount, i, i / fps);
                                            break;
                                        }

                                    }
                                }
                                av_packet_unref(packet);
                            }




                    }else{
                        emit appendText(text);
                    }
                }
            }
        }else{
            for(int i=0;i<=frameCount;i+=intervalFrames){

                    int64_t targetTimestamp = av_rescale_q(i, av_inv_q(av_d2q(fps, 1000000)), formatContext->streams[videoStreamIndex]->time_base);

                    if (av_seek_frame(formatContext, videoStreamIndex,targetTimestamp, AVSEEK_FLAG_BACKWARD) < 0) {
                        std::cerr << "Error seeking to target PTS." << std::endl;
                        break;
                    }
                    avcodec_flush_buffers(codecContext);

                    while (av_read_frame(formatContext, packet) >= 0) {

                        if (packet->stream_index == videoStreamIndex) {

                            if (avcodec_send_packet(codecContext, packet) == 0) {

                                if (avcodec_receive_frame(codecContext, frame) == 0) {

                                    cv::Mat img(codecContext->height, codecContext->width, CV_8UC3);
                                    uint8_t *data[1] = { img.data };
                                    int linesize[1] = { static_cast<int>(img.step[0]) };
                                    sws_scale(swsContext, frame->data, frame->linesize, 0, codecContext->height, data, linesize);
                                    emit frameCaptured(img, frameCount, i , i / fps);

                                    break;
                                }

                            }
                        }
                        av_packet_unref(packet);
                    }



            }
        }

        av_frame_free(&frame);
        av_packet_free(&packet);
        avcodec_free_context(&codecContext);
        avformat_close_input(&formatContext);
        sws_freeContext(swsContext);
        emit finish();
    }
private:
    QString videoPath;
    QStringList textList;
};
