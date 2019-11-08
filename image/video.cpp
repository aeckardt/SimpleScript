#include "image/screenshot.h"
#include "image/video.h"

#include <QEventLoop>
#include <QBuffer>
#include <QFile>

VideoFrame::VideoFrame(const VideoFrame &src)
{
    if (src.img != nullptr) {
        img = new QImage(*src.img);
        byte_array = nullptr;
        _size = nullptr;
    } else if (src.byte_array != nullptr && src._size != nullptr) {
        img = nullptr;
        byte_array = new QByteArray(*src.byte_array);
        _size = new QSize(*src._size);
    } else {
        img = nullptr;
        byte_array = nullptr;
        _size = nullptr;
    }
    ms = src.ms;
}

VideoFrame::VideoFrame(VideoFrame &&src)
{
    if (src.img != nullptr) {
        img = new QImage(std::move(*src.img));
        byte_array = nullptr;
        _size = nullptr;
    } else if (src.byte_array != nullptr && src._size != nullptr) {
        img = nullptr;
        byte_array = new QByteArray(std::move(*src.byte_array));
        _size = new QSize(std::move(*src._size));
    } else {
        img = nullptr;
        byte_array = nullptr;
        _size = nullptr;
    }
    ms = src.ms;
}

void VideoFrame::clear()
{
    if (img != nullptr) {
        delete img;
    }
    if (byte_array != nullptr) {
        delete byte_array;
    }
    if (_size != nullptr)
    {
        delete _size;
    }
}

VideoFrame &VideoFrame::operator=(const VideoFrame &src)
{
    clear();

    if (src.img != nullptr) {
        img = new QImage(*src.img);
    }
    else if (src.byte_array != nullptr)
    {
        byte_array = new QByteArray(*src.byte_array);
        _size = new QSize(*src._size);
    }
    ms = src.ms;

    return *this;
}

void VideoFrame::compress()
{
    if (img == nullptr) {
        return;
    }

    int byte_size = img->size().width() * img->size().height() * 4;

    if (byte_array != nullptr) {
        delete byte_array;
    }
    byte_array = new QByteArray(qCompress(img->bits(), byte_size, 1));

    if (_size != nullptr) {
        delete _size;
    }
    _size = new QSize(img->size());

    delete img;
    img = nullptr;
}

void VideoFrame::decompress()
{
    if (byte_array == nullptr) {
        return;
    }

    if (img != nullptr) {
        delete img;
    }
    img = new QImage(*_size, QImage::Format_RGB32);
    img->loadFromData(qUncompress(*byte_array));

    delete byte_array;
    byte_array = nullptr;
    delete _size;
    _size = nullptr;
}

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
    : QObject(nullptr), rect(rect), video(&video_ref), frame_rate(frame_rate),
      last_compressed_frame(-1), hotkey(QKeySequence("Ctrl+."), false)
{
    connect(&timer, &QTimer::timeout, this, &Recorder::timeout);
    connect(&hotkey, &QHotkey::activated, this, &Recorder::hotkeyPressed);

    CompressionWorker *worker = new CompressionWorker;
    worker->moveToThread(&compressThread);
    connect(&compressThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(this, &Recorder::compress, worker, &CompressionWorker::compressFrame);

    if (frame_rate > 0 && frame_rate <= 30)
        interval = 1000 / frame_rate;
    else if (frame_rate <= 0)
        interval = 1000;
    else // if (frame_rate > 30)
        interval = 30;
}

Recorder::~Recorder()
{
    compressThread.wait();
}

void Recorder::hotkeyPressed()
{
    compressThread.quit();
    timer.stop();
    hotkey.setRegistered(false);
    loop.quit();
}

void Recorder::timeout()
{
    if (rect.size() == QSize(0, 0)) {
        video->addFrame(VideoFrame(captureDesktop(), elapsed_timer.elapsed()));
    }
    else {
        video->addFrame(VideoFrame(captureRect(rect), elapsed_timer.elapsed()));
    }
    compress(video->frame(video->size() - 1));
}

void Recorder::exec()
{
    elapsed_timer.start();

    compressThread.start();

    timeout();

    hotkey.setRegistered(true);

    timer.setInterval(interval);
    timer.start();

    loop.exec();
}
