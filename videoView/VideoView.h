#ifndef VIDEOVIEW_H
#define VIDEOVIEW_H

#include <QDialog>
#include <QIcon>
#include <QTimer>

QT_BEGIN_NAMESPACE
class Video;
class QScrollArea;
class QScrollBar;
class QToolBar;
class QVBoxLayout;
class QSlider;
class QPixmap;
QT_END_NAMESPACE

class FrameImage;
class NavigationButtons;

class VideoView : public QDialog
{
    Q_OBJECT

public:
    VideoView();

    void showVideo(const Video &video);

private slots:
    void play();
    void nextFrame();

    void sliderMoved(size_t);
    void sliderReleased();

private:
    const Video *video;

    QTimer timer;

    size_t currentFrame;
    bool play_active;

    FrameImage *frameImage;
    QToolBar *toolBar;
    QVBoxLayout *mainLayout;
    QSlider *slider;
    NavigationButtons *navigationButtons;

    friend NavigationButtons;
};

class FrameImage : public QWidget
{
    Q_OBJECT

public:
    FrameImage(QWidget *parent = nullptr) : QWidget(parent) {}

    void setFrame(size_t frame);
    void setVideo(const Video &video) { this->video = &video; }

protected:
    void paintEvent(QPaintEvent *event) override;
    QSize sizeHint() const override;

private:
    const Video *video;
    QPixmap pixmap;
};

class NavigationButtons : public QWidget
{
    Q_OBJECT

public:
    NavigationButtons(VideoView *parent);

    inline void setPlayActive(bool active)
    { if (play.active != active) { play.active = active; update(); } }

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    QSize sizeHint() const override;

private:
    struct ToolButton
    {
        QIcon icon;
        QRect rect;
        bool pressed;
        bool enabled;
        bool active;
    };

    void drawButton(QPainter &painter, const ToolButton &toolButton) const;
    bool buttonPressed(const QPoint &pos, ToolButton &toolButton);

    VideoView *parent;

    ToolButton play;
};

#endif // VIDEOVIEW_H
