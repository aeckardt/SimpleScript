#ifndef RECORDER_H
#define RECORDER_H

#include <QObject>
#include <QThread>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QMutex>

#include "image/image.h"
#include "utils/circularqueue.h"
#include "encoder.h"

#include <vector>

#include "qhotkey.h"

class FrameQueue : public CircularQueue<Image>
{
public:
    FrameQueue(int width, int height, int linesize_alignment);

    size_t frameSize() const { return obj_size; }
};

class RecorderThread : public QThread
{
    Q_OBJECT

public:
    RecorderThread(QObject *parent = nullptr);

    void setup(FrameQueue *queue, const QRect &rect, int frame_rate);

public slots:
    void stop();

signals:
    void finished();

protected:
    void run() override;

private:
    FrameQueue *queue;
    QRect screen_rect;
    int frame_rate;

    bool quit;

    QMutex mutex;
    QElapsedTimer elapsed_timer;
};

class EncoderThread : public QThread
{
    Q_OBJECT

public:
    EncoderThread(QObject *parent = nullptr);

    void setup(FrameQueue *queue, const VideoFile &video_file, const QRect &rect, int frame_rate);

public slots:
    void stop();

signals:
    void finished();

protected:
    void run() override;

private:
    FrameQueue *queue;
    VideoEncoder encoder;

    bool quit;

    QMutex mutex;
};

class ScreenRecorder : public QObject
{
    Q_OBJECT

public:
    ScreenRecorder();

    void exec(const VideoFile &video_file, QRect rect, int frame_rate, QString hotkeySequence = "Ctrl+.");

private:
    FrameQueue *frame_queue;
    RecorderThread recorder_thread;
    EncoderThread encoder_thread;

    QHotkey hotkey;
    QEventLoop loop;
};

#endif // RECORDER_H
