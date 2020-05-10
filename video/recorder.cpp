#include "recorder.h"

#include "image/image.h"
#include "utils/memoryusage.h"

#include <iostream>

#ifdef __APPLE__
#define _RETINA_DISPLAY_
#endif

FrameQueue::FrameQueue(int width, int height, int linesize_alignment) :
    CircularQueue<Image>(((width * 4 + linesize_alignment - 1) / linesize_alignment * linesize_alignment) * height)
{
}

RecorderThread::RecorderThread(QObject *parent)
    : QThread(parent),
      queue(nullptr)
{
}

void RecorderThread::setup(FrameQueue *queue, const QRect &rect, int frame_rate)
{
    this->queue = queue;
    screen_rect = rect;
    this->frame_rate = frame_rate;
}

void RecorderThread::run()
{
    setPriority(QThread::HighestPriority);

    int captured = 0;
    qint64 interval;
    quit = false;

    elapsed_timer.start();

    Image img(0);

    mutex.lock();
    while (!quit) {
        mutex.unlock();

        qint64 t_before_capturing = elapsed_timer.elapsed();
        fprintf(stderr, "Timer at %llums before capturing\n", elapsed_timer.elapsed());
        img.captureRect(screen_rect);

        // This loop keeps the focus in this thread
        // -> It works, but it should be improved!
        // -> Unfortunately, sleep does not guarantee to return in a specified timeframe
        //    which makes it impossible to record the screen with a certain framerate
        fprintf(stderr, "Check if queue is full...\n");
        while (queue->full()) {}

        fprintf(stderr, "Pushing image to queue...\n");
        queue->push(img);
        fprintf(stderr, "Timer at %llums after capturing (it took %llums)\n", elapsed_timer.elapsed(), elapsed_timer.elapsed() - t_before_capturing);
        captured++;

        // Determine time till next frame should be captured
        interval = captured * 1000 / frame_rate - elapsed_timer.elapsed();

        fprintf(stderr, "Calculated interval to next is: %llims\n", interval);

        // Same awful thing as above!!
        while (interval > 0) {
            interval = captured * 1000 / frame_rate - elapsed_timer.elapsed();
        }

        fprintf(stderr, "---------------------------------------------------------------------------\n");

        mutex.lock();
    }
    mutex.unlock();

    fprintf(stderr, "Recording done...\n");

    emit finished();
}

void RecorderThread::stop()
{
    QMutexLocker locker(&mutex);

    quit = true;
}

EncoderThread::EncoderThread(QObject *parent) :
    QThread(parent),
    queue(nullptr)
{
}

void EncoderThread::setup(FrameQueue *queue, const VideoFile &video_file, const QRect &rect, int frame_rate)
{
    this->queue = queue;

    int width = rect.width();
    int height = rect.height();

#ifdef _RETINA_DISPLAY_
    // With retina display, the size of the screenshot is 2x the area of the screen
    width  *= 2;
    height *= 2;
#endif

    encoder.open(video_file, width, height, frame_rate);
}

void EncoderThread::run()
{
    setPriority(QThread::HighestPriority);

    int captured = 0;
    quit = false;

    QElapsedTimer elapsed_timer;
    elapsed_timer.start();

    mutex.lock();
    while (true) {
        fprintf(stderr, "Checking if queue is empty...\n");

        while (queue->empty() && !quit) {
            mutex.unlock();
            // No problem going to sleep, even if the accuracy
            // cannot be guaranteed, because the frames are stacked
            // up in the queue
            QThread::msleep(1);
            mutex.lock();
        }

        if (queue->empty() && quit)
            break;
        mutex.unlock();

        fprintf(stderr, "Queue is not empty, has %zu elements -> popping image...\n", queue->size());

        qint64 t_before_encoding = elapsed_timer.elapsed();
        fprintf(stderr, "Timer at %llums before encoding\n", elapsed_timer.elapsed());

        queue->pop(encoder.frame());
        encoder.addFrame();
        fprintf(stderr, "Timer at %llums after encoding (it took %llums)\n", elapsed_timer.elapsed(), elapsed_timer.elapsed() - t_before_encoding);
        captured++;

        mutex.lock();
    }
    mutex.unlock();

    fprintf(stderr, "Encoding done, flushing...\n");

    encoder.finish();

    emit finished();
}

void EncoderThread::stop()
{
    QMutexLocker locker(&mutex);

    quit = true;
}

ScreenRecorder::ScreenRecorder()
{
    connect(&hotkey, &QHotkey::activated, &recorder_thread, &RecorderThread::stop);
    connect(&recorder_thread, &RecorderThread::finished, &encoder_thread, &EncoderThread::stop);
    connect(&encoder_thread, &EncoderThread::finished, &loop, &QEventLoop::quit);
}

void ScreenRecorder::exec(const VideoFile &video_file, QRect rect, int frame_rate, QString hotkeySequence)
{
    // Width / height need to be aligned by a factor of 2 for video encoding
    rect.setSize(QSize(rect.width() & 0xfffe, rect.height() & 0xfffe));

    if (rect.width() == 0 || rect.height() == 0)
        // Cannot record empty frames
        return;

    MemoryUsage usage_stats;
    usage_stats.retrieveInfo();
    int max_buffer_size = std::max(0, usage_stats.unused() - 600);

    frame_queue = new FrameQueue(rect.width(), rect.height(), 32);
    fprintf(stderr, "The frame size is: %zu bytes\n", frame_queue->frameSize());
    fprintf(stderr, "The remaining RAM size is: %d MB\n", usage_stats.unused());
    int queue_size = std::max(50, (max_buffer_size * 1024) / (static_cast<int>(frame_queue->frameSize()) / 1024));
    fprintf(stderr, "The queue size is: %d\n", queue_size);
    frame_queue->resize(queue_size);

    // Prepare recording
    hotkey.setShortcut(hotkeySequence);
    recorder_thread.setup(frame_queue, rect, frame_rate);
    encoder_thread.setup(frame_queue, video_file, rect, frame_rate);

    // Start recorder thread and register hotkey to stop thread
    recorder_thread.start();
    encoder_thread.start();
    hotkey.setRegistered(true);

    loop.exec();

    hotkey.setRegistered(false);

    recorder_thread.wait();
    encoder_thread.wait();

    delete frame_queue;
}
