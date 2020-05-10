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
    void addFrame();
    void finish();

    Image &frame() { return image; }

    int av_error;
    QString last_error;

private slots:
    void allocFrameBuffer(uint8_t *bits);

private:
    void allocFormatContext();
    void allocCodecContext();
    void writeHeader();
    void allocFrame();

    void initialize();
    void cleanUp();

    void encodeFrame(bool finish);

    void errorMsg(const char *msg);

    int width;
    int height;
    int frame_counter;
    int frame_rate;

    struct AVOutputFormat *output_fmt;
    struct AVFormatContext *format_ctx;
    struct AVStream *video_stream;

    const struct AVCodec *codec;
    struct AVCodecContext *codec_ctx;

    struct AVFrame *frame_rgb;
    struct AVPacket *pkt;

    Image image;

    const VideoFile *video_file;
};

#endif // VIDEO_H
