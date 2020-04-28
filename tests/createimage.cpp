#include "createimage.h"

void fillImage(Image &img, int hash)
{
    int x, y;

    for (y = 0; y < img.height(); y++) {
        for (x = 0; x < img.width(); x++) {
            *(img.scanLine(y) + (x * 4))     = (128 + y + hash * 2) & 0xff;
            *(img.scanLine(y) + (x * 4) + 1) = (64 + x + hash * 5) & 0xff;
            *(img.scanLine(y) + (x * 4) + 2) = (x + y + hash * 3) & 0xff;
            *(img.scanLine(y) + (x * 4) + 3) = 255;
        }
    }
}

