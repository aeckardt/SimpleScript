#ifndef RECORDER_H
#define RECORDER_H

#include "video.h"

#include <QObject>
#include <QThread>

class ScreenRecorder
{
public:
    void exec(QRect rect, Video &video, int frame_rate);

private:
    QRect rect;
    int frame_rate;
    int interval;

    Video *video;

    void captureFrame();
};

#endif // RECORDER_H
