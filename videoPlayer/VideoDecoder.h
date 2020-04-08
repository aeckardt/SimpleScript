#ifndef VIDEODECODER_H
#define VIDEODECODER_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QImage>

#include "video/videofile.h"

struct AVFormatContext;
struct AVCodecContext;
struct AVCodecParameters;
struct AVCodec;
struct AVFrame;
struct AVPacket;

class VideoDecoder : public QThread
{
    Q_OBJECT

public:
    VideoDecoder(QObject *parent = nullptr);
    ~VideoDecoder() override;

    void setVideo(const VideoFile &video);

    int frameRate() const { return frame_rate; }

    void next();
    void stop();

signals:
    void error(const QString &msg);
    void newFrame(const QImage *image);
    void finished();

protected:
    void run() override;

private:
    const VideoFile *video;

    AVFormatContext *format_ctx;
    int              video_stream;

    int frame_counter;
    AVCodecParameters *codec_par;

    AVCodecContext  *codec_ctx;
    AVCodec         *codec;

    AVFrame *frame;
    AVFrame *frame_rgb;

    uint8_t *buffer;
    int num_bytes;

    struct SwsContext *sws_ctx;
    int                frame_finished;
    AVPacket          *packet;

    int av_error;

    QImage image;
    int frame_rate;

    QMutex mutex;
    QWaitCondition condition;

    bool continue_reading;
    bool quit;
};

#endif // VIDEODECODER_H
