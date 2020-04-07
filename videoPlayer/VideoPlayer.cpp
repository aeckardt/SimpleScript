#include "VideoReader.h"
#include "VideoPlayer.h"

#include <QPainter>
#include <QtWidgets>

VideoPlayer::VideoPlayer(QWidget *parent) : QDialog(parent)
{
    image = nullptr;

    connect(&reader, SIGNAL(newFrame(const QImage *)), this, SLOT(receiveFrame(const QImage *)));
    connect(&reader, SIGNAL(error(const QString &)), this, SLOT(error(const QString &)));
}

void VideoPlayer::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    if (image != nullptr) {
        painter.drawImage(QPoint(0, 0), *image);
    }
}

void VideoPlayer::runVideo(const QString &fileName)
{
    reader.setFileName(fileName);
    reader.start();

    first_frame = true;
    frame_index = 0;

    elapsed_timer.start();

    show();

    exec();
}

void VideoPlayer::keyPressEvent(QKeyEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        if (event->key() == Qt::Key_W)
            // Close when Ctrl+W is pressed
            close();
    }
}

void VideoPlayer::receiveFrame(const QImage *image)
{
    this->image = image;

    if (first_frame && isVisible()) {
        setFixedSize(image->size());
        first_frame = false;
        frame_rate = reader.frameRate();
    }

    frame_index++;

    update();

    // Determine time till next frame and reconfigure interval
    qint64 interval = frame_index * 1000 / frame_rate - elapsed_timer.elapsed();
    if (interval <= 0) {
        // In case the player lags, reset the frame counter and timer
        frame_index = 0;
        elapsed_timer.start();
        reader.next();
        return;
    } else if (interval > 1000)
        interval = 1000;
    QTimer::singleShot(static_cast<int>(interval), Qt::PreciseTimer, &reader, &VideoReader::next);
}

void VideoPlayer::error(const QString &msg)
{
    fflush(stderr);
    fprintf(stderr, "%s\n", msg.toStdString().c_str());
}
