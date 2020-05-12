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

class VideoProgressBar;

class VideoPlayer : public QDialog
{
    Q_OBJECT

public:
    VideoPlayer(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    void paintEvent(QPaintEvent *event) override;

    void runVideo(const VideoFile &video);

protected:
    void keyPressEvent(QKeyEvent *) override;
    void resizeEvent(QResizeEvent *) override;

private slots:
    void receiveFrame(const Image *image);
    void error(const QString &msg);

private:
    QVBoxLayout *mainLayout;
    VideoProgressBar *progressBar;

    DecoderThread decoder;
    QImage image;
    bool firstFrame;
    int frameIndex;
    int frameRate;

    QElapsedTimer elapsedTimer;
};

class VideoProgressBar : public QWidget
{
    Q_OBJECT

public:
    VideoProgressBar(QWidget *parent = nullptr);

protected:
    QSize sizeHint() const override;
};

#endif // PLAYER_H
