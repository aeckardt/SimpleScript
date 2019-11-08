#include <QApplication>
#include <QElapsedTimer>
#include <QDebug>

#include "../image/screenshot.h"
#include "../image/video.h"

//#define USE_CAPTURE_RECTCOMPRESSED

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Video video;

    QElapsedTimer elapsedTimer;

    int i, iterations = 5000;

#ifdef USE_CAPTURE_RECTCOMPRESSED
    QRect rect = {0, 0, 1280, 800};
//    QRect rect = {0, 0, 1920, 1200};
#else
    qint64 ssms = 0;
    qint64 last_elapsed = 0;
#endif

    elapsedTimer.start();
    for (i = 0; i < iterations; ++i) {
#ifdef USE_CAPTURE_RECTCOMPRESSED
        video.addFrame(VideoFrame(captureRectCompressed(rect), rect.size(), iterations * 500));
#else
        video.addFrame(VideoFrame(captureDesktop(), iterations * 500));
        ssms += (elapsedTimer.elapsed() - last_elapsed);
        video.frame(i).compress();
        last_elapsed = elapsedTimer.elapsed();
//        video.frame(i).decompress();
#endif
    }

#ifndef USE_CAPTURE_RECTCOMPRESSED
    qDebug() << "Capturing took" << (ssms / iterations) << "ms per frame";
    qDebug() << "Compressing took" << ((elapsedTimer.elapsed() - ssms) / iterations) << "ms per frame";
#endif
    qDebug() << "Capturing and compressing frames took" << (static_cast<double>(elapsedTimer.elapsed()) / static_cast<double>(iterations)) << "ms per frame";

    return 0;
}
