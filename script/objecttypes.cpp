#include "objecttypes.h"

void ImageObject::copyTo(void *&value) const
{
    value = new ImageObject();
    static_cast<ImageObject*>(value)->image = image;
}

void VideoObject::copyTo(void *&value) const
{
    value = new VideoObject();
    static_cast<VideoObject*>(value)->video = video;
}
