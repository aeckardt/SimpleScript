#include "image/screenshot.h"
#include "image/video.h"

#include <QEventLoop>
#include <QBuffer>
#include <QFile>

VideoFrame::VideoFrame(const VideoFrame &src)
{
    if (src.img != nullptr) {
        img = new QImage(*src.img);
        compressed_data = nullptr;
    } else if (src.compressed_data != nullptr) {
        img = nullptr;
        compressed_data = new CompressedData(*src.compressed_data);
    } else {
        img = nullptr;
        compressed_data = nullptr;
    }
    ms = src.ms;
}

VideoFrame::VideoFrame(VideoFrame &&src)
{
    if (src.img != nullptr) {
        img = new QImage(std::move(*src.img));
        compressed_data = nullptr;
    } else if (src.compressed_data != nullptr) {
        img = nullptr;
        compressed_data = new CompressedData(std::move(*src.compressed_data));
    } else {
        img = nullptr;
        compressed_data = nullptr;
    }
    ms = src.ms;
}

void VideoFrame::clear()
{
    if (img != nullptr) {
        delete img;
        img = nullptr;
    }
    if (compressed_data != nullptr) {
        delete compressed_data;
        compressed_data = nullptr;
    }
}

VideoFrame &VideoFrame::operator=(const VideoFrame &src)
{
    clear();

    if (src.img != nullptr)
        img = new QImage(*src.img);
    else if (src.compressed_data != nullptr)
        compressed_data = new CompressedData(*src.compressed_data);

    ms = src.ms;

    return *this;
}

void VideoFrame::compress()
{
    if (img == nullptr)
        return;

    int nbytes  = img->size().width() * img->size().height() * 4;

    if (compressed_data != nullptr)
        delete compressed_data;

    compressed_data = new CompressedData;
    compressed_data->byte_array = QByteArray(qCompress(img->bits(), nbytes, 1));
    compressed_data->size = img->size();

    delete img;
    img = nullptr;
}

void VideoFrame::decompress()
{
    if (compressed_data == nullptr)
        return;

    if (img != nullptr)
        delete img;

    QByteArray *img_bits = new QByteArray(qUncompress(compressed_data->byte_array));
    img = new QImage(reinterpret_cast<uchar *>(img_bits->data()), compressed_data->size.width(), compressed_data->size.height(),
                     QImage::Format_RGB32, [](void *byte_array) { delete static_cast<QByteArray *>(byte_array); }, img_bits);

    delete compressed_data;
    compressed_data = nullptr;
}

bool Video::load(const QString &fileName)
{
    frames.clear();

    QFile file(fileName);
    file.open(QIODevice::ReadOnly);

    QByteArray ba;

    size_t size;
    file.read(reinterpret_cast<char *>(&size), sizeof(size));

    if (size > 0) {
        QSize image_size;
        file.read(reinterpret_cast<char *>(&image_size), sizeof(image_size));

        size_t index;
        for (index = 0; index < size; ++index) {
            qint64 bytes;
            file.read(reinterpret_cast<char *>(&bytes), sizeof(bytes));
            ba += file.read(bytes);

            QImage img;
            img.loadFromData(ba, "PNG");
            addFrame(VideoFrame(std::move(img), static_cast<qint64>(index * 500)));

            ba.clear();
        }
    }

    file.close();

    return true;
}

bool Video::save(const QString &fileName) const
{
    QByteArray ba;
    QBuffer buffer(&ba);
    QFile file(fileName);

    buffer.open(QIODevice::WriteOnly);
    file.open(QIODevice::WriteOnly);

    size_t size = this->size();
    file.write(reinterpret_cast<char *>(&size), sizeof(size));

    if (size > 0) {
        const QImage &first_img = frame(0).image();

        QSize image_size = first_img.size();
        file.write(reinterpret_cast<char *>(&image_size), sizeof(image_size));

        if (first_img.format() == QImage::Format_RGB32) {
            for (const VideoFrame &videoFrame : frames) {
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
