#include "VideoReader.h"
#include "VideoView.h"

#include <QPainter>

VideoView::VideoView(QWidget *parent) : QDialog(parent)
{
    image = nullptr;

    connect(&reader, SIGNAL(newFrame(const QImage *)), this, SLOT(receiveFrame(const QImage *)));
    connect(&reader, SIGNAL(error(const QString &)), this, SLOT(error(const QString &)));
}

void VideoView::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    if (image != nullptr) {
        painter.drawImage(QPoint(0, 0), *image);
    }
}

void VideoView::runVideo(const QString &fileName)
{
    reader.setFileName(fileName);
    reader.start();

    firstFrame = true;

    show();

    exec();
}

void VideoView::receiveFrame(const QImage *image)
{
    this->image = image;

    if (firstFrame && isVisible()) {
        setFixedSize(image->size());
        firstFrame = false;
    }

    update();
    reader.next();
}

void VideoView::error(const QString &msg)
{
    fflush(stderr);
    fprintf(stderr, "%s\n", msg.toStdString().c_str());
}
