#ifndef VIDEO_H
#define VIDEO_H

#include <QTemporaryFile>
#include <QImage>

#include <string>

struct AVCodecContext;
struct AVCodec;
struct AVFrame;
struct AVPacket;

class Video
{
public:
    Video();
    Video(const Video &src) : Video() { operator=(src); }
    Video(Video &&src) : Video() { operator=(std::move(src)); }
    ~Video() { cleanUp(); }

    void create(int width, int height, int frame_rate);
    void encodeFrame();

    QImage &currentFrame() { return image; }

    Video &operator=(const Video &) { throw "Video cannot be copied"; }
    Video &operator=(Video &&src);

    int av_error;
    std::string last_error;

private:
    void allocContext();
    void allocFrame();

    void initialize();
    void cleanUp();

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
