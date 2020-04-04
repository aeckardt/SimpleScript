#include "recorder.h"

#include "image/screenshot.h"

void ScreenRecorder::captureFrame()
{
    captureRect(rect, video->currentFrame());
    video->encodeFrame();
}

void ScreenRecorder::exec(QRect rect, Video &video)
{
    this->rect = rect;
    this->video = &video;
    frame_rate = video.frameRate();

    if (frame_rate > 0 && frame_rate <= 30)
        interval = 1000 / frame_rate;
    else if (frame_rate <= 0)
        interval = 1000;
    else // if (frame_rate > 30)
        interval = 30;

    video.create();

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
}
