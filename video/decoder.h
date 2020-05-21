#ifndef DECODER_H
#define DECODER_H

#include <QSize>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QImage>

#include <vector>

#include "image/image.h"
#include "videofile.h"

class DecoderFrame
{
public:
    DecoderFrame() : DecoderFrame(0, 0) {}
    DecoderFrame(int width, int height);
    ~DecoderFrame() { cleanUpAll(); }

    bool isValid() const;

    void resize(int width, int height);
    void resizeHard(int width, int height) { resize(width, height); reset(); }

    const Image &image() const { return cycles[current].image; }

    void shift();
    void reset(); // -> shift one cycle

private:
    void alloc(size_t index);
    void allocAll();
    void cleanUp(size_t index);
    void cleanUpAll();

    struct AVFrame *frame() { return cycles[current].frame; }

    void errorMsg(const char *msg);

    int width;
    int height;
    int num_bytes;

    size_t current;

    struct Cycle
    {
        Image image;
        struct AVFrame *frame;
        uint8_t *buffer;
        bool need_resize;
        bool has_errors;
    };

    std::vector<Cycle> cycles;

    friend class VideoDecoder;
};

struct VideoInfo
{
    int width;
    int height;
    int framerate;
    int framecount;
};

class VideoDecoder
{
public:
    VideoDecoder();
    ~VideoDecoder() { cleanUp(); }

    void open(const VideoFile &video_file);
    bool readFrame();
    void swsScale();

    inline const VideoInfo &info() const { return _info; }

    inline bool isOpen() const { return !_eof && av_error == 0; }
    inline const Image &frame() const { return frame_rgb.image(); }
    inline bool eof() const { return _eof; }

    void resize(const QSize &size);

    void seek(int n_frame);

    int av_error;
    QString last_error;

private:
    void errorMsg(const char *msg);

private:
    void initialize();
    void cleanUp();

    Image image;

    struct AVFormatContext *format_ctx;
    const struct AVStream *video_stream;

    int frame_counter;
    struct AVCodecParameters *codec_par;

    struct AVCodecContext *codec_ctx;
    struct AVCodec        *codec;

    VideoInfo _info;

    struct AVFrame *frame_;
    DecoderFrame frame_rgb;

    struct AVPacket *pkt;
    struct SwsContext *sws_ctx;

    int frame_finished;

    bool _eof;

    QSize initial_size;

    friend class DecoderThread;
};

class DecoderThread : public QThread
{
    Q_OBJECT

public:
    DecoderThread(QObject *parent = nullptr);
    ~DecoderThread() override;

    void setFile(const VideoFile &video);

    const VideoInfo &info() const { return decoder.info(); }

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
    QSize initial_size;

    VideoDecoder decoder;
    QSize new_size;

    QMutex mutex;
    QWaitCondition condition;

    bool continue_reading;
    bool quit;
};

#endif // DECODER_H
