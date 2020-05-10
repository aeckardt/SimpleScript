#include <QApplication>
#include <QSettings>
#include <QElapsedTimer>

#include <iostream>
#include <string>

#include "image/image.h"
#include "tests/createimage.h"
#include "video/decoder.h"
#include "video/encoder.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    int width = 352;
    int height = 288;
    int framecount = 200;
    int framerate = 30;
    int i;

    // Create temporary file
    VideoFile video_file;
    video_file.createTemporary();

    QElapsedTimer elapsed_timer;
    elapsed_timer.start();

    // Setup encoder
    VideoEncoder encoder;
    encoder.open(video_file, width, height, framerate);

    if (encoder.last_error != "") {
        fprintf(stderr, "Error opening encoder: %s\n", encoder.last_error.toStdString().c_str());
        return -1;
    }

    fprintf(stdout, "Opening encoder took %lld milliseconds.\n", elapsed_timer.restart());

    encoder.frame().resize(width, height);

    // Encode frames
    for (i = 0; i < framecount; i++) {
        fillImage(encoder.frame(), i);
        encoder.addFrame();
    }

    fprintf(stdout, "Encoding %d frames took %lld milliseconds.\n", framecount, elapsed_timer.restart());

    // Flush and close file
    encoder.finish();

    fprintf(stdout, "Finishing encoding and closing file took %lld milliseconds.\n\n", elapsed_timer.restart());

    VideoDecoder decoder;
    decoder.open(video_file);

    if (decoder.last_error != "") {
        fprintf(stderr, "Error opening decoder: %s\n", decoder.last_error.toStdString().c_str());
        return -1;
    }

    fprintf(stdout, "Opening decoder took %lld milliseconds.\n", elapsed_timer.restart());

    std::vector<int> frame_numbers = {99, 98, 23, 199, 198, 0, 199, 5, 6, 7, 6, 5, 209};

    for (int frame_number : frame_numbers) {
        decoder.seek(frame_number);
        qint64 tdelta_seeking = elapsed_timer.restart();

        bool eof = !decoder.readFrame();
        fprintf(stdout, "Seeking frame %03d took %lld milliseconds -> reading %s, took %lld milliseconds.\n",
                frame_number, tdelta_seeking, (eof ? "failed" : "successful"), elapsed_timer.restart());
    }

    return 0;
}
