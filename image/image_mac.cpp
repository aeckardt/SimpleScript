#ifdef __APPLE__

#include "image.h"

#include <ApplicationServices/ApplicationServices.h>

inline void captureFromRef(Image &dest, CGImageRef &image_ref, bool can_reallocate)
{
    int width = static_cast<int>(CGImageGetWidth(image_ref));
    int height = static_cast<int>(CGImageGetHeight(image_ref));

    CGDataProviderRef provider = CGImageGetDataProvider(image_ref);
    CFDataRef dataref = CGDataProviderCopyData(provider);

    size_t bpp = CGImageGetBitsPerPixel(image_ref) >> 3;
    size_t bpr = CGImageGetBytesPerRow(image_ref);

    if (bpp == 4) {
        if (dest.size() != QSize(width, height) && can_reallocate) {
            dest.resize(width, height);
        }

        if (dest.size() == QSize(width, height)) {
            const UInt8 *bits = CFDataGetBytePtr(dataref);

            size_t h;
            for (h = 0; h < static_cast<size_t>(height); ++h)
                memcpy(dest.scanLine(h), bits + h * bpr, static_cast<size_t>(width) * bpp);
        }
    } else if (can_reallocate)
        dest.clear();

    CFRelease(dataref);
    CGImageRelease(image_ref);
}

void Image::captureDesktop()
{
    CGDirectDisplayID main_display = CGMainDisplayID();
    CGImageRef image_ref = CGDisplayCreateImage(main_display);

    captureFromRef(*this, image_ref, can_reallocate);
}

void Image::captureRect(const QRect &rect)
{
    CGDirectDisplayID main_display = CGMainDisplayID();
    CGImageRef image_ref = CGDisplayCreateImageForRect(main_display, CGRectMake(rect.x(), rect.y(), rect.width(), rect.height()));

    captureFromRef(*this, image_ref, can_reallocate);
}

#endif
