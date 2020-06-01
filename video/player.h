#ifndef PLAYER_H
#define PLAYER_H

#include <QWidget>
#include <QDialog>
#include <QElapsedTimer>

#include "decoder.h"
#include "videofile.h"

QT_BEGIN_NAMESPACE
class QVBoxLayout;
class QHBoxLayout;
class QLabel;
QT_END_NAMESPACE

class VideoCanvas;
class ProgressBar;

#define PROGRESS_BAR_HEIGHT 35

class VideoPlayer : public QDialog
{
    Q_OBJECT

public:
    VideoPlayer(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    void runVideo(const VideoFile &video);

protected:
    void keyPressEvent(QKeyEvent *) override;
    void resizeEvent(QResizeEvent *event) override;
    QSize sizeHint() const override;

private slots:
    void receiveFrame(const Image *img);
    void error(const QString &msg);
    void moveSliderHandle(int newPosition);

private:
    QVBoxLayout *mainLayout;

    VideoCanvas *videoCanvas;
    ProgressBar *progressBar;

    DecoderThread decoderThread;

    bool firstFrame;
    int frameIndex;
    int frameRate;

    bool playing;

    QElapsedTimer elapsedTimer;
};

class VideoCanvas : public QWidget
{
    Q_OBJECT

public:
    VideoCanvas(QWidget *parent = nullptr) : QWidget(parent) {}

protected:
    void paintEvent(QPaintEvent *) override;
    QSize sizeHint() const override;

private:
    QImage image;

    friend class VideoPlayer;
};

class ProgressBar : public QWidget
{
    Q_OBJECT

public:
    ProgressBar(QWidget *parent = nullptr);

    void setSize(int frameCount) { this->frameCount = frameCount; }
    void setIndex(int current);

signals:
    void changePosition(int pos);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    void paintEvent(QPaintEvent *event) override;
    QSize sizeHint() const override { return QSize(0, PROGRESS_BAR_HEIGHT); }

private:
    QHBoxLayout *layout;
    QLabel *frameLabel;

    int frameCount;
    int current;

    int sliderLeft;
    int sliderTop;
    int sliderWidth;
    int sliderPosition;
    int ellipseDiameter;

    QRect sliderRect;
    QRect sliderHandleRect;
    QRegion sliderClickRegion;

    bool dragMode;

    void calculateSliderDimensions();
    void emitSliderPosition(int x);

    void drawUpperBorder(QPainter &painter);
    void drawSlider(QPainter &painter);
};

#endif // PLAYER_H
