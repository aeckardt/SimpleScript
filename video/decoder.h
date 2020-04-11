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
    bool readFrame();
    void scaleFrame();

    bool eof() const { return _eof; }

    Image &frame() { return image; }

    int av_error;
    QString last_error;

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

    struct AVFrame *frame_;
    struct AVFrame *frame_rgb;

    struct SwsContext *sws_ctx;

    bool _eof;

    int frame_finished;
    struct AVPacket *pkt;

    int num_bytes;
    uint8_t *buffer;

    int frame_rate;

    friend class DecoderThread;
};

class DecoderThread : public QThread
{
    Q_OBJECT

public:
    DecoderThread(QObject *parent = nullptr);
    ~DecoderThread() override;

    void setFile(const VideoFile &video);

    int frameRate() const { return decoder.frame_rate; }

    void next();
    void stop();

signals:
    void error(const QString &msg);
    void newFrame(const Image *image);
    void finished();

protected:
    void run() override;

private:
    const VideoFile *video;

    VideoDecoder decoder;

    QMutex mutex;
    QWaitCondition condition;

    bool continue_reading;
    bool quit;
};

#endif // DECODER_H
