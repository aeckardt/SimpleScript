#include "recorder.h"
#include "screenshot.h"

#include <QThread>

void CompressionWorker1::run()
{
    while (!recorder->finished) {

        while (recorder->next_task % 2 != 0 && !recorder->finished) {
            QThread::msleep(1);
        }

        while (recorder->last_frame == nullptr && !recorder->finished) {
            QThread::msleep(1);
        }

        recorder->mutex.lock();

        if (recorder->last_frame == nullptr && recorder->finished) {
            recorder->mutex.unlock();
            break;
        }

        recorder->worker1_frame = recorder->last_frame;
        int task = recorder->next_task;

        recorder->last_frame = nullptr;
        recorder->next_task++;

        recorder->mutex.unlock();

        recorder->worker1_frame->compress();
        while (recorder->compressed < task) {
            QThread::msleep(1);
        }

        recorder->mutex.lock();

        recorder->video->addFrame(std::move(*recorder->worker1_frame));

        delete recorder->worker1_frame;
        recorder->worker1_frame = nullptr;
        recorder->compressed++;

        recorder->mutex.unlock();
    }
}

void CompressionWorker2::run()
{
    while (!recorder->finished) {

        while (recorder->next_task % 2 != 1 && !recorder->finished) {
            QThread::msleep(1);
        }

        while (recorder->last_frame == nullptr && !recorder->finished) {
            QThread::msleep(1);
        }

        recorder->mutex.lock();

        if (recorder->last_frame == nullptr && recorder->finished) {
            recorder->mutex.unlock();
            break;
        }

        recorder->worker2_frame = recorder->last_frame;
        int task = recorder->next_task;

        recorder->last_frame = nullptr;
        recorder->next_task++;

        recorder->mutex.unlock();

        recorder->worker2_frame->compress();
        while (recorder->compressed < task) {
            QThread::msleep(1);
        }

        recorder->mutex.lock();

        recorder->video->addFrame(std::move(*recorder->worker2_frame));

        delete recorder->worker2_frame;
        recorder->worker2_frame = nullptr;
        recorder->compressed++;

        recorder->mutex.unlock();
    }
}

Recorder::Recorder()
{
    connect(&timer, &QTimer::timeout, this, &Recorder::captureFrame);
    connect(&hotkey, &QHotkey::activated, &loop, &QEventLoop::quit);

    worker1.recorder = this;
    worker2.recorder = this;
}

void Recorder::captureFrame()
{
    while (last_frame != nullptr) {
        QThread::msleep(1);
    }

    if (rect.size() == QSize(0, 0))
        last_frame = new VideoFrame(captureDesktop(), elapsed_timer.elapsed());
    else
        last_frame = new VideoFrame(captureRect(rect), elapsed_timer.elapsed());

    mutex.lock();
    captured++;
    mutex.unlock();
}

void Recorder::exec(QRect rect, Video &video, int frame_rate, const QString &hotkeySequence)
{
    this->rect = std::move(rect);
    this->video = &video;
    hotkey.setShortcut(hotkeySequence);

    if (frame_rate > 0 && frame_rate <= 30)
        interval = 1000 / frame_rate;
    else if (frame_rate <= 0)
        interval = 1000;
    else // if (frame_rate > 30)
        interval = 30;

    captured = 0;
    compressed = 0;
    next_task = 0;

    last_frame = nullptr;
    worker1_frame = nullptr;
    worker2_frame = nullptr;

    elapsed_timer.start();

    finished = false;

    worker1.start();
    worker2.start();

    captureFrame();

    hotkey.setRegistered(true);

    timer.setInterval(interval);
    timer.start();

    loop.exec();

    timer.stop();
    hotkey.setRegistered(false);

    mutex.lock();
    finished = true;
    mutex.unlock();

    worker2.wait();
    worker2.quit();

    worker1.wait();
    worker1.quit();
}

#ifdef TEST_THREADING
void Recorder::iterate(QRect rect, Video &video, int iterations)
{
    this->rect = std::move(rect);
    this->video = &video;
    this->iterations = iterations;

    captured = 0;
    compressed = 0;
    next_task = 0;

    last_frame = nullptr;
    worker1_frame = nullptr;
    worker2_frame = nullptr;

    elapsed_timer.start();

    finished = false;

    worker1.start();
    worker2.start();

    int i;
    for (i = 0; i < iterations; ++i) {
        captureFrame();
    }

    mutex.lock();
    finished = true;
    mutex.unlock();

    worker1.wait();
    worker1.quit();

    worker2.wait();
    worker2.quit();
}
#endif
