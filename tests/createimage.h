#ifndef CREATEIMAGE_H
#define CREATEIMAGE_H

#include "image/image.h"

void fillImage(Image &img, int hash);

inline Image createImage(int width, int height, int hash, int linesize_alignment = 0)
{
    Image img(linesize_alignment);
    img.resize(width, height);

    fillImage(img, hash);
    return img;
}

#endif // CREATEIMAGE_H
