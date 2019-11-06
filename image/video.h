#ifndef VIDEO_H
#define VIDEO_H

#include <vector>
#include <sys/time.h>

#include <QEventLoop>
#include <QImage>
#include <QTimer>
#include <QString>
#include <QElapsedTimer>

#include "hotkey/qhotkey.h"

class VideoFrame
{
public:
    VideoFrame() : img(nullptr), ms(0) {}
    VideoFrame(const VideoFrame &src) { img = new QImage(*src.img); ms = src.ms; }
    VideoFrame(VideoFrame &&src) { img = src.img; src.img = nullptr; ms = src.ms; }
    VideoFrame(const QImage &img, qint64 ms) { this->img = new QImage(img); this->ms = ms; }
    VideoFrame(QImage &&img, qint64 ms) { this->img = new QImage(std::move(img)); this->ms = ms; }
    ~VideoFrame() { if (img != nullptr) { delete img; } }

    const QImage &image() const { return *img; }
    qint64 msFromStart() const { return ms; }

    VideoFrame &operator=(const VideoFrame& src) { img = new QImage(*src.img); return *this; }

private:
    QImage *img;

    qint64 ms;
};

class Video
{
public:
    Video() {}
    Video(const Video &src) { frames = src.frames; }
    Video(Video &&src) { frames = std::move(src.frames); }
    ~Video() {}

    void addFrame(const VideoFrame &frame) { frames.push_back(frame); }
    void addFrame(VideoFrame &&frame) { frames.push_back(std::move(frame)); }

    const VideoFrame &frame(size_t index) const { return frames[index]; }

    size_t size() const { return frames.size(); }

    bool load(const QString &str);
    bool save(const QString &str) const;

    void compress();

    Video &operator=(const Video &src) { frames = src.frames; return *this; }
    Video &operator=(Video &&src) { frames = std::move(src.frames); return *this; }

    qint64 png_size;
    qint64 compressed_size;

private:
    std::vector<VideoFrame> frames;

    std::vector<QByteArray> compressed_frames;
};

class Recorder : public QObject
{
    Q_OBJECT

public:
    explicit Recorder(Video &video_ref, int frame_rate) : Recorder(QRect(), video_ref, frame_rate) {}
    explicit Recorder(const QRect &rect, Video &video_ref, int frame_rate);

    void setHotkey(const QString &keySequence) { hotkey.setShortcut(QKeySequence(keySequence)); }

    void exec();

signals:

public slots:
    void hotkeyPressed();
    void timeout();

private:
    QRect rect;
    Video *video;
    int frame_rate;
    int interval; // in milliseconds

    QEventLoop loop;
    QTimer timer;
    QHotkey hotkey;
    QElapsedTimer elapsed_timer;
};

#endif // VIDEO_H
