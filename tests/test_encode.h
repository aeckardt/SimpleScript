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
    int width = 350;
    int height = 288;
    int framecount = 72;
    int framerate = 25;
    int i;

    // Create temporary file
    VideoFile video_file;
    video_file.createTemporary();

    // Setup encoder
    VideoEncoder encoder;
    encoder.open(video_file, width, height, framerate);

    ASSERT_EQ(encoder.last_error, "");

    encoder.frame().resize(width, height);

    // Encode frames
    for (i = 0; i < framecount; i++) {
        fillImage(encoder.frame(), i);
        encoder.addFrame();
    }

    // Flush and close file
    encoder.finish();

    VideoDecoder decoder;
    decoder.open(video_file);

    ASSERT_EQ(decoder.last_error, "");

    EXPECT_EQ(decoder.info().width, width);
    EXPECT_EQ(decoder.info().height, height);
    EXPECT_EQ(decoder.info().framerate, framerate);
    EXPECT_EQ(decoder.info().framecount, framecount);

    // Compare all frames with the created frames from the function createImage
    // and see if they match -> proves lossless compression
    for (i = 0; i < framecount; i++) {
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

TEST(Video, SeekFrame)
{
    int width = 352;
    int height = 288;
    int framecount = 200;
    int framerate = 30;
    int i;

    // Create temporary file
    VideoFile video_file;
    video_file.createTemporary();

    // Setup encoder
    VideoEncoder encoder;
    encoder.open(video_file, width, height, framerate);

    ASSERT_EQ(encoder.last_error, "");

    encoder.frame().resize(width, height);

    // Encode frames
    for (i = 0; i < framecount; i++) {
        fillImage(encoder.frame(), i);
        encoder.addFrame();
    }

    // Flush and close file
    encoder.finish();

    VideoDecoder decoder;
    decoder.open(video_file);

    ASSERT_EQ(decoder.last_error, "");

    EXPECT_EQ(decoder.info().width, width);
    EXPECT_EQ(decoder.info().height, height);
    EXPECT_EQ(decoder.info().framerate, framerate);
    EXPECT_EQ(decoder.info().framecount, framecount);

    // See if frame seeking works for single frame
    std::vector<int> seek_frames = {5, 199, 198, 197, 0, 15, 16, 17, 88};
    for (int seek_frame : seek_frames) {
        decoder.seek(seek_frame);
        ASSERT_TRUE(decoder.readFrame());

        decoder.swsScale();

        const Image &img = decoder.frame();
        EXPECT_EQ(img.size(), QSize(width, height));
        EXPECT_EQ(img, createImage(width, height, seek_frame));
    }

    // Jump to frame and then see if subsequent frames are read afterwards
    decoder.seek(91);
    for (i = 91; i < 104; i++) {
        // Assert that decoder has one more frame
        ASSERT_TRUE(decoder.readFrame());

        decoder.swsScale();

        const Image &img = decoder.frame();
        EXPECT_EQ(img.size(), QSize(width, height));
        EXPECT_EQ(img, createImage(width, height, i));
    }
}

#endif // TEST_VIDEO_H
