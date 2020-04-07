#ifndef VIDEOREADER_H
#define VIDEOREADER_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QImage>

struct AVFormatContext;
struct AVCodecContext;
struct AVCodecParameters;
struct AVCodec;
struct AVFrame;
struct AVPacket;

class VideoReader : public QThread
{
    Q_OBJECT

public:
    VideoReader(QObject *parent = nullptr);
    ~VideoReader() override;

    void setFileName(const QString &fileName);

    void next();
    void stop();

signals:
    void error(const QString &msg);
    void newFrame(const QImage *image);
    void finished();

protected:
    void run() override;

private:
    QString fileName;

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

    QMutex mutex;
    QWaitCondition condition;

    bool continue_reading;
    bool quit;
};

#endif // VIDEOREADER_H
