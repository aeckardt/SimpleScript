#include "image.h"

Image::Image(int linesize_alignment) :
    bits(nullptr),
    _width(0),
    _height(0),
    linesize_alignment(linesize_alignment),
    bpr(0),
    cleanup_fnc(nullptr),
    cleanup_info(nullptr),
    can_reallocate(true)
{
}

Image::Image(const Image &src) :
    Image(src.linesize_alignment)
{
    resize(src.size());
    memcpy(bits, src.bits, bpr * static_cast<size_t>(_height));
}

Image::Image(const QString &file_name) :
    Image(0)
{
    QImage *src = new QImage(file_name);

    if (src->format() != QImage::Format_RGB32 && src->format() != QImage::Format_ARGB32) {
        // Need to have 32bit format!
        delete src;
        return;
    }

    assign(src->bits(), src->width(), src->height(), [](void *ptr) { delete static_cast<QImage *>(ptr); }, src);
}

void Image::clear()
{
    if (cleanup_fnc != nullptr) {
        cleanup_fnc(cleanup_info);
        cleanup_fnc = nullptr;
    }
    bits = nullptr;
    _width = 0;
    _height = 0;
}

size_t Image::bytesPerRow(int width)
{
    if (linesize_alignment == 0)
        return static_cast<size_t>(width * 4);
    else
        return static_cast<size_t>(((width * 4 + linesize_alignment - 1) / linesize_alignment) * linesize_alignment);
}

void Image::assign(uint8_t *bits, int width, int height, ImageCleanupFunction cleanup_fnc, void *cleanup_info)
{
    clear();

    this->bits = bits;
    this->_width = width;
    this->_height = height;

    bpr = static_cast<size_t>(bytesPerRow(width));

    this->cleanup_fnc = cleanup_fnc;
    this->cleanup_info = cleanup_info;
}



void Image::resize(int width, int height)
{
    uint8_t* buffer = new uint8_t[bytesPerRow(width) * static_cast<size_t>(height)];

    assign(buffer,
           width, height,
           [](void *ptr) { delete [] static_cast<uint8_t *>(ptr); },
           buffer);
}

QImage Image::toQImage() const
{
    if (linesize_alignment == 0)
        // Returned QImage does not have ownership over image data
        return QImage(bits, _width, _height, QImage::Format_RGB32);
    else {
        // New QImage is created that owns the image data
        QImage image = QImage(_width, _height, QImage::Format_RGB32);

        size_t line;
        for (line = 0; line < static_cast<size_t>(_height); line++)
            memcpy(image.bits() + static_cast<size_t>(image.bytesPerLine()) * line,
                   scanLine(line),
                   static_cast<size_t>(image.bytesPerLine()));

        return image;
    }
}

Image &Image::operator=(const Image &src)
{
    if (_width != src._width || _height != src._height) {
        if (can_reallocate)
            resize(src.size());
        else
            return *this;
    }

    size_t line;
    for (line = 0; line < static_cast<size_t>(_height); line++)
        memcpy(scanLine(line), src.scanLine(line), static_cast<size_t>(_width * 4));

    return *this;
}

bool Image::operator==(const Image &cmp) const
{
    if (_width != cmp._width || _height != cmp._height)
        return false;

    size_t line;
    for (line = 0; line < static_cast<size_t>(_height); line++) {
        if (memcmp(scanLine(line), cmp.scanLine(line), static_cast<size_t>(_width * 4)) != 0)
            return false;
    }

    return true;
}
