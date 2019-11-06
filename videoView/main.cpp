#include <QApplication>
#include <VideoView.h>

#include "../image/screenshot.h"
#include "../image/video.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Video video;

//    QRect rect = {100, 100, 500, 300};
//    video.addFrame(VideoFrame(captureRect(rect), 0));
//    video.addFrame(VideoFrame(captureRect(rect), 500));
//    video.addFrame(VideoFrame(captureRect(rect), 1000));
//    video.addFrame(VideoFrame(captureRect(rect), 1500));
//    video.addFrame(VideoFrame(captureRect(rect), 2000));
//    video.addFrame(VideoFrame(captureRect(rect), 2500));
//    video.save("/Users/albrecht/Documents/Code/GameScript/output/videofile");

    video.load("/Users/albrecht/Documents/Code/GameScript/output/videofile");

    VideoView videoView;
    videoView.showVideo(video);

    return 0;
}
