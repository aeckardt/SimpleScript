#include "image/screenshot.h"
#include "image/video.h"
#include "image/recorder.h"

#include <QApplication>
#include <QDebug>
#include <QElapsedTimer>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    Video video;

    QElapsedTimer elapsedTimer;
    elapsedTimer.start();

    int iterations = 100;

    Recorder recorder;
    recorder.iterate(video, iterations);

    qDebug() << "(with threading): Taking and compressing screenshot took" << (elapsedTimer.elapsed() / iterations) << "ms per frame";

    size_t i;
    for (i = 0; i < static_cast<size_t>(iterations); ++i) {
        if (!video.frame(i).isCompressed()) {
            qDebug() << "The frame" << i << "is not compressed!";
        }
    }

    elapsedTimer.start();

    qint64 last_time = elapsedTimer.elapsed();
    qint64 record_time = 0;
    qint64 compress_time = 0;

    video.clear();

    for (i = 0; i < static_cast<size_t>(iterations); ++i) {
        video.addFrame(VideoFrame(captureDesktop(), 0));
        record_time += elapsedTimer.elapsed() - last_time;
        last_time = elapsedTimer.elapsed();
        video.back().compress();
        compress_time += elapsedTimer.elapsed() - last_time;
        last_time = elapsedTimer.elapsed();
    }

    qDebug() << "(without threading): Taking and compressing screenshot took" << (elapsedTimer.elapsed() / iterations) << "ms per frame";
    qDebug() << "Taking a screenshot took" << (record_time / iterations) << "ms per frame";
    qDebug() << "Compressing a screenshot took" << (compress_time / iterations) << "ms per frame";

    return 0;
}
