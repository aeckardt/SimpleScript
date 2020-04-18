#ifdef _WIN32

#include "image.h"

#include <windef.h>
#include <wingdi.h>
#include <winuser.h>

void Image::captureDesktop()
{
    HDC hScreenDC = GetDC(nullptr);
    int screen_width = GetDeviceCaps(hScreenDC, HORZRES);
    int screen_height = GetDeviceCaps(hScreenDC, VERTRES);
    ReleaseDC(nullptr, hScreenDC);

    return captureRect({0, 0, screen_width, screen_height});
}

void Image::captureRect(const QRect &rect)
{
    if (rect.width() < 1 || rect.height() < 1) {
        clear();
        return;
    }

    // Assumed format is QImage::Format_RGB32
    size_t bpr = static_cast<size_t>(rect.width() * 4);

    HDC hScreenDC = GetDC(nullptr);
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

    BITMAPINFOHEADER bih;
    memset(&bih, 0, sizeof(BITMAPINFOHEADER));
    bih.biWidth = rect.width();
    bih.biHeight = -rect.height(); // force top-down
    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biPlanes = 1;
    bih.biBitCount = 32;
    bih.biSizeImage = static_cast<DWORD>(static_cast<size_t>(rect.height()) * bpr);

    uint8_t* bits;

    HBITMAP hbmp = CreateDIBSection(nullptr, reinterpret_cast<LPBITMAPINFO>(&bih), DIB_RGB_COLORS, reinterpret_cast<void **>(&bits), nullptr, 0);

    HGDIOBJ hOldObj = SelectObject(hMemoryDC, static_cast<HGDIOBJ>(hbmp));
    BitBlt(hMemoryDC, 0, 0, rect.width(), rect.height(), hScreenDC, rect.x(), rect.y(), SRCCOPY | CAPTUREBLT);
    SelectObject(hMemoryDC, hOldObj);

    DeleteDC(hMemoryDC);
    ReleaseDC(nullptr, hScreenDC);

    if (can_reallocate)
        assign(reinterpret_cast<uint8_t *>(bits),
               rect.width(),
               rect.height(),
               [](void *hbmp) { DeleteObject(hbmp); },
               hbmp);
    else {
        if (rect.size() == size()) {
            size_t h;
            for (h = 0; h < static_cast<size_t>(_height); ++h)
                memcpy(scanLine(h), bits + h * bpr, static_cast<size_t>(_width) * 4);
        }
        DeleteObject(hbmp);
    }
}

#endif
