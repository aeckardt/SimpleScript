#ifndef TST_VIDEO_H
#define TST_VIDEO_H

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "createimage.h"
#include "image/image.h"
#include "video/decoder.h"
#include "video/encoder.h"

#include <QImage>
#include <QThread>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

using namespace testing;

TEST(Video, EncodeAndDecode)
{
    int width = 256;
    int height = 256;
    int frames = 60;
    int framerate = 25;

    // Creates temporary file
    VideoFile video_file;

    VideoEncoder encoder;
    encoder.setFile(video_file);
    encoder.create(width, height, framerate);

    int i;
    for (i = 0; i < frames; i++) {
        encoder.nextFrame() = createImage(width, height, i, 32);
        encoder.encodeFrame();
    }

    encoder.flush();

    VideoDecoder decoder;
    decoder.open(video_file);

    for (i = 0; i < frames; i++) {
        decoder.decodeFrame();

        ASSERT_FALSE(decoder.eof());

        const Image &img = decoder.nextFrame();
        EXPECT_EQ(img.size(), QSize(width, height));
        EXPECT_EQ(img, createImage(width, height, i));
    }

    decoder.decodeFrame();

    ASSERT_TRUE(decoder.eof());
}

#endif // TST_VIDEO_H
