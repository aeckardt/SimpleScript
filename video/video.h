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
    Video(int width, int height, int frame_rate);
    ~Video() { cleanUp(); }

    QImage &currentFrame() { return image; }

    void create();
    void encodeFrame();

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
