#include <QApplication>
#include <QElapsedTimer>
#include <QDebug>

#include "../image/screenshot.h"
#include "../image/video.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Video video;

    QElapsedTimer elapsedTimer;

    int i, iterations = 100;

    qint64 ssms = 0;
    qint64 last_elapsed = 0;

    elapsedTimer.start();
    for (i = 0; i < iterations; ++i) {
        video.addFrame(VideoFrame(captureDesktop(), iterations * 500));
        ssms += (elapsedTimer.elapsed() - last_elapsed);
        video.frame(i).compress();
        last_elapsed = elapsedTimer.elapsed();
//        video.frame(i).decompress();
    }

    qDebug() << "Capturing took" << (ssms / iterations) << "ms per frame";
    qDebug() << "Compressing took" << ((elapsedTimer.elapsed() - ssms) / iterations) << "ms per frame";
    qDebug() << "Capturing and compressing frames took" << (static_cast<double>(elapsedTimer.elapsed()) / static_cast<double>(iterations)) << "ms per frame";

    return 0;
}
