#ifndef VIDEO_H
#define VIDEO_H

#include <QObject>
#include <QImage>
#include <QString>

#include "image/image.h"
#include "videofile.h"

class VideoEncoder : public QObject
{
    Q_OBJECT

public:
    VideoEncoder();
    ~VideoEncoder() { cleanUp(); }

    void open(const VideoFile &video_file, int width, int height, int frame_rate);
    void writeFrame() { encodeFrame(false); }
    void flush() { encodeFrame(true); }

    Image &frame() { return image; }

    int av_error;
    QString last_error;

private slots:
    void allocFrameBuffer(uint8_t *bits);

private:
    void allocContext();
    void allocFrame();

    void initialize();
    void cleanUp();

    void encodeFrame(bool flush);

    void errorMsg(const char *msg);

    int width;
    int height;
    int frame_rate;

    struct AVCodecContext *ctx;
    const struct AVCodec *codec;
    struct AVFrame *frame_;
    struct AVPacket *pkt;
    int pts;

    Image image;

    const VideoFile *video_file;
    FILE *file;
};

#endif // VIDEO_H
