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

    elapsedTimer.start();

    int i, iterations = 100;

    QRect rect = {0, 0, 1280, 800};
    for (i = 0; i < iterations; ++i) {
        video.addFrame(VideoFrame(captureRect(rect), i * 500));
    }

    qint64 bytesize = video.frame(0).image().width() * video.frame(0).image().height() * 4;

    qDebug() << "Capturing frames took" << (elapsedTimer.elapsed() / iterations) << "ms with" << bytesize << "bytes per frame";

    elapsedTimer.start();
    video.save("/Users/albrecht/Documents/Code/GameScript/output/videofile");

    qDebug() << "Saving frames took" << (elapsedTimer.elapsed() / iterations) << "ms per frame";

    elapsedTimer.start();
    video.load("/Users/albrecht/Documents/Code/GameScript/output/videofile");

    qDebug() << "Loading" << video.size() << "frames took" << (elapsedTimer.elapsed() / iterations) << "ms with" << video.png_size << "bytes per frame";

    elapsedTimer.start();
    video.compress();

    qDebug() << "Compressing frames took" << (elapsedTimer.elapsed() / iterations) << "ms with" << video.compressed_size << "bytes per frame";

    QByteArray byte_array;

    elapsedTimer.start();
    for (i = 0; i < iterations; ++i) {
        byte_array.clear();
        byte_array = captureCompressed(rect);
    }

    qDebug() << "Capturing and compressing frames took" << (elapsedTimer.elapsed() / iterations) << "ms with" << byte_array.size() << "bytes per frame";

    return 0;
}
