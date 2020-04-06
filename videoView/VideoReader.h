#ifndef VIDEOREADER_H
#define VIDEOREADER_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QImage>

struct AVFormatContext;
struct AVCodecContext;
struct AVCodecParameters;
struct AVCodec;
struct AVFrame;
struct AVPacket;

class VideoReader : public QThread
{
    Q_OBJECT

public:
    VideoReader(QObject *parent = nullptr);
    ~VideoReader() override;

    void setFileName(const QString &fileName);

    void next();
    void stop();

signals:
    void error(const QString &msg);
    void newFrame(const QImage *image);
    void finished();

protected:
    void run() override;

private:
    QString fileName;

    AVFormatContext *pFormatCtx;
    int              videoStream;

    int frame;
    AVCodecParameters *pCodecPar;

    AVCodecContext  *pCodecCtx;
    AVCodec         *pCodec;

    AVFrame *pFrame;
    AVFrame *pFrameRGB;

    uint8_t *buffer;
    int numBytes;

    struct SwsContext *sws_ctx;
    int                frameFinished;
    AVPacket          *packet;

    int av_error;

    QImage image;

    QMutex mutex;
    QWaitCondition condition;

    bool continueReading;
    bool quit;
};

#endif // VIDEOREADER_H
