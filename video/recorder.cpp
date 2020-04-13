#include "recorder.h"

#include "image/image.h"

void RecorderThread::setupVideo(const VideoFile &video_file, const QRect &rect, int frame_rate)
{
    screen_rect = rect;
    this->frame_rate = frame_rate;

    int width = rect.width();
    int height = rect.height();

#ifdef __APPLE__
    // On MacOS, the size of the screenshot is 2x the area of the screen
    width  *= 2;
    height *= 2;
#endif

    encoder.open(video_file, width, height, frame_rate);
}

void RecorderThread::run()
{
    setPriority(QThread::HighestPriority);

    int captured = 0;
    qint64 interval;
    quit = false;

    elapsed_timer.start();

    mutex.lock();
    while (!quit) {
        mutex.unlock();

        encoder.frame().captureRect(screen_rect);
        encoder.writeFrame();
        captured++;

        // Determine time till next frame should be captured
        interval = captured * 1000 / frame_rate - elapsed_timer.elapsed();

        // This loop keeps the focus in this thread
        // -> It works, but it should be improved!
        // -> Unfortunately, sleep does not guarantee to return in a specified timeframe
        //    which makes it impossible to record the screen with a certain framerate
        while (interval > 0) {
            interval = captured * 1000 / frame_rate - elapsed_timer.elapsed();
        }

        mutex.lock();
    }
    mutex.unlock();

    encoder.flush();
}

void RecorderThread::stop()
{
    QMutexLocker locker(&mutex);

    quit = true;
}

ScreenRecorder::ScreenRecorder()
{
    connect(&hotkey, &QHotkey::activated, &recorder_thread, &RecorderThread::stop);
    connect(&recorder_thread, &RecorderThread::finished, &loop, &QEventLoop::quit);
}

void ScreenRecorder::exec(const VideoFile &video_file, QRect rect, int frame_rate, QString hotkeySequence)
{
#ifndef __APPLE
    // Width / height need to be aligned by a factor of 2 for video encoding
    // -> on MacOS this step can be skipped
    //    as to why look into RecorderThread::setupVideo!
    rect.setSize(QSize(rect.width() & 0xfffe, rect.height() & 0xfffe));
#endif

    if (rect.width() == 0 || rect.height() == 0)
        // Cannot record empty frames
        return;

    // Prepare recording
    hotkey.setShortcut(hotkeySequence);
    recorder_thread.setupVideo(video_file, rect, frame_rate);

    // Start recorder thread and register hotkey to stop thread
    recorder_thread.start();
    hotkey.setRegistered(true);

    loop.exec();

    hotkey.setRegistered(false);
}
