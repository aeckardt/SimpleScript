#ifndef RECORDER_H
#define RECORDER_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QElapsedTimer>
#include <QEventLoop>

#include "encoder.h"
#include "external/QHotkey/qhotkey.h"

class ScreenRecorder : public QObject
{
    Q_OBJECT

public:
    ScreenRecorder();

    void exec(QRect rect, VideoFile &video, int frame_rate, QString hotkeySequence = "Ctrl+.");

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
