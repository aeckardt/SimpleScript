#include "decoder.h"
#include "player.h"

#include <QPainter>
#include <QtWidgets>

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
  , playing(false)
{
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(videoCanvas, 1);
    mainLayout->addWidget(progressBar);
    mainLayout->setAlignment(progressBar, Qt::AlignBottom);

    progressBar->setFixedHeight(PROGRESS_BAR_HEIGHT);

    setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));

    connect(&decoderThread, &DecoderThread::newFrame, this, &VideoPlayer::receiveFrame);
    connect(&decoderThread, &DecoderThread::error, this, &VideoPlayer::error);
    connect(progressBar, &ProgressBar::changePosition, this, &VideoPlayer::moveSliderHandle);

//    LOG_DESTROYED(mainLayout);
//    LOG_DESTROYED(videoCanvas);
//    LOG_DESTROYED(progressBar);
}

void VideoPlayer::runVideo(const VideoFile &video)
{
    decoderThread.setFile(video);
    decoderThread.resize(QSize(400, 300));
    decoderThread.start();

    firstFrame = true;
    frameIndex = 0;

    elapsedTimer.start();

    playing = true;

    setMinimumSize(QSize(0, PROGRESS_BAR_HEIGHT));

    show();
    exec();
}

void VideoPlayer::keyPressEvent(QKeyEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        if (event->key() == Qt::Key_W) {
            // Close when Ctrl+W is pressed
            decoderThread.stop();
            close();
        }
    }
}

void VideoPlayer::resizeEvent(QResizeEvent *event)
{
    decoderThread.resize(event->size() - QSize(0, PROGRESS_BAR_HEIGHT));
}

QSize VideoPlayer::sizeHint() const
{
    return QSize(400, 300 + PROGRESS_BAR_HEIGHT);
}

void VideoPlayer::receiveFrame(const Image *image)
{
    // This just creates a representation as QImage
    // -> no copy is made!
    videoCanvas->image = image->toQImage();

    if (firstFrame && isVisible()) {
//        resize(decoder.info().width, decoder.info().height);
        frameRate = decoderThread.info().framerate;
        firstFrame = false;
        progressBar->setSize(decoderThread.info().framecount);
    }

    frameIndex++;

    videoCanvas->update();
    progressBar->setIndex(decoderThread.frameIndex());

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

void VideoPlayer::error(const QString &msg)
{
    fprintf(stderr, "%s\n", msg.toStdString().c_str());
    fflush(stderr);
}

void VideoPlayer::moveSliderHandle(int newPosition)
{
    decoderThread.seekFrame(newPosition);
}

void VideoCanvas::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    if (image != QImage())
        painter.drawImage(QPoint(0, 0), image);
}

QSize VideoCanvas::sizeHint() const
{
    return QSize(400, 300);
}

ProgressBar::ProgressBar(QWidget *parent) :
    QWidget(parent)
  , layout(new QHBoxLayout(this))
  , frameLabel(new QLabel)
  , dragMode(false)
{
    layout->addWidget(frameLabel, 0, Qt::AlignRight);
}

void ProgressBar::setIndex(int current)
{
    this->current = current;
    if (frameCount > 0) {
        frameLabel->setText(QString().sprintf("%d / %d", current + 1, frameCount));
        update();
    } else if (frameLabel->text() != "")
        frameLabel->setText("");
}

void ProgressBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() & Qt::LeftButton) {
        if (!sliderClickRegion.isNull() && sliderClickRegion.contains(event->pos())) {
            dragMode = true;
            emitSliderPosition(event->pos().x());
        }
    }
}

void ProgressBar::mouseMoveEvent(QMouseEvent *event)
{
    if (event->button() & Qt::LeftButton && dragMode)
        emitSliderPosition(event->pos().x());
}

void ProgressBar::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() & Qt::LeftButton && dragMode)
        dragMode = false;
}

void ProgressBar::drawUpperBorder(QPainter &painter)
{
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(127, 127, 127));

    painter.save();
    painter.drawRect(0, 0, width(), 1);
    painter.restore();

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255));

    painter.save();
    painter.drawRect(0, 1, width(), 1);
    painter.restore();
}

void ProgressBar::drawSlider(QPainter &painter)
{
    calculateSliderDimensions();

    if (frameCount > 0 && current >= 0 && sliderWidth > 0) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(159, 159, 159));

        painter.save();
        painter.drawRect(sliderRect);
        painter.restore();

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(95, 95, 95));

        painter.save();
        painter.drawEllipse(sliderHandleRect);
        painter.restore();

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 127, 127));

        painter.save();
        painter.drawEllipse(sliderHandleRect.adjusted(1, 1, -1, -1));
        painter.restore();
    }
}

void ProgressBar::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    drawUpperBorder(painter);
    drawSlider(painter);
}

void ProgressBar::calculateSliderDimensions()
{
    sliderLeft = 60;
    sliderTop = PROGRESS_BAR_HEIGHT / 2;
    sliderWidth = width() - sliderLeft - 95; // leave space for the label
    ellipseDiameter = 14;

    sliderRect = {sliderLeft, sliderTop, sliderWidth, 1};

    sliderPosition = sliderLeft + static_cast<int>(static_cast<double>(sliderWidth) *
                                                   (static_cast<double>(current) /
                                                   static_cast<double>(frameCount)));

    sliderHandleRect = {sliderPosition + 2 - ellipseDiameter / 2,
                        sliderTop + 2 - ellipseDiameter / 2,
                        ellipseDiameter - 2,
                        ellipseDiameter - 2};

    if (sliderWidth > 0)
        sliderClickRegion = QRegion(sliderRect.adjusted(-1, -1, 1, 1), QRegion::Rectangle) +
                            QRegion(sliderHandleRect, QRegion::Ellipse);
    else
        sliderClickRegion = QRegion();
}

void ProgressBar::emitSliderPosition(int x)
{
    int newFrameIndex = std::min(frameCount - 1, std::max(0,
                                 (x - sliderLeft) * frameCount / sliderWidth));
    emit changePosition(newFrameIndex);
}
