#include "VideoView.h"
#include "imageView/ImageView.h"
#include "image/video.h"

#include <QtWidgets>

VideoView::VideoView()
   : play_active(false)
   , frameImage(new FrameImage)
   , toolBar(new QToolBar)
   , mainLayout(new QVBoxLayout(this))
   , slider(new QSlider(Qt::Horizontal))
   , navigationButtons(new NavigationButtons(this))
{
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(frameImage);
    mainLayout->addWidget(slider);
    mainLayout->addWidget(toolBar);

    toolBar->addWidget(navigationButtons);

    setWindowTitle("Video Viewer");

    connect(slider, &QSlider::sliderMoved, this, &VideoView::sliderMoved);
}

void VideoView::showVideo(const Video &video)
{
    this->video = &video;    

    currentFrame = 0;
    frameImage->setVideo(video);

    slider->setMaximum(video.size());

    show();

    exec();
}

void VideoView::nextFrame()
{
    if (play_active)
    {
        frameImage->setFrame(currentFrame);
        slider->setValue(currentFrame);
        currentFrame++;

        if (video->size() > currentFrame)
        {
            qint64 msecsToNextFrame = video->frame(currentFrame).msFromStart() - elapsedTimer.elapsed();
            timer.singleShot(msecsToNextFrame, this, &VideoView::nextFrame);
        }
        else
        {
            play_active = false;
            navigationButtons->setPlayActive(play_active);
        }
    }
}

void VideoView::play()
{
    play_active = !play_active;

    if (play_active)
    {
        if (video->size() > currentFrame)
        {
            elapsedTimer.start();
            timer.singleShot(video->frame(0).msFromStart(), this, &VideoView::nextFrame);
        }
        else
        {
            play_active = false;
        }
    }
    else
    {
        timer.stop();
    }

    navigationButtons->setPlayActive(play_active);
}

void VideoView::sliderMoved(int value)
{
    currentFrame = value;
}

void FrameImage::setFrame(int frame)
{
    pixmap = QPixmap::fromImage(video->frame(frame).image());
    update();
}

void FrameImage::paintEvent(QPaintEvent *event)
{
    double scaleFactor = static_cast<double>(pixmap.size().width()) / static_cast<double>(size().width());

    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    QRectF pixmapRect;
    pixmapRect.setTopLeft(scaleFactor * QPointF(event->rect().topLeft()));
    pixmapRect.setSize(scaleFactor * QSizeF(event->rect().size()));
    painter.drawPixmap(QRectF(event->rect()), pixmap, pixmapRect);
}

QSize FrameImage::sizeHint() const
{
    return QSize(600, 400);
}

NavigationButtons::NavigationButtons(VideoView *parent)
    : QWidget(parent), parent(parent)
{
    play = {
        QIcon(":/resources/Play.png"),
        QRect(10, 10, 26, 26), false, true, false};
}

void NavigationButtons::drawButton(QPainter &painter, const ToolButton &toolButton) const
{
    QPainterPath path;
    path.addEllipse(QRectF(toolButton.rect));
    QPen pen(QColor(195, 194, 194), 1);
    painter.setPen(pen);
    painter.fillPath(path, toolButton.pressed || toolButton.active ? QColor(227, 227, 227) : QColor(246, 246, 246));
    painter.drawPath(path);
    painter.drawPixmap(toolButton.rect.topLeft() + QPoint(5, 5), toolButton.icon.pixmap(QSize(16, 16)));
}

void NavigationButtons::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    drawButton(painter, play);
}

bool NavigationButtons::buttonPressed(const QPoint &pos, ToolButton &toolButton)
{
    if (toolButton.enabled && toolButton.rect.contains(pos))
    {
        toolButton.pressed = true;
        update();
        return true;
    }

    return false;
}

void NavigationButtons::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        QPoint relPos = mapFromGlobal(event->globalPos());
        if (buttonPressed(relPos, play))
        {
            event->accept();
        }
    }
}

void NavigationButtons::mouseReleaseEvent(QMouseEvent *event)
{
    QPoint relPos = mapFromGlobal(event->globalPos());
    if (play.pressed && play.enabled && play.rect.contains(relPos))
    {
        parent->play();
    }
    if (play.pressed)
    {
        play.pressed = false;
        update();
        event->accept();
    }
}

QSize NavigationButtons::sizeHint() const
{
    return QSize(70, 70);
}
