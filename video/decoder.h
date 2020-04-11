#ifndef DECODER_H
#define DECODER_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QImage>

#include "image/image.h"
#include "videofile.h"

class VideoDecoder : QObject
{
    Q_OBJECT

public:
    VideoDecoder();
    ~VideoDecoder() { cleanUp(); }

    void open(const VideoFile &video_file);
    void decodeFrame();

    bool eof() const { return _eof; }

    Image &nextFrame() { return image; }

private:
    void errorMsg(const char *msg);

private:
    void initialize();
    void cleanUp();

    Image image;

    struct AVFormatContext *format_ctx;
    int video_stream;

    int frame_counter;
    struct AVCodecParameters *codec_par;

    struct AVCodecContext *codec_ctx;
    struct AVCodec        *codec;

    struct AVFrame *frame;
    struct AVFrame *frame_rgb;

    struct SwsContext *sws_ctx;

    bool _eof;

    int frame_finished;
    struct AVPacket *pkt;

    int num_bytes;
    uint8_t *buffer;

    int av_error;

    int frame_rate;
};

class DecoderThread : public QThread
{
    Q_OBJECT

public:
    DecoderThread(QObject *parent = nullptr);
    ~DecoderThread() override;

    void setFile(const VideoFile &video);

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

    struct AVFormatContext *format_ctx;
    int video_stream;

    int frame_counter;
    struct AVCodecParameters *codec_par;

    struct AVCodecContext  *codec_ctx;
    struct AVCodec         *codec;

    struct AVFrame *frame;
    struct AVFrame *frame_rgb;

    uint8_t *buffer;
    int num_bytes;

    struct SwsContext *sws_ctx;
    int                frame_finished;
    struct AVPacket   *packet;

    int av_error;

    QImage image;
    int frame_rate;

    QMutex mutex;
    QWaitCondition condition;

    bool continue_reading;
    bool quit;
};

#endif // DECODER_H
