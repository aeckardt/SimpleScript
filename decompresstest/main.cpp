#include <QApplication>
#include <QElapsedTimer>

#include <QDebug>

#include "image/screenshot.h"
#include "image/video.h"

#define CAPTURE_RECT

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    int i, iterations = 100;

#ifdef CAPTURE_RECT
    QRect rect = {100, 100, 500, 300};
#endif

    QElapsedTimer elapsedTimer;
    elapsedTimer.start();

    qint64 last_time, comp = 0, decomp = 0;

    Video video;

    for (i = 0; i < iterations; ++i) {
#ifdef CAPTURE_RECT
        video.addFrame(VideoFrame(captureRect(rect), 0));
#else
        video.addFrame(VideoFrame(captureDesktop(), 0));
#endif
        last_time = elapsedTimer.elapsed();
        video.back().compress();
        comp += elapsedTimer.elapsed() - last_time;
    }

    for (i = 0; i < iterations; ++i) {
        last_time = elapsedTimer.elapsed();
        video.frame(static_cast<size_t>(i)).decompress();
        decomp += elapsedTimer.elapsed() - last_time;
    }

    qDebug() << "Compressing a frame took" << (comp / iterations) << "ms per frame";
    qDebug() << "Decompressing a frame took" << (decomp / iterations) << "ms per frame";

    return 0;
}
