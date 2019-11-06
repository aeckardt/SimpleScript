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
    VideoFrame() : img(nullptr), byte_array(nullptr), _size(nullptr), ms(0) {}
    VideoFrame(const VideoFrame &src);
    VideoFrame(VideoFrame &&src);

    VideoFrame(const QImage &img, qint64 ms) : byte_array(nullptr), _size(nullptr)
    { this->img = new QImage(img); this->ms = ms; }

    VideoFrame(QImage &&img, qint64 ms) : byte_array(nullptr), _size(nullptr)
    { this->img = new QImage(std::move(img)); this->ms = ms; }

    VideoFrame(const QByteArray &byte_array, const QSize &size, qint64 ms) : img(nullptr)
    { this->byte_array = new QByteArray(byte_array); this->_size = new QSize(size); this->ms = ms;}

    VideoFrame(QByteArray &&byte_array, const QSize &size, qint64 ms) : img(nullptr)
    { this->byte_array = new QByteArray(std::move(byte_array)); this->_size = new QSize(size); this->ms = ms;}

    ~VideoFrame() { clear(); }

    void clear();

    const QImage &image() const { return *img; }
    const QByteArray &byteArray() const { return *byte_array; }
    const QSize &size() const { return *_size; }

    qint64 msFromStart() const { return ms; }

    bool isEmpty() const { return img == nullptr && byte_array == nullptr; }

    void compress();
    void decompress();

    VideoFrame &operator=(const VideoFrame& src);

private:
    QImage *img;

    QByteArray *byte_array;
    QSize *_size;

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

    VideoFrame &frame(size_t index) { return frames[index]; }
    const VideoFrame &frame(size_t index) const { return frames[index]; }

    size_t size() const { return frames.size(); }

    bool load(const QString &str);
    bool save(const QString &str) const;

    Video &operator=(const Video &src) { frames = src.frames; return *this; }
    Video &operator=(Video &&src) { frames = std::move(src.frames); return *this; }

private:
    std::vector<VideoFrame> frames;
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
