#ifndef RECORDER_H
#define RECORDER_H

#include <QObject>
#include <QThread>
#include <QElapsedTimer>
#include <QEventLoop>

#include "image/image.h"
#include "encoder.h"

#include "qhotkey.h"

class RecorderThread : public QThread
{
    Q_OBJECT

public:
    void setFile(const VideoFile &video_file) { encoder.setFile(video_file); }
    void setupVideo(const QRect &rect, int frame_rate);

public slots:
    void stop();

signals:
    void flushed();

protected:
    void run() override;

private:
    VideoEncoder encoder;

    QRect screen_rect;
    int frame_rate;

    bool quit;

    QMutex mutex;
    QElapsedTimer elapsed_timer;
};

class ScreenRecorder : public QObject
{
    Q_OBJECT

public:
    ScreenRecorder(VideoFile &video);

    void exec(QRect rect, int frame_rate, QString hotkeySequence = "Ctrl+.");

private:
    RecorderThread recorder_thread;

    QHotkey hotkey;
    QEventLoop loop;
};

#endif // RECORDER_H
