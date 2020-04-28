#ifndef IMAGE_H
#define IMAGE_H

#include <QObject>
#include <QImage>
#include <QRect>
#include <QString>

typedef void (*ImageCleanupFunction)(void *);

class Image : public QObject
{
    Q_OBJECT

public:
    Image() : Image(0) {}
    Image(int linesize_alignment);
    Image(const Image &src);
    Image(Image &&src);
    Image(const QString &file_name);
    ~Image() { clear(); }

    void clear();

    void assign(uint8_t *_bits, int _width, int _height,
                ImageCleanupFunction cleanup_fnc = nullptr, void *cleanup_info = nullptr);

    void resize(const QSize &new_size) { resize(new_size.width(), new_size.height()); }
    void resize(int _width, int _height);
    int width() const { return _width; }
    int height() const { return _height; }
    QSize size() const { return QSize(_width, _height); }

    void captureDesktop();
    void captureRect(const QRect &rect);

    uint8_t *scanLine(size_t line) { return _bits + bpr * line; }
    const uint8_t *scanLine(size_t line) const { return _bits + bpr * line; }
    const uint8_t *bits() const { return _bits; }

    QImage toQImage() const;

    Image &operator=(const Image &src);
    Image &operator=(Image &&src);

    bool operator==(const Image &cmp) const;
    bool operator!=(const Image &cmp) const { return !operator==(cmp); }

signals:
    void reallocate(uint8_t *bits);

private:
    uint8_t *_bits;
    int _width;
    int _height;
    int linesize_alignment;
    size_t bpr;

    size_t bytesPerRow(int _width);

    ImageCleanupFunction cleanup_fnc;
    void *cleanup_info;
};

#endif // IMAGE_H
