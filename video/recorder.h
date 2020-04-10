#ifndef RECORDER_H
#define RECORDER_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QElapsedTimer>
#include <QEventLoop>

#include "image/image.h"
#include "encoder.h"

#include "qhotkey.h"

class ScreenRecorder : public QObject
{
    Q_OBJECT

public:
    ScreenRecorder(VideoFile &video);

    void exec(QRect rect, int frame_rate, QString hotkeySequence = "Ctrl+.");

private:
    QRect rect;
    int frame_rate;
    int captured;

    VideoEncoder encoder;

    QTimer timer;
    QElapsedTimer elapsed_timer;
    QHotkey hotkey;
    QEventLoop loop;

    void captureFrame();
};

#endif // RECORDER_H
