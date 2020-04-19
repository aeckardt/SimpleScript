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
        cycle_infos.push_back({Image(), nullptr, nullptr, false, false});

    if (width != 0 && height != 0)
        allocAll();
}

bool FrameCycle::isValid() const
{
    size_t index;
    for (index = 0; index < cycle_infos.size(); index++) {
        if (cycle_infos[index].has_errors ||
                cycle_infos[index].frame == nullptr ||
                cycle_infos[index].buffer == nullptr)
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
        cycle_infos[index].need_resize = true;
}

void FrameCycle::reset()
{
    size_t index;
    for (index = 0; index < FRAME_CYCLES; index++)
        shift();
}

void FrameCycle::shift()
{
    current = (current + 1) % cycle_infos.size();
    if (cycle_infos[current].need_resize) {
        cleanUp(current);
        alloc(current);
    }
}

void FrameCycle::alloc(size_t frame_index)
{
    CycleInfo &cycle = cycle_infos[frame_index];

    // Allocate an AVFrame structure
    cycle.frame = av_frame_alloc();
    if (cycle.frame == nullptr) {
        cycle.has_errors = true;
        errorMsg("Could not allocate frame");
        return;
    }

    cycle.buffer = static_cast<uint8_t*>(av_malloc(static_cast<size_t>(num_bytes)));
    if (cycle.buffer == nullptr) {
        cycle.has_errors = true;
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
    av_image_fill_arrays(cycle.frame->data, cycle.frame->linesize, cycle.buffer, AV_PIX_FMT_RGB32,
                         width, height, 1);

    cycle.image.assign(cycle.frame->data[0], width, height);

    cycle.need_resize = false;
}

void FrameCycle::allocAll()
{
    // Remark: Not sure if align needs to be 1 or 32, see
    // https://stackoverflow.com/questions/35678041/what-is-linesize-alignment-meaning
    num_bytes = av_image_get_buffer_size(AV_PIX_FMT_RGB32, width, height, 32);

    size_t index;
    for (index = 0; index < cycle_infos.size(); index++)
        alloc(index);
}

void FrameCycle::cleanUp(size_t frame_index)
{
    CycleInfo &cycle = cycle_infos[frame_index];

    // Clean up
    if (cycle.frame != nullptr)
        av_frame_free(&cycle.frame);
    if (cycle.buffer != nullptr)
        av_freep(&cycle.buffer);
    cycle.has_errors = false;
}

void FrameCycle::cleanUpAll()
{
    size_t index;
    for (index = 0; index < cycle_infos.size(); index++)
        cleanUp(index);
}

void FrameCycle::errorMsg(const char *msg)
{
    fprintf(stderr, "%s\n", msg);
}
