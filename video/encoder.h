#ifndef ENCODER_H
#define ENCODER_H

#include <string>
#include <QImage>

struct AVCodecContext;
struct AVCodec;
struct AVFrame;
struct AVPacket;

class VideoEncoder
{
public:
    VideoEncoder() : ctx(nullptr), codec(nullptr), frame(nullptr), pkt(nullptr), pts(0), file(nullptr), av_error(0) {}
    VideoEncoder(int width, int height, int frameRate, const char *fileName) : VideoEncoder() { create(width, height, frameRate, fileName); }
    ~VideoEncoder() { clear(); }

    void create(int width, int height, int frameRate, const char *fileName);
    void clear();

    QImage &nextFrame() { return image; }
    void encodeFrame();

    int getLastErrorCode() { return av_error; }
    const std::string &getLastError() { return last_error; }

private:
    AVCodecContext *ctx;
    const AVCodec *codec;

    AVFrame *frame;
    AVPacket *pkt;
    int pts;

    QImage image;

    FILE *file;

    int av_error;
    std::string last_error;

    AVCodecContext *allocContext(int width, int height, int framerate);
    AVFrame *allocFrame();
};

#endif // ENCODER_H
