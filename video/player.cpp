#include "decoder.h"
#include "player.h"

#include <QPainter>
#include <QtWidgets>

#define PROGRESS_BAR_HEIGHT 35

#ifdef QT_DEBUG
#define LOG_DESTROYED(obj) connect(obj, &QObject::destroyed, []() { fprintf(stderr, "Destroyed pointer of %s\n", #obj); })
#else
#define LOG_DESTROYED(obj)
#endif

VideoPlayer::VideoPlayer(QWidget *parent, Qt::WindowFlags f) :
    QDialog(parent, f)
  , mainLayout(new QVBoxLayout(this))
  , videoCanvas(new VideoCanvas)
  , progressBar(new ProgressBar)
{
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(videoCanvas, 1);
    mainLayout->addWidget(progressBar);
    mainLayout->setAlignment(progressBar, Qt::AlignBottom);

    progressBar->setFixedHeight(PROGRESS_BAR_HEIGHT);

    setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));

    LOG_DESTROYED(mainLayout);
    LOG_DESTROYED(videoCanvas);
    LOG_DESTROYED(progressBar);
}

void VideoPlayer::runVideo(const VideoFile &video)
{
    videoCanvas->runVideo(video);

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

QSize VideoPlayer::sizeHint() const
{
    return QSize(400, 300 + PROGRESS_BAR_HEIGHT);
}

VideoCanvas::VideoCanvas(QWidget *parent) :
    QWidget(parent)
{
    connect(&decoderThread, SIGNAL(newFrame(const Image *)), this, SLOT(receiveFrame(const Image *)));
    connect(&decoderThread, SIGNAL(error(const QString &)), this, SLOT(error(const QString &)));
}

void VideoCanvas::runVideo(const VideoFile &video)
{
    decoderThread.setFile(video);
    decoderThread.resize(QSize(400, 300));
    decoderThread.start();

    firstFrame = true;
    frameIndex = 0;

    elapsedTimer.start();
}

void VideoCanvas::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    if (image != QImage())
        painter.drawImage(QPoint(0, 0), image);
}

void VideoCanvas::resizeEvent(QResizeEvent *event)
{
    decoderThread.resize(event->size());
}

QSize VideoCanvas::sizeHint() const
{
    return QSize(400, 300);
}

void VideoCanvas::receiveFrame(const Image *image)
{
    // This just creates a representation as QImage
    // -> no copy is made!
    this->image = image->toQImage();

    if (firstFrame && isVisible()) {
//        resize(decoder.info().width, decoder.info().height);
        frameRate = decoderThread.info().framerate;
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
        decoderThread.next();
        return;
    } else if (interval > 1000)
        interval = 1000;
    QTimer::singleShot(static_cast<int>(interval), Qt::PreciseTimer, &decoderThread, &DecoderThread::next);
}

void VideoCanvas::error(const QString &msg)
{
    fprintf(stderr, "%s\n", msg.toStdString().c_str());
    fflush(stderr);
}

ProgressBar::ProgressBar(QWidget *parent) :
    QWidget(parent)
{
}

QSize ProgressBar::sizeHint() const
{
    return QSize(0, PROGRESS_BAR_HEIGHT);
}
