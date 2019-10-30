#ifndef VIDEO_H
#define VIDEO_H

#include <vector>

#include <QImage>

class VideoFrame
{
public:
    VideoFrame(const QImage &&img) { image = img; }

private:
    QImage image;
};

class Video
{
public:
    ~Video() { clear(); }

    // addFrame is binding the frame to this class
    void addFrame(VideoFrame *frame) { frames.push_back(frame); }
    void clear();

    const VideoFrame &frame(size_t index) const { return *frames[index]; }

    bool save(const QString &fileName) const;

private:
    std::vector<VideoFrame *> frames;
};

#endif // VIDEO_H
