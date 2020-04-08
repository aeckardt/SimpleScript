#ifndef VIDEO_H
#define VIDEO_H

#include <QImage>
#include <QString>

#include "image/image.h"
#include "videofile.h"

struct AVCodecContext;
struct AVCodec;
struct AVFrame;
struct AVPacket;

class VideoEncoder
{
public:
    VideoEncoder();
    ~VideoEncoder() { cleanUp(); }

    void setFile(const VideoFile &video_file) { this->video_file = video_file; }

    void create(int width, int height, int frame_rate);
    void encodeFrame() { encodeFrames(false); }
    void flush() { encodeFrames(true); }

    Image &nextFrame() { return image; }

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

    AVCodecContext *ctx;
    const AVCodec *codec;
    AVFrame *frame;
    AVPacket *pkt;
    int pts;

    Image image;

    VideoFile video_file;
    FILE *file;
};

#endif // VIDEO_H
