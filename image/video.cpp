#include "image/screenshot.h"
#include "image/video.h"

#include <QEventLoop>
#include <QBuffer>
#include <QFile>

bool Video::load(const QString &str)
{
    frames.clear();

    QFile file(str);
    file.open(QIODevice::ReadOnly);

    QByteArray ba;

    size_t size;
    file.read(reinterpret_cast<char *>(&size), sizeof(size));

    if (size > 0)
    {
        QSize image_size;
        file.read(reinterpret_cast<char *>(&image_size), sizeof(image_size));

        size_t index;
        for (index = 0; index < size; ++index)
        {
            qint64 bytes;
            file.read(reinterpret_cast<char *>(&bytes), sizeof(bytes));
            ba += file.read(bytes);

            QImage img;
            img.loadFromData(ba, "PNG");
            addFrame(VideoFrame(std::move(img), index * 500));

            ba.clear();
        }
    }

    file.close();

    return true;
}

bool Video::save(const QString &str) const
{
    QByteArray ba;
    QBuffer buffer(&ba);
    QFile file(str);

    buffer.open(QIODevice::WriteOnly);
    file.open(QIODevice::WriteOnly);

    size_t size = this->size();
    file.write(reinterpret_cast<char *>(&size), sizeof(size));

    if (size > 0)
    {
        const QImage &first_img = frame(0).image();

        QSize image_size = first_img.size();
        file.write(reinterpret_cast<char *>(&image_size), sizeof(image_size));

        if (first_img.format() == QImage::Format_RGB32)
        {
            for (const VideoFrame &videoFrame : frames)
            {
                videoFrame.image().save(&buffer, "PNG");

                qint64 bytes = buffer.size();

                file.write(reinterpret_cast<char*>(&bytes), sizeof(bytes));
                file.write(ba);

                buffer.close();
                buffer.open(QIODevice::WriteOnly);
            }
        }
    }

    file.close();

    return true;
}

Recorder::Recorder(const QRect &rect, Video &video_ref, int frame_rate)
    : QObject(nullptr), rect(rect), video(&video_ref), frame_rate(frame_rate), hotkey(QKeySequence("Ctrl+."), false)
{
    QObject::connect(&timer, SIGNAL(timeout()), this, SLOT(timeout()));
    QObject::connect(&hotkey, SIGNAL(activated()), this, SLOT(hotkeyPressed()));

    if (frame_rate > 0 && frame_rate <= 30)
        interval = 1000 / frame_rate;
    else if (frame_rate <= 0)
        interval = 1000;
    else // if (frame_rate > 30)
        interval = 30;
}

void Recorder::hotkeyPressed()
{
    timer.stop();
    hotkey.setRegistered(false);
    loop.quit();
}

void Recorder::timeout()
{
    if (rect.size() == QSize(0, 0))
        video->addFrame(VideoFrame(captureDesktop(), elapsed_timer.elapsed()));
    else
        video->addFrame(VideoFrame(captureRect(rect), elapsed_timer.elapsed()));
}

void Recorder::exec()
{
    elapsed_timer.start();

    timeout();

    hotkey.setRegistered(true);

    timer.setInterval(interval);
    timer.start();

    loop.exec();
}
