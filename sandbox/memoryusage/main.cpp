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
    qDebug() << "Total memory:" << usage_stats.total() << "MB";

    qint64 t = elapsed_timer.elapsed();

    qDebug() << "The memory usage request took" << t << "ms";

    int memory_buffer;
    if (usage_stats.total() < 8000)
        memory_buffer = 500; // MB
    else
        memory_buffer = 700;

    int unused = usage_stats.unused();
    int unused_with_buffer = std::max(unused - memory_buffer, 0);
    int unused_with_buffer_cap = std::min(4096, std::max(unused - memory_buffer, 0));
    int unused_with_buffer_rest = unused_with_buffer - unused_with_buffer_cap;

    int width = 1800;
    int height = 1200;
    int framerate = 15;

    int num_bytes = height * width * 4;

    int max_frames = (unused * 1024) / (num_bytes / 1024);
    int max_frames_using_buffer = (unused_with_buffer * 1024) / (num_bytes / 1024);
    int max_frames_using_buffer_cap = (unused_with_buffer_cap * 1024) / (num_bytes / 1024);
    int max_frames_using_buffer_rest = (unused_with_buffer_rest * 1024) / (num_bytes / 1024);

    qDebug() << "Frame size:" << QSize(width, height) << "-> corresponding to" << num_bytes << "Bytes";
    qDebug() << "Max frames to store:" << max_frames;
    qDebug() << "With" << memory_buffer << "MB left as buffer:" << max_frames_using_buffer;
    qDebug() << "That would result in" << (max_frames_using_buffer / framerate) << "seconds with a framefrate of" << framerate << "fps";

    std::vector<Image> images;
    std::vector<Image> images_over_4gb;

    int index;
    for (index = 0; index < max_frames_using_buffer_cap; index++)
        images.push_back(createImage(width, height, index));
    if (max_frames_using_buffer_rest > 0)
        qDebug() << "4096 MB allocated so far...";
    for (index = 0; index < max_frames_using_buffer_rest; index++)
        images_over_4gb.push_back(createImage(width, height, index));

    usage_stats.retrieveInfo();

    qDebug() << "Used memory after storing images:" << usage_stats.used() << "MB";
    qDebug() << "Unused memory after storing images:" << usage_stats.unused() << "MB";

    return 0;
}
