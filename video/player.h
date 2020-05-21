#ifndef PLAYER_H
#define PLAYER_H

#include <QWidget>
#include <QDialog>
#include <QElapsedTimer>

#include "decoder.h"
#include "videofile.h"

QT_BEGIN_NAMESPACE
class QVBoxLayout;
QT_END_NAMESPACE

class VideoCanvas;
class ProgressBar;

class VideoPlayer : public QDialog
{
    Q_OBJECT

public:
    VideoPlayer(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    void runVideo(const VideoFile &video);

protected:
    void keyPressEvent(QKeyEvent *) override;
    QSize sizeHint() const override;

private:
    QVBoxLayout *mainLayout;

    VideoCanvas *videoCanvas;
    ProgressBar *progressBar;
};

class VideoCanvas : public QWidget
{
    Q_OBJECT

public:
    VideoCanvas(QWidget *parent = nullptr);

    void runVideo(const VideoFile &video);

protected:
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *event) override;
    QSize sizeHint() const override;

private slots:
    void receiveFrame(const Image *img);
    void error(const QString &msg);

private:
    QImage image;

    DecoderThread decoderThread;
    bool firstFrame;
    int frameIndex;
    int frameRate;

    QElapsedTimer elapsedTimer;
};

class ProgressBar : public QWidget
{
    Q_OBJECT

public:
    ProgressBar(QWidget *parent = nullptr);

protected:
    QSize sizeHint() const override;
};

#endif // PLAYER_H
