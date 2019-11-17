#include <QApplication>
#include <QElapsedTimer>
#include <QDebug>

#include "VideoView.h"

#include "../image/screenshot.h"
#include "../image/recorder.h"
#include "../image/video.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Video video;

    QElapsedTimer elapsedTimer;

    int iterations = 100;

    Recorder recorder;
    elapsedTimer.start();
    recorder.iterate({100, 100, 500, 300}, video, iterations);

    qDebug() << "Capturing took" << (elapsedTimer.elapsed() / iterations) << "ms per frame";

//    size_t i;
//    for (i = 0; i < static_cast<size_t>(iterations); ++i) {
//        qDebug() << "Frame" << i << "at" << video.frame(i).msFromStart() << "ms";
//    }

    VideoView videoView;
    videoView.showVideo(video);

    return 0;
}
