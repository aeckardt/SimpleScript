#include "framecycle.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavutil/imgutils.h"
}

#define FRAME_CYCLES 2

FrameCycle::FrameCycle(int width, int height) :
    width(width),
    height(height),
    current(0)
{
    int index;
    for (index = 0; index < FRAME_CYCLES; index++)
        frame_data.push_back({Image(), nullptr, nullptr, false, false});

    if (width != 0 && height != 0)
        allocAll();
}

bool FrameCycle::isValid() const
{
    size_t index;
    for (index = 0; index < frame_data.size(); index++) {
        if (frame_data[index].has_errors ||
                frame_data[index].frame == nullptr ||
                frame_data[index].buffer == nullptr)
            return false;
    }
    return true;
}

void FrameCycle::resize(int width, int height)
{
    this->width = width;
    this->height = height;

    num_bytes = av_image_get_buffer_size(AV_PIX_FMT_RGB32, width, height, 32);

    size_t index;
    for (index = 0; index < FRAME_CYCLES; index++)
        frame_data[index].need_resize = true;
}

void FrameCycle::reset()
{
    size_t index;
    for (index = 0; index < FRAME_CYCLES; index++)
        shift();
}

void FrameCycle::shift()
{
    current = (current + 1) % frame_data.size();
    if (frame_data[current].need_resize) {
        cleanUp(current);
        alloc(current);
    }
}

void FrameCycle::alloc(size_t frame_index)
{
    FrameData &frame = frame_data[frame_index];

    // Allocate an AVFrame structure
    frame.frame = av_frame_alloc();
    if (frame.frame == nullptr) {
        frame.has_errors = true;
        errorMsg("Could not allocate frame");
        return;
    }

    frame.buffer = static_cast<uint8_t*>(av_malloc(static_cast<size_t>(num_bytes)));
    if (frame.buffer == nullptr) {
        frame.has_errors = true;
        errorMsg("Could not allocate frame buffer");
        return;
    }

    // Assign appropriate parts of buffer to image planes in frame_rgb
    // Note that frame_rgb is an AVFrame, but AVFrame is a superset
    // of AVPicture

    // Replace avpicture_fill -> deprecated
    // with av_image_fill_arrays

    // Examples of av_image_fill_arrays from
    // https://mail.gnome.org/archives/commits-list/2016-February/msg05531.html
    // https://github.com/bernhardu/dvbcut-deb/blob/master/src/avframe.cpp
    av_image_fill_arrays(frame.frame->data, frame.frame->linesize, frame.buffer, AV_PIX_FMT_RGB32,
                         width, height, 1);

    frame.image.assign(frame.frame->data[0], width, height);

    frame.need_resize = false;
}

void FrameCycle::allocAll()
{
    // Remark: Not sure if align needs to be 1 or 32, see
    // https://stackoverflow.com/questions/35678041/what-is-linesize-alignment-meaning
    num_bytes = av_image_get_buffer_size(AV_PIX_FMT_RGB32, width, height, 32);

    size_t index;
    for (index = 0; index < frame_data.size(); index++)
        alloc(index);
}

void FrameCycle::cleanUp(size_t frame_index)
{
    FrameData &frame = frame_data[frame_index];

    // Clean up
    if (frame.frame != nullptr)
        av_frame_free(&frame.frame);
    if (frame.buffer != nullptr)
        av_freep(&frame.buffer);
    frame.has_errors = false;
}

void FrameCycle::cleanUpAll()
{
    size_t index;
    for (index = 0; index < frame_data.size(); index++)
        cleanUp(index);
}

void FrameCycle::errorMsg(const char *msg)
{
    fprintf(stderr, "%s\n", msg);
}
