#ifndef OBJECTTYPES_H
#define OBJECTTYPES_H

#include "treewalker.h"
#include "image/video.h"

class ImageObject : public tw::ParameterObject
{
public:
    ~ImageObject() override {}
    void copyTo(void *&) const override;

    QImage image;
};

class VideoObject : public tw::ParameterObject
{
public:
    ~VideoObject() override {}
    void copyTo(void *&) const override;

    Video video;
};

#endif // OBJECTTYPES_H
