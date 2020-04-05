#ifndef VIDEO_H
#define VIDEO_H

#include <QTemporaryFile>
#include <QImage>

#include <string>

#include <QDebug>

struct AVCodecContext;
struct AVCodec;
struct AVFrame;
struct AVPacket;

class Video
{
public:
    Video();
    Video(Video &&src) : Video() { operator=(std::move(src)); }
    ~Video() { cleanUp(); }

    void create(int width, int height, int frame_rate);
    void encodeFrame() { encodeFrames(false); }
    void flush() { encodeFrames(true); }

    QImage &nextFrame() { return image; }

    Video &operator=(Video &&src);

    int av_error;
    std::string last_error;

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

    QImage image;

    QTemporaryFile temp_file;
    FILE *file;
};

#endif // VIDEO_H
