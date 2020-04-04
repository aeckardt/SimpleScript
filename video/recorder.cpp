#include "recorder.h"

#include "image/screenshot.h"

void ScreenRecorder::captureFrame()
{
    captureRect(rect, encoder.nextFrame());
}

void ScreenRecorder::exec(QRect rect, int framerate)
{
    this->rect = rect;
    this->framerate = framerate;

    if (framerate > 0 && framerate <= 30)
        interval = 1000 / framerate;
    else if (framerate <= 0)
        interval = 1000;
    else // if (frame_rate > 30)
        interval = 30;

//    encoder.init(rect.width(), rect.height(), framerate, fileName);

//    hotkey.setShortcut(hotkeySequence);

//    startWorkers();

    // Capture one frame directly at start
//    captureFrame();

//    hotkey.setRegistered(true);

//    timer.setInterval(interval);
//    timer.start();

//    timer.stop();
//    hotkey.setRegistered(false);

//    stopWorkers();
}
