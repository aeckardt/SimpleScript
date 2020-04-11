#ifndef IMAGE_H
#define IMAGE_H

#include <QImage>
#include <QRect>
#include <QString>

typedef void (*ImageCleanupFunction)(void *);

class Image
{
public:
    Image() : Image(0) {}
    Image(int linesize_alignment);
    Image(const Image &src);
    Image(const QString &file_name);
    ~Image() { clear(); }

    void clear();

    void assign(uint8_t *bits, int width, int height,
                ImageCleanupFunction cleanup_fnc = nullptr, void *cleanup_info = nullptr);

    void resize(const QSize &new_size) { resize(new_size.width(), new_size.height()); }
    void resize(int width, int height);
    QSize size() const { return QSize(width, height); }

    void captureDesktop();
    void captureRect(const QRect &rect);

    uint8_t *scanLine(size_t line) { return bits + bpr * line; }
    const uint8_t *scanLine(size_t line) const { return bits + bpr * line; }

    uint32_t &pixelAt(int x, int y) { return *reinterpret_cast<uint32_t *>(scanLine(y) + x * 4); }
    const uint32_t &pixelAt(int x, int y) const { return *reinterpret_cast<const uint32_t *>(scanLine(y) + x * 4); }

    QImage toQImage() const;

    Image &operator=(const Image &src);

    bool operator==(const Image &cmp) const;
    bool operator!=(const Image &cmp) const { return !operator==(cmp); }

private:
    uint8_t *bits;
    int width;
    int height;
    int linesize_alignment;
    size_t bpr;

    int bytesPerRow(int width);

    ImageCleanupFunction cleanup_fnc;
    void *cleanup_info;
};

inline Image captureDesktop() { Image image; image.captureDesktop(); return image; }
inline Image captureRect(const QRect &rect) { Image image; image.captureRect(rect); return image; }

#endif // IMAGE_H
