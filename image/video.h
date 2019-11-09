#ifndef VIDEO_H
#define VIDEO_H

#include <vector>

#include <QImage>
#include <QByteArray>
#include <QString>

class VideoFrame
{
public:
    struct CompressedData
    {
    public:
        inline CompressedData() {}

        inline CompressedData(const CompressedData &src)
        { byte_array = src.byte_array; size = src.size; }

        inline CompressedData(CompressedData &&src)
        { byte_array = std::move(src.byte_array); size = std::move(src.size); }

        QByteArray byte_array;
        QSize size;
    };

    VideoFrame() : img(nullptr), compressed_data(nullptr), ms(0) {}
    VideoFrame(const VideoFrame &src);
    VideoFrame(VideoFrame &&src);

    VideoFrame(const QImage &img, qint64 ms) : compressed_data(nullptr)
    { this->img = new QImage(img); this->ms = ms; }

    VideoFrame(QImage &&img, qint64 ms) : compressed_data(nullptr)
    { this->img = new QImage(std::move(img)); this->ms = ms; }

    VideoFrame(const CompressedData &_cmp, qint64 ms) : img(nullptr)
    { this->compressed_data = new CompressedData(_cmp); this->ms = ms; }

    VideoFrame(CompressedData &&_cmp, qint64 ms) : img(nullptr)
    { this->compressed_data = new CompressedData(std::move(_cmp)); this->ms = ms;}

    inline ~VideoFrame() { clear(); }

    void clear();

    inline const QImage &image() const { return *img; }
    inline const CompressedData &compressedData() const { return *compressed_data; }

    inline qint64 msFromStart() const { return ms; }

    inline bool isEmpty() const { return img == nullptr && compressed_data == nullptr; }

    void compress();
    void decompress();

    inline bool isCompressed() const { return img == nullptr && compressed_data != nullptr; }

    VideoFrame &operator=(const VideoFrame& src);

private:
    QImage *img;
    CompressedData *compressed_data;

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

    inline VideoFrame &back() { return frames.back(); }
    inline size_t size() const { return frames.size(); }
    inline void clear() { frames.clear(); }

    bool load(const QString &str);
    bool save(const QString &str) const;

    Video &operator=(const Video &src) { frames = src.frames; return *this; }
    Video &operator=(Video &&src) { frames = std::move(src.frames); return *this; }

private:
    std::vector<VideoFrame> frames;
};

#endif // VIDEO_H
