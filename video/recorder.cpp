#include "recorder.h"

#include "image/screenshot.h"

void ScreenRecorder::captureFrame()
{
    captureRect(rect, video->nextFrame());
    video->encodeFrame();
}

void ScreenRecorder::exec(QRect rect, Video &video, int frame_rate)
{
    this->rect = rect;
    this->video = &video;
    this->frame_rate = frame_rate;

    if (frame_rate > 0 && frame_rate <= 30)
        interval = 1000 / frame_rate;
    else if (frame_rate <= 0)
        interval = 1000;
    else // if (frame_rate > 30)
        interval = 30;

    int width = rect.width();
    int height = rect.height();

#ifdef __APPLE__
    width  *= 2;
    height *= 2;
#endif

    video.create(width, height, frame_rate);

//    encoder.init(rect.width(), rect.height(), framerate, fileName);

//    hotkey.setShortcut(hotkeySequence);

//    startWorkers();

    // Capture one frame directly at start
    captureFrame();

//    hotkey.setRegistered(true);

//    timer.setInterval(interval);
//    timer.start();

//    timer.stop();
//    hotkey.setRegistered(false);

//    stopWorkers();

    video.flush();
}
