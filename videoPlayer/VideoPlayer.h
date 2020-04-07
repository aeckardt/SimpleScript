#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QDialog>
#include <QThread>
#include <QPixmap>
#include <QImage>
#include <QElapsedTimer>

#include "VideoDecoder.h"

class VideoPlayer : public QDialog
{
    Q_OBJECT

public:
    VideoPlayer(QWidget *parent = nullptr);

    void paintEvent(QPaintEvent *event) override;

    void runVideo(const QString &fileName);

protected:
    void keyPressEvent(QKeyEvent *) override;

private slots:
    void receiveFrame(const QImage *image);
    void error(const QString &msg);

private:
    VideoDecoder decoder;
    const QImage *image;
    bool first_frame;
    int frame_index;
    int frame_rate;

    QElapsedTimer elapsed_timer;
};

#endif // VIDEOPLAYER_H
