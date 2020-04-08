#include "recorder.h"

#include "image/screenshot.h"

ScreenRecorder::ScreenRecorder()
{
    connect(&timer, &QTimer::timeout, this, &ScreenRecorder::captureFrame);
    connect(&hotkey, &QHotkey::activated, &loop, &QEventLoop::quit);
}

void ScreenRecorder::captureFrame()
{
    // The linesize needs to be aligned with 32 bytes
    // Unfortunately that corrupts the QImage, you see it for instance,
    // when you save it as png, it becomes erroneous!
    captureRect(rect, video->nextFrame(), 32);
    video->encodeFrame();
    captured++;

    // Determine time till next frame and reconfigure interval
    qint64 interval = captured * 1000 / frame_rate - elapsed_timer.elapsed();
    if (interval < 10)
        interval = 10;
    else if (interval > 1000)
        interval = 1000;
    timer.setInterval(static_cast<int>(interval));
}

void ScreenRecorder::exec(QRect rect, Video &video, int frame_rate, QString hotkeySequence)
{
#ifndef __APPLE
    // Width / height need to be aligned by a factor of 2 for video encoding
    // -> on MacOS this step can be skipped
    //    as to why see further down this function!
    rect.setSize(QSize(rect.width() & 0xfffe, rect.height() & 0xfffe));
#endif

    if (rect.width() == 0 || rect.height() == 0)
        // Cannot record empty frames
        return;

    this->rect = rect;
    this->video = &video;
    this->frame_rate = frame_rate;

    captured = 0;

    int width = rect.width();
    int height = rect.height();

#ifdef __APPLE__
    // On MacOS, the size of the screenshot is 2x the area of the screen
    width  *= 2;
    height *= 2;
#endif

    hotkey.setShortcut(hotkeySequence);

    video.create(width, height, frame_rate);
    elapsed_timer.start();

    // Capture one frame directly at start
    captureFrame();

    hotkey.setRegistered(true);
    timer.start();

    loop.exec();

    timer.stop();
    hotkey.setRegistered(false);

    video.flush();
}
