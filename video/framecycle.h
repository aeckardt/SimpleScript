#ifndef VIDEOFRAME_H
#define VIDEOFRAME_H

#include <vector>

#include "image/image.h"

class FrameCycle
{
public:
    FrameCycle() : FrameCycle(0, 0) {}
    FrameCycle(int width, int height);
    ~FrameCycle() { cleanUpAll(); }

    void resize(int width, int height);

    struct AVFrame *frame() { return frame_data[current].frame; }
    const Image &image() { return frame_data[current].image; }

    void shift();

private:
    void alloc(int frame_index);
    void allocAll();
    void cleanUp(int frame_index);
    void cleanUpAll();

    void errorMsg(const char *msg);

    int width;
    int height;
    int num_bytes;

    int current;

    struct FrameData
    {
        Image image;
        struct AVFrame *frame;
        uint8_t *buffer;
        bool need_resize;
    };

    std::vector<FrameData> frame_data;
};

#endif // VIDEOFRAME_H
