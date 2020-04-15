#ifdef _WIN32

#include "image.h"

#include <windef.h>
#include <wingdi.h>
#include <winuser.h>

void Image::captureDesktop()
{
    HDC hScreenDC = GetDC(nullptr);
    int width = GetDeviceCaps(hScreenDC, HORZRES);
    int height = GetDeviceCaps(hScreenDC, VERTRES);
    ReleaseDC(nullptr, hScreenDC);

    return captureRect({0, 0, width, height});
}

void Image::captureRect(const QRect &rect)
{
    if (rect.width() < 1 || rect.height() < 1) {
        clear();
        return;
    }

    // Assumed format is QImage::Format_RGB32
    int bpr = rect.width() * 4;

    HDC hScreenDC = GetDC(nullptr);
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

    BITMAPINFOHEADER bih;
    memset(&bih, 0, sizeof(BITMAPINFOHEADER));
    bih.biWidth = rect.width();
    bih.biHeight = -rect.height(); // force top-down
    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biPlanes = 1;
    bih.biBitCount = 32;
    bih.biSizeImage = static_cast<DWORD>(rect.height() * bpr);

    char* data;

    HBITMAP hbmp = CreateDIBSection(nullptr, reinterpret_cast<LPBITMAPINFO>(&bih), DIB_RGB_COLORS, reinterpret_cast<void **>(&data), nullptr, 0);

    HGDIOBJ hOldObj = SelectObject(hMemoryDC, static_cast<HGDIOBJ>(hbmp));
    BitBlt(hMemoryDC, 0, 0, rect.width(), rect.height(), hScreenDC, rect.x(), rect.y(), SRCCOPY | CAPTUREBLT);
    SelectObject(hMemoryDC, hOldObj);

    DeleteDC(hMemoryDC);
    ReleaseDC(nullptr, hScreenDC);

    assign(reinterpret_cast<uint8_t *>(data),
           rect.width(),
           rect.height(),
           [](void *hbmp) { DeleteObject(hbmp); },
           hbmp);
}

#endif
