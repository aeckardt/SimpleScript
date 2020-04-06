#ifdef __APPLE__

#include "screenshot.h"

#include <ApplicationServices/ApplicationServices.h>

inline void captureFromRef(QImage &dest, CGImageRef &image_ref, int linesize_align, QImage::Format format)
{
    if (format != QImage::Format_RGB32) {
        // Can only capture in one format with the following routines
        // In case another format is needed, convert later!
        dest = QImage();
        return;
    }

    size_t width = CGImageGetWidth(image_ref);
    size_t height = CGImageGetHeight(image_ref);

    if (width == 0 || height == 0) {
        dest = QImage();
        return;
    }

    CGDataProviderRef provider = CGImageGetDataProvider(image_ref);
    CFDataRef dataref = CGDataProviderCopyData(provider);

    size_t bpp = CGImageGetBitsPerPixel(image_ref) >> 3;
    size_t bpr = CGImageGetBytesPerRow(image_ref);

    if (bpp != 4) {
        CFRelease(dataref);
        CGImageRelease(image_ref);
        dest = QImage();
        return;
    }

    if (dest.size() != QSize(static_cast<int>(width), static_cast<int>(height)))
        dest = QImage(static_cast<int>(width), static_cast<int>(height), format);

    const UInt8 *bits = CFDataGetBytePtr(dataref);

    size_t h;
    if (linesize_align == 0) {
        for (h = 0; h < height; ++h)
            memcpy(dest.scanLine(static_cast<int>(h)), bits + h * bpr, width * bpp);
    } else {
        size_t dest_bpr = ((width * bpp - 1) / linesize_align) * linesize_align + linesize_align;
        for (h = 0; h < height; ++h)
            memcpy(dest.bits() + h * dest_bpr, bits + h * bpr, width * bpp);
    }

    CFRelease(dataref);
    CGImageRelease(image_ref);
}

void captureDesktop(QImage &dest, int linesize_align, QImage::Format format)
{
    CGDirectDisplayID main_display = CGMainDisplayID();
    CGImageRef image_ref = CGDisplayCreateImage(main_display);

    captureFromRef(dest, image_ref, linesize_align, format);
}

void captureRect(const QRect &rect, QImage &dest, int linesize_align, QImage::Format format)
{
    CGDirectDisplayID main_display = CGMainDisplayID();
    CGImageRef image_ref = CGDisplayCreateImageForRect(main_display, CGRectMake(rect.x(), rect.y(), rect.width(), rect.height()));

    captureFromRef(dest, image_ref, linesize_align, format);
}

#endif
