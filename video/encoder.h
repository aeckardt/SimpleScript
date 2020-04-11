#ifndef VIDEO_H
#define VIDEO_H

#include <QImage>
#include <QString>

#include "image/image.h"
#include "videofile.h"

class VideoEncoder
{
public:
    VideoEncoder();
    ~VideoEncoder() { cleanUp(); }

    void setFile(const VideoFile &video_file) { this->video_file = video_file; }

    void create(int width, int height, int frame_rate);
    void writeFrame() { encodeFrames(false); }
    void flush() { encodeFrames(true); }

    Image &frame() { return image; }

    int av_error;
    QString last_error;

private:
    void allocContext();
    void allocFrame();

    void initialize();
    void cleanUp();

    void encodeFrames(bool flush);

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

    VideoFile video_file;
    FILE *file;
};

#endif // VIDEO_H
