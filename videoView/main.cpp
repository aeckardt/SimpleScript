#include <QApplication>
#include <QElapsedTimer>
#include <QDebug>

#include "VideoView.h"

#include "../image/screenshot.h"
#include "../image/video.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Video video;

    QElapsedTimer elapsedTimer;

    int i, iterations = 100;

    QRect rect = {0, 0, 1280, 800};

    elapsedTimer.start();
    for (i = 0; i < iterations; ++i) {
        video.addFrame(VideoFrame(captureDesktop(), 0));
        video.frame(i).compress();
        video.frame(i).decompress();
//        video.addFrame(VideoFrame(captureRectCompressed(rect), rect.size(), iterations * 500));
    }

    qDebug() << "Capturing and compressing frames took" << (elapsedTimer.elapsed() / iterations) << "ms";


    return 0;
}
