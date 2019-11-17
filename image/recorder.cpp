#include "recorder.h"
#include "screenshot.h"

#include <QThread>

void CompressionWorker::run()
{
    while (!recorder->finished) {

        while (recorder->next_task % COMPRESSION_WORKERS != _n && !recorder->finished) {
            QThread::usleep(100);
        }

        while (recorder->last_frame == nullptr && !recorder->finished) {
            QThread::usleep(100);
        }

        recorder->mutex.lock();

        if (recorder->last_frame == nullptr && recorder->finished) {
            recorder->mutex.unlock();
            break;
        }

        // Consume next frame
        recorder->worker_frame[_n] = recorder->last_frame;
        int task = recorder->next_task;

        recorder->last_frame = nullptr;
        recorder->next_task++;

        recorder->mutex.unlock();

        // Compress frame
        recorder->worker_frame[_n]->compress();
        while (recorder->compressed < task) {
            QThread::usleep(100);
        }

        recorder->mutex.lock();

        recorder->video->addFrame(std::move(*recorder->worker_frame[_n]));

        delete recorder->worker_frame[_n];
        recorder->worker_frame[_n] = nullptr;
        recorder->compressed++;

        recorder->mutex.unlock();
    }
}

Recorder::Recorder()
{
    connect(&timer, &QTimer::timeout, this, &Recorder::captureFrame);
    connect(&hotkey, &QHotkey::activated, &loop, &QEventLoop::quit);

    int i;
    for (i = 0; i < COMPRESSION_WORKERS; ++i) {
        worker[i].recorder = this;
        worker[i]._n = i;
    }
}

void Recorder::startWorkers()
{
    // Except for elapsed_timer
    // all the variables are used for
    // the compression worker(s)
    captured = 0;
    compressed = 0;
    next_task = 0;

    int i;

    last_frame = nullptr;
    for (i = 0; i < COMPRESSION_WORKERS; ++i) {
        worker_frame[i] = nullptr;
    }

    elapsed_timer.start();

    finished = false;

    for (i = 0; i < COMPRESSION_WORKERS; ++i) {
        worker[i].start();
    }
}

void Recorder::stopWorkers()
{
    mutex.lock();
    finished = true;
    mutex.unlock();

    int i;
    for (i = COMPRESSION_WORKERS - 1; i >= 0; --i) {
        worker[i].wait();
        worker[i].quit();
    }
}

void Recorder::captureFrame()
{
    while (last_frame != nullptr) {
        QThread::usleep(100);
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

    startWorkers();

    // Capture one frame directly at start
    captureFrame();

    hotkey.setRegistered(true);

    timer.setInterval(interval);
    timer.start();

    loop.exec();

    timer.stop();
    hotkey.setRegistered(false);

    stopWorkers();
}

#ifdef TEST_THREADING
void Recorder::iterate(QRect rect, Video &video, int iterations)
{
    this->rect = std::move(rect);
    this->video = &video;

    startWorkers();

    int i;
    for (i = 0; i < iterations; ++i) {
        captureFrame();
    }

    stopWorkers();
}
#endif
