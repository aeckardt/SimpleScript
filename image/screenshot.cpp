#include <ApplicationServices/ApplicationServices.h>

#include "screenshot.h"

QImage captureDesktop()
{
    CGDirectDisplayID main_display = CGMainDisplayID();

    CGImageRef image_ref = CGDisplayCreateImage(main_display);
    CGDataProviderRef provider = CGImageGetDataProvider(image_ref);
    CFDataRef dataref = CGDataProviderCopyData(provider);

    size_t width  = CGImageGetWidth(image_ref);
    size_t height = CGImageGetHeight(image_ref);

    if (width == 0 || height == 0)
        return QImage();

    size_t bpp = CGImageGetBitsPerPixel(image_ref) >> 3;
    size_t bpr = CGImageGetBytesPerRow(image_ref);

    QImage::Format format;

    if (bpp == 4)
        format = QImage::Format_RGB32;
    else
    {
        CFRelease(dataref);
        CGImageRelease(image_ref);
        return QImage();
    }

    QImage img(static_cast<int>(width), static_cast<int>(height), format);

    const UInt8 *bits = CFDataGetBytePtr(dataref);

    size_t h;
    for (h = 0; h < height; ++h)
        memcpy(img.scanLine(static_cast<int>(h)), bits + h * bpr, width * bpp);

    CFRelease(dataref);
    CGImageRelease(image_ref);

    return img;
}

QImage captureRect(const QRect &rect)
{
    CGDirectDisplayID main_display = CGMainDisplayID();
    CGImageRef image_ref = CGDisplayCreateImageForRect(main_display, CGRectMake(rect.x(), rect.y(), rect.width(), rect.height()));

    size_t width = CGImageGetWidth(image_ref);
    size_t height = CGImageGetHeight(image_ref);

    if (width == 0 || height == 0)
        return QImage();

    CGDataProviderRef provider = CGImageGetDataProvider(image_ref);
    CFDataRef dataref = CGDataProviderCopyData(provider);

    size_t bpp = CGImageGetBitsPerPixel(image_ref) >> 3;
    size_t bpr = CGImageGetBytesPerRow(image_ref);

    QImage::Format format;

    if (bpp == 4)
        format = QImage::Format_RGB32;
    else
    {
        CFRelease(dataref);
        CGImageRelease(image_ref);
        return QImage();
    }

    QImage img(static_cast<int>(width), static_cast<int>(height), format);

    const UInt8 *bits = CFDataGetBytePtr(dataref);

    size_t h;
    for (h = 0; h < height; ++h)
        memcpy(img.scanLine(static_cast<int>(h)), bits + h * bpr, width * bpp);

    CFRelease(dataref);
    CGImageRelease(image_ref);

    return img;
}
