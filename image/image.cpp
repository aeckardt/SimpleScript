#include "image.h"

Image::Image(int linesize_alignment) :
    bits(nullptr),
    width(0),
    height(0),
    linesize_alignment(linesize_alignment),
    bpr(0),
    cleanup_fnc(nullptr),
    cleanup_info(nullptr)
{
}

Image::Image(const Image &src) :
    Image(src.linesize_alignment)
{
    resize(src.size());
    memcpy(bits, src.bits, bpr * height);
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
    width = 0;
    height = 0;
}

int Image::bytesPerRow(int width)
{
    if (linesize_alignment == 0)
        return width * 4;
    else
        return ((width * 4 + linesize_alignment - 1) / linesize_alignment) * linesize_alignment;
}

void Image::assign(uint8_t *bits, int width, int height, ImageCleanupFunction cleanup_fnc, void *cleanup_info)
{
    clear();

    this->bits = bits;
    this->width = width;
    this->height = height;

    bpr = bytesPerRow(width);

    this->cleanup_fnc = cleanup_fnc;
    this->cleanup_info = cleanup_info;
}

void Image::resize(int width, int height)
{
    assign(new uint8_t[bytesPerRow(width) * height],
           width, height,
           [](void *ptr) { delete [] static_cast<uint8_t *>(ptr); },
           bits);
}

QImage Image::toQImage() const
{
    if (linesize_alignment == 0)
        // Returned QImage does not have ownership over image data
        return QImage(bits, width, height, QImage::Format_RGB32);
    else {
        // New QImage is created that owns the image data
        QImage image = QImage(width, height, QImage::Format_RGB32);

        int line;
        for (line = 0; line < height; line++)
            memcpy(image.bits() + image.bytesPerLine() * line, scanLine(line), image.bytesPerLine());

        return image;
    }
}
