#ifdef _WIN32

#include <QImage>

#include <windef.h>
#include <wingdi.h>
#include <winuser.h>

#include "screenshot.h"

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
    int width = rect.width();
    int height = rect.height();

    if (width < 1 || height < 1 || (format != QImage::Format_RGB888 && format != QImage::Format_RGB32))
    {
        return QImage();
    }

    HDC hScreenDC = GetDC(nullptr);
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

    int bytes_per_line = 0;

    BITMAPINFOHEADER bih;
    memset(&bih, 0, sizeof(BITMAPINFOHEADER));
    bih.biWidth = width;
    bih.biHeight = -height; // force top-down
    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biPlanes = 1;

    if (format == QImage::Format_RGB888)
    {
        bytes_per_line = ((width * 3) + 3) & 0xfffffc;
        bih.biBitCount = 24;
    }
    else if (format == QImage::Format_RGB32)
    {
        bytes_per_line = width * 4;
        bih.biBitCount = 32;
    }

    bih.biSizeImage = static_cast<DWORD>(height * bytes_per_line);

    HBITMAP hbmp;
    uchar *bits;

    hbmp = CreateDIBSection(nullptr, reinterpret_cast<LPBITMAPINFO>(&bih), DIB_RGB_COLORS, reinterpret_cast<void **>(&bits), nullptr, 0);

    HGDIOBJ hOldObj = SelectObject(hMemoryDC, static_cast<HGDIOBJ>(hbmp));
    BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, rect.x(), rect.y(), SRCCOPY | CAPTUREBLT);
    SelectObject(hMemoryDC, hOldObj);

    DeleteDC(hMemoryDC);
    ReleaseDC(nullptr, hScreenDC);

    return QImage(bits, width, height, format, [](void *hbmp) { DeleteObject(hbmp); }, hbmp);
}

#endif
