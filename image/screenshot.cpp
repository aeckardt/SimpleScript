#include "screenshot.h"

#ifdef _WIN32

#include <windef.h>
#include <wingdi.h>
#include <winuser.h>

void captureDesktop(QImage &dest, QImage::Format format)
{
    HDC hScreenDC = GetDC(nullptr);
    int width = GetDeviceCaps(hScreenDC, HORZRES);
    int height = GetDeviceCaps(hScreenDC, VERTRES);
    ReleaseDC(nullptr, hScreenDC);

    return captureRect({0, 0, width, height}, dest, format);
}

void captureRect(const QRect &rect, QImage &dest, QImage::Format format)
{
    if (rect.width() < 1 || rect.height() < 1 ||
        (format != QImage::Format_RGB888 && format != QImage::Format_RGB32)) {
        dest = QImage();
        return;
    }

    HDC hScreenDC = GetDC(nullptr);
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

    int bytes_per_line = 0;

    BITMAPINFOHEADER bih;
    memset(&bih, 0, sizeof(BITMAPINFOHEADER));
    bih.biWidth = rect.width();
    bih.biHeight = -rect.height(); // force top-down
    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biPlanes = 1;

    if (format == QImage::Format_RGB888) {
        bytes_per_line = ((rect.width() * 3) + 3) & 0xfffffc;
        bih.biBitCount = 24;
    } else if (format == QImage::Format_RGB32) {
        bytes_per_line = rect.width() * 4;
        bih.biBitCount = 32;
    } else {
        dest = QImage();
        return;
    }

    bih.biSizeImage = static_cast<DWORD>(rect.height() * bytes_per_line);

    char* data;

    HBITMAP hbmp = CreateDIBSection(nullptr, reinterpret_cast<LPBITMAPINFO>(&bih), DIB_RGB_COLORS, reinterpret_cast<void **>(&data), nullptr, 0);

    HGDIOBJ hOldObj = SelectObject(hMemoryDC, static_cast<HGDIOBJ>(hbmp));
    BitBlt(hMemoryDC, 0, 0, rect.width(), rect.height(), hScreenDC, rect.x(), rect.y(), SRCCOPY | CAPTUREBLT);
    SelectObject(hMemoryDC, hOldObj);

    DeleteDC(hMemoryDC);
    ReleaseDC(nullptr, hScreenDC);

    dest = QImage(reinterpret_cast<uchar *>(data), rect.width(), rect.height(), format, [](void *hbmp) { DeleteObject(hbmp); }, hbmp);
}

#elif __APPLE__

#include <ApplicationServices/ApplicationServices.h>

inline void captureFromRef(QImage &dest, CGImageRef &image_ref, QImage::Format format)
{
    if (format != QImage::Format_RGB32) {
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
    size_t bpr = width * 4;

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
    for (h = 0; h < height; ++h)
        memcpy(dest.scanLine(static_cast<int>(h)), bits + h * bpr, width * bpp);

    CFRelease(dataref);
    CGImageRelease(image_ref);
}

void captureDesktop(QImage &dest, QImage::Format format)
{
    CGDirectDisplayID main_display = CGMainDisplayID();
    CGImageRef image_ref = CGDisplayCreateImage(main_display);

    captureFromRef(dest, image_ref, format);
}

void captureRect(const QRect &rect, QImage &dest, QImage::Format format)
{
    CGDirectDisplayID main_display = CGMainDisplayID();
    CGImageRef image_ref = CGDisplayCreateImageForRect(main_display, CGRectMake(rect.x(), rect.y(), rect.width(), rect.height()));

    captureFromRef(dest, image_ref, format);
}

#endif
