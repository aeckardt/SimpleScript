#ifdef _WIN32

#include <QImage>

#include <windef.h>
#include <wingdi.h>
#include <winuser.h>

#include "screenshot.h"

bool _captureRect(const QRect &rect, QImage::Format format, char *&data, HBITMAP &hbmp, int &image_size)
{
    if (rect.width() < 1 || rect.height() < 1 ||
        (format != QImage::Format_RGB888 && format != QImage::Format_RGB32)) {
        return false;
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
    }
    else if (format == QImage::Format_RGB32) {
        bytes_per_line = rect.width() * 4;
        bih.biBitCount = 32;
    }
    else {
        return false;
    }

    bih.biSizeImage = static_cast<DWORD>(rect.height() * bytes_per_line);
    image_size = bih.biSizeImage;

    hbmp = CreateDIBSection(nullptr, reinterpret_cast<LPBITMAPINFO>(&bih), DIB_RGB_COLORS, reinterpret_cast<void **>(&data), nullptr, 0);

    HGDIOBJ hOldObj = SelectObject(hMemoryDC, static_cast<HGDIOBJ>(hbmp));
    BitBlt(hMemoryDC, 0, 0, rect.width(), rect.height(), hScreenDC, rect.x(), rect.y(), SRCCOPY | CAPTUREBLT);
    SelectObject(hMemoryDC, hOldObj);

    DeleteDC(hMemoryDC);
    ReleaseDC(nullptr, hScreenDC);

    return true;
}

QImage captureDesktop(QImage::Format format)
{
    HDC hScreenDC = GetDC(nullptr);
    int width = GetDeviceCaps(hScreenDC, HORZRES);
    int height = GetDeviceCaps(hScreenDC, VERTRES);
    ReleaseDC(nullptr, hScreenDC);

    return captureRect({0, 0, width, height}, format);
}

QImage captureRect(const QRect &rect, QImage::Format format)
{
    char *data;
    HBITMAP hbmp;
    int image_size;

    if (!_captureRect(rect, format, data, hbmp, image_size)) {
        return QImage();
    }

    return QImage(reinterpret_cast<uchar *>(data), rect.width(), rect.height(), format, [](void *hbmp) { DeleteObject(hbmp); }, hbmp);
}

#endif
