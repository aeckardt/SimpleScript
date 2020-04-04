#ifndef RECORDER_H
#define RECORDER_H

#include "encoder.h"

#include <QObject>
#include <QThread>

class ScreenRecorder
{
public:
    ScreenRecorder();

    void exec(QRect rect, int framerate);

private:
    QRect rect;
    int framerate;
    int interval;

    VideoEncoder encoder;

    void captureFrame();
};

#endif // RECORDER_H
