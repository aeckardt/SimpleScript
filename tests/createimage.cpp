#include "createimage.h"

Image createImage(int width, int height, int hash, int linesize_alignment)
{
    Image img(linesize_alignment);
    img.resize(width, height);

    int x, y;

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            *(img.scanLine(y) + (x * 4))     = (128 + y + hash * 2) & 0xff;
            *(img.scanLine(y) + (x * 4) + 1) = (64 + x + hash * 5) & 0xff;
            *(img.scanLine(y) + (x * 4) + 2) = (x + y + hash * 3) & 0xff;
            *(img.scanLine(y) + (x * 4) + 3) = 255;
        }
    }

    return img;
}

