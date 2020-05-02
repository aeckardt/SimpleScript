#include <QApplication>
#include <QElapsedTimer>

#include <iostream>

#include "image/image.h"
#include "tests/createimage.h"
#include "utils/circularqueue.h"
#include "utils/memoryusage.h"

typedef CircularQueue<Image> FrameQueue;

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QElapsedTimer elapsed_timer;
    elapsed_timer.start();

    MemoryUsage usage_stats;
    usage_stats.retrieveInfo();

    std::cout << "Used memory: " << usage_stats.used() << " MB" << std::endl;
    std::cout << "Unused memory: " << usage_stats.unused() << " MB" << std::endl;
    std::cout << "Total memory: " << usage_stats.total() << " MB" << std::endl;

    qint64 t = elapsed_timer.elapsed();

    std::cout << "The memory usage request took " << t << " ms" << std::endl;

    int memory_buffer;
    if (usage_stats.total() < 8000)
        memory_buffer = 500; // MB
    else
        memory_buffer = 700;

    int unused = usage_stats.unused();
    int unused_with_buffer = std::max(unused - memory_buffer, 0);

    int width = 1800;
    int height = 1200;
    int framerate = 15;

    int num_bytes = height * width * 4;

    int max_frames = (unused * 1024) / (num_bytes / 1024);
    int max_frames_using_buffer = (unused_with_buffer * 1024) / (num_bytes / 1024);

    std::cout << "Frame size: QSize(" << width << ", " << height << ") -> corresponding to " << num_bytes << " Bytes" << std::endl;
    std::cout << "Max frames to store: " << max_frames << std::endl;
    std::cout << "With " << memory_buffer << " MB left as buffer: " << max_frames_using_buffer << std::endl;
    std::cout << "That would result in " << (max_frames_using_buffer / framerate) << " seconds with a framefrate of " << framerate << " fps" << std::endl;

    FrameQueue images(num_bytes);
    images.resize(max_frames_using_buffer);

    Image created = createImage(width, height, 0);
    Image temp;

    int index;
    for (index = 0; index < max_frames_using_buffer; index++) {
        temp = created;
        images.push(temp);
    }

    usage_stats.retrieveInfo();

    std::cout << "Used memory after storing images: " << usage_stats.used() << " MB" << std::endl;
    std::cout << "Unused memory after storing images: " << usage_stats.unused() << " MB" << std::endl;

    for (index = 0; index < max_frames * 20; index++) {
        if (!images.full())
            std::cout << "Error: CircularQueue should be full!";
        images.pop(temp);
        images.push(temp);
    }

    for (index = 0; index < max_frames; index++) {
        images.pop(temp);
    }

    if (!images.empty())
        std::cout << "Error: CircularQueue should be empty!";

    std::cout << "Used memory after cycling through queue: " << usage_stats.used() << " MB" << std::endl;
    std::cout << "Unused memory after cycling through queue: " << usage_stats.unused() << " MB" << std::endl;

    return 0;
}
