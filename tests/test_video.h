#ifndef TEST_VIDEO_H
#define TEST_VIDEO_H

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "createimage.h"
#include "image/image.h"
#include "video/decoder.h"
#include "video/encoder.h"

#include <QImage>

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

    // Setup encoder
    VideoEncoder encoder;
    encoder.setFile(video_file);
    encoder.create(width, height, framerate);

    EXPECT_GE(encoder.last_error, "");
    if (encoder.last_error != "")
        return;

    // Encode frames
    int i;
    for (i = 0; i < frames; i++) {
        encoder.frame() = createImage(width, height, i, 32);
        encoder.writeFrame();
    }

    // Flush and close file
    encoder.flush();

    // Setup decoder
    VideoDecoder decoder;
    decoder.open(video_file);

    EXPECT_GE(decoder.last_error, "");
    if (decoder.last_error != "")
        return;

    // Compare all frames with the created frames from the function createImage
    // and see if they match -> proves lossless compression
    for (i = 0; i < frames; i++) {
        ASSERT_TRUE(decoder.readFrame());

        if (decoder.eof())
            return;

        decoder.scaleFrame();

        const Image &img = decoder.frame();
        EXPECT_EQ(img.size(), QSize(width, height));
        EXPECT_EQ(img, createImage(width, height, i));
    }

    // After the last frame, there should be no more!
    ASSERT_FALSE(decoder.readFrame());
}

#endif // TEST_VIDEO_H
