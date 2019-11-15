#ifndef RECORD_H
#define RECORD_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QMutex>

#include "hotkey/qhotkey.h"
#include "image/video.h"

class Recorder;

class CompressionWorker : public QThread
{
    Q_OBJECT
    void run() override;

public:
    Recorder *recorder;
    int _n;

    char padding[4]; // silences compiler warning
};

#define COMPRESSION_WORKERS 2

class Recorder : public QObject
{
    Q_OBJECT
    CompressionWorker worker[COMPRESSION_WORKERS];
    QMutex mutex;

public:
    Recorder();

    void exec(QRect rect, Video &video, int frame_rate, const QString &hotkeySequence = "Ctrl+.");
    inline void exec(Video &video, int frame_rate, const QString &hotkeySequence = "Ctrl+.")
    { exec(QRect(), video, frame_rate, hotkeySequence); }

#ifdef TEST_THREADING
    void iterate(QRect rect, Video &video, int iterations);
    inline void iterate(Video &video, int iterations)
    { iterate(QRect(), video, iterations); }
#endif

public slots:
    void captureFrame();

private:
    int captured;
    int compressed;

#ifdef TEST_THREADING
    int iterations;
#endif
    int frame_rate;
    int interval;

    int next_task;

    bool finished;

    VideoFrame *last_frame;
    VideoFrame *worker_frame[COMPRESSION_WORKERS];
    Video *video;

    QRect rect;

    QEventLoop loop;
    QTimer timer;
    QHotkey hotkey;
    QElapsedTimer elapsed_timer;

    friend CompressionWorker;
};

#endif // RECORDER_H
