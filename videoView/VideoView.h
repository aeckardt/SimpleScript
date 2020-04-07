#ifndef VIDEOVIEW_H
#define VIDEOVIEW_H

#include <QDialog>
#include <QThread>
#include <QPixmap>
#include <QImage>
#include <QElapsedTimer>

#include "VideoReader.h"

class VideoView : public QDialog
{
    Q_OBJECT

public:
    VideoView(QWidget *parent = nullptr);

    void paintEvent(QPaintEvent *event) override;

    void runVideo(const QString &fileName);

protected:
    void keyPressEvent(QKeyEvent *) override;

private slots:
    void receiveFrame(const QImage *image);
    void error(const QString &msg);

private:
    VideoReader reader;
    const QImage *image;
    bool first_frame;
    int frame_index;
    int frame_rate;

    QElapsedTimer elapsed_timer;
};

#endif // VIDEOVIEW_H
