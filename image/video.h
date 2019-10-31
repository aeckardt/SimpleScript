#ifndef VIDEO_H
#define VIDEO_H

#include <vector>

#include <QImage>

class VideoFrame
{
public:
    VideoFrame() : img(nullptr) {}
    VideoFrame(const VideoFrame &src) { img = new QImage(*src.img); }
    VideoFrame(VideoFrame &&src) { img = src.img; src.img = nullptr; }
    VideoFrame(const QImage &img) { this->img = new QImage(img); }
    VideoFrame(QImage &&img) { this->img = new QImage(std::move(img)); }
    ~VideoFrame() { if (img != nullptr) { delete img; } }

    const QImage &image() const { return *img; }

    VideoFrame &operator=(const VideoFrame& src) { img = new QImage(*src.img); return *this; }

private:
    QImage *img;
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

    bool save(const QString &fileName) const;

    Video &operator=(const Video &src) { frames = src.frames; return *this; }
    Video &operator=(Video &&src) { frames = std::move(src.frames); return *this; }

private:
    std::vector<VideoFrame> frames;
};

#endif // VIDEO_H
