#include "decoder.h"
#include "player.h"

#include <QPainter>
#include <QtWidgets>

#define PROGRESS_BAR_HEIGHT 35

VideoPlayer::VideoPlayer(QWidget *parent, Qt::WindowFlags f) :
    QDialog(parent, f)
  , mainLayout(new QVBoxLayout(this))
{
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    connect(&decoder, SIGNAL(newFrame(const Image *)), this, SLOT(receiveFrame(const Image *)));
    connect(&decoder, SIGNAL(error(const QString &)), this, SLOT(error(const QString &)));

    setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
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

    firstFrame = true;
    frameIndex = 0;

    elapsedTimer.start();

    setMinimumSize(QSize(0, PROGRESS_BAR_HEIGHT));

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
    if (!firstFrame && isVisible())
        decoder.resize(event->size() - QSize(0, PROGRESS_BAR_HEIGHT));
}

void VideoPlayer::receiveFrame(const Image *image)
{
    // This just creates a representation as QImage
    // -> no copy is made!
    this->image = image->toQImage();

    if (firstFrame && isVisible()) {
        resize(decoder.info().width, decoder.info().height + PROGRESS_BAR_HEIGHT);
        frameRate = decoder.info().framerate;
        firstFrame = false;
    }

    frameIndex++;

    update();

    // Determine interval till next frame
    qint64 interval = frameIndex * 1000 / frameRate - elapsedTimer.elapsed();
    if (interval <= 0) {
        // In case the player lags, reset the frame counter and timer
        frameIndex = 0;
        elapsedTimer.start();
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

VideoProgressBar::VideoProgressBar(QWidget *parent) :
    QWidget(parent)
{
}

QSize VideoProgressBar::sizeHint() const
{
    return QSize(0, PROGRESS_BAR_HEIGHT);
}
