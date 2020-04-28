#include <QApplication>
#include <QElapsedTimer>
#include <QDebug>

#include <vector>

#include "image/image.h"
#include "tests/createimage.h"
#include "utils/memoryusage.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QElapsedTimer elapsed_timer;
    elapsed_timer.start();

    MemoryUsage usage_stats;
    usage_stats.retrieveInfo();

    qDebug() << "Used memory:" << usage_stats.used() << "MB";
    qDebug() << "Unused memory:" << usage_stats.unused() << "MB";

    qint64 t = elapsed_timer.elapsed();

    qDebug() << "The memory usage request took" << t << "ms";

    int unused = usage_stats.unused();
    int unused_with_buffer = std::max(unused - 500, 0);

    int width = 1800;
    int height = 1200;
    int framerate = 15;

    int num_bytes = height * width * 4;

    int max_frames = (unused * 1000) / (num_bytes / 1000);
    int max_frames_using_buffer = (unused_with_buffer * 1000) / (num_bytes / 1000);

    qDebug() << "Frame size:" << QSize(width, height) << "-> corresponding to" << num_bytes << "Bytes";
    qDebug() << "Max frames to store:" << max_frames;
    qDebug() << "With 500 MB left as buffer:" << max_frames_using_buffer;
    qDebug() << "That would result in" << (max_frames_using_buffer / framerate) << "seconds with a framefrate of" << framerate << "fps";

    std::vector<Image> images;

    int index;
    for (index = 0; index < max_frames_using_buffer; index++)
        images.push_back(createImage(width, height, index));

    usage_stats.retrieveInfo();

    qDebug() << "Used memory after storing images:" << usage_stats.used() << "MB";
    qDebug() << "Unused memory after storing images:" << usage_stats.unused() << "MB";

    return 0;
}
