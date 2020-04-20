#include "decoder.h"
#include "player.h"

#include <QPainter>
#include <QtWidgets>

VideoPlayer::VideoPlayer(QWidget *parent) : QDialog(parent)
{
    connect(&decoder, SIGNAL(newFrame(const Image *)), this, SLOT(receiveFrame(const Image *)));
    connect(&decoder, SIGNAL(error(const QString &)), this, SLOT(error(const QString &)));
}

void VideoPlayer::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    if (image != QImage())
        painter.drawImage(QPoint(0, 0), image);
}

void VideoPlayer::runVideo(const VideoFile &video)
{
    decoder.setFile(video.fileName());
    decoder.start();

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

void VideoPlayer::resizeEvent(QResizeEvent *event)
{
    if (!first_frame && isVisible())
        decoder.resize(event->size());
}

void VideoPlayer::receiveFrame(const Image *image)
{
    this->image = image->toQImage();

    if (first_frame && isVisible()) {
        resize(decoder.info().width, decoder.info().height);
        frame_rate = decoder.info().framerate;
        first_frame = false;
    }

    frame_index++;

    update();

    // Determine interval till next frame
    qint64 interval = frame_index * 1000 / frame_rate - elapsed_timer.elapsed();
    if (interval <= 0) {
        // In case the player lags, reset the frame counter and timer
        frame_index = 0;
        elapsed_timer.start();
        decoder.next();
        return;
    } else if (interval > 1000)
        interval = 1000;
    QTimer::singleShot(static_cast<int>(interval), Qt::PreciseTimer, &decoder, &DecoderThread::next);
}

void VideoPlayer::error(const QString &msg)
{
    fflush(stderr);
    fprintf(stderr, "%s\n", msg.toStdString().c_str());
}
