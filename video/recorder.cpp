#include "recorder.h"

#include "image/image.h"

#ifdef __APPLE__
#define _RETINA_DISPLAY_
#endif

FrameQueue::FrameQueue(int width, int height, int linesize_alignment) :
    width(width),
    height(height),
    linesize_alignment(linesize_alignment),
    total_size(0),
    limit(0)
{
    if (linesize_alignment == 0)
        bpr = static_cast<size_t>(this->width * 4);
    else
        bpr = static_cast<size_t>(((this->width * 4 + this->linesize_alignment - 1) / this->linesize_alignment) * this->linesize_alignment);
    num_bytes = this->height * bpr;
    max_queue_size = INT32_MAX / num_bytes;
}

void FrameQueue::push(Image &img)
{
    QMutexLocker locker(&mutex);

    if (total_size == limit)
        return;

    images.push(std::move(img));
    total_size++;
}

void FrameQueue::pop(Image &img)
{
    QMutexLocker locker(&mutex);

    if (total_size == 0)
        return;

    img = std::move(images.front());
    images.pop();
    total_size--;
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

        fprintf(stderr, "Calculated interval to next is: %llums\n", interval);

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
    while (!quit) {
        mutex.unlock();

        // This loop keeps the focus in this thread
        // -> It works, but it should be improved!
        // -> Unfortunately, sleep does not guarantee to return in a specified timeframe
        //    which makes it impossible to record the screen with a certain framerate
        mutex.lock();
        fprintf(stderr, "Checking if queue empty...\n");
        while (queue->empty() && !quit)
        {
            mutex.unlock();
            mutex.lock();
        }

        if (queue->empty() && quit) {
            break;
        }
        mutex.unlock();

        fprintf(stderr, "Queue is not empty, has %d elements -> popping image...\n", queue->size());

        qint64 t_before_encoding = elapsed_timer.elapsed();
        fprintf(stderr, "Timer at %llums before encoding\n", elapsed_timer.elapsed());

        queue->pop(encoder.frame());
        encoder.writeFrame();
        fprintf(stderr, "Timer at %llums after encoding (it took %llums)\n", elapsed_timer.elapsed(), elapsed_timer.elapsed() - t_before_encoding);
        captured++;

        mutex.lock();
    }
    mutex.unlock();

    fprintf(stderr, "Encoding done, flushing...\n");

    encoder.flush();

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
#ifndef __APPLE
    // Width / height need to be aligned by a factor of 2 for video encoding
    // -> on MacOS this step can be skipped
    //    as to why look into RecorderThread::setupVideo!
    rect.setSize(QSize(rect.width() & 0xfffe, rect.height() & 0xfffe));
#endif

    if (rect.width() == 0 || rect.height() == 0)
        // Cannot record empty frames
        return;

    frame_queue = new FrameQueue(rect.width(), rect.height(), 32);
    frame_queue->setLimit(100);

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
