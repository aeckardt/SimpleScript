#ifndef RECORDER_H
#define RECORDER_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QEventLoop>

#include "video.h"
#include "external/QHotkey/qhotkey.h"

class ScreenRecorder : public QObject
{
    Q_OBJECT

public:
    ScreenRecorder();

    void exec(QRect rect, Video &video, int frame_rate);

private:
    QRect rect;
    int frame_rate;
    int interval;

    Video *video;

    QTimer timer;
    QHotkey hotkey;
    QEventLoop loop;

    void captureFrame();
};

#endif // RECORDER_H
