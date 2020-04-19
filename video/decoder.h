#ifndef DECODER_H
#define DECODER_H

#include <QSize>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QImage>

#include "image/image.h"
#include "videofile.h"

class VideoDecoder
{
public:
    VideoDecoder();
    ~VideoDecoder() { cleanUp(); }

    void open(const VideoFile &video_file);
    bool readFrame();
    void swsScale();

    void setTargetSize(const QSize &size);

    bool eof() const { return _eof; }

    const Image &frame() { return image; }

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

    int num_bytes;
    uint8_t *buffer;

    struct AVPacket *pkt;
    struct SwsContext *sws_ctx;

    int frame_finished;
    int frame_rate;

    bool _eof;

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

    void resize(const QSize &size);

signals:
    void error(const QString &msg);
    void newFrame(const Image *image);
    void finished();

protected:
    void run() override;

private:
    const VideoFile *video;

    VideoDecoder decoder;
    QSize new_size;

    QMutex mutex;
    QWaitCondition condition;

    bool continue_reading;
    bool quit;
};

#endif // DECODER_H
