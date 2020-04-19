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
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
}

using namespace testing;

TEST(Video, EncodeAndDecode)
{
    int width = 256;
    int height = 256;
    int frames = 60;
    int framerate = 25;

    // Create temporary file
    VideoFile video_file = temporaryVideo();

    // Setup encoder
    VideoEncoder encoder;
    encoder.open(video_file, width, height, framerate);

    ASSERT_EQ(encoder.last_error, "");

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

    ASSERT_EQ(decoder.last_error, "");

    // Compare all frames with the created frames from the function createImage
    // and see if they match -> proves lossless compression
    for (i = 0; i < frames; i++) {
        // Assert that decoder has one more frame
        ASSERT_TRUE(decoder.readFrame());

        decoder.swsScale();

        const Image &img = decoder.frame();
        EXPECT_EQ(img.size(), QSize(width, height));
        EXPECT_EQ(img, createImage(width, height, i));
    }

    // After the last frame, there should be no more!
    EXPECT_FALSE(decoder.readFrame());
}

#endif // TEST_VIDEO_H
