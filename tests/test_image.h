#ifndef TEST_IMAGE_H
#define TEST_IMAGE_H

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "createimage.h"
#include "image/image.h"

#include <QTemporaryFile>
#include <QImage>
#include <QString>

using namespace testing;

TEST(Image, Copy)
{
    int width = 256;
    int height = 256;

    Image test_image = createImage(width, height, 0, 32);
    Image copied_image(test_image);

    EXPECT_EQ(test_image, copied_image);
    EXPECT_EQ(createImage(width, height, 0), copied_image);

    Image test_image_2 = createImage(width, height, 1, 32);

    EXPECT_NE(test_image, test_image_2);
}

TEST(Image, UseExternalBuffer)
{
    int width = 254;
    int height = 256;
    int linesize_alignment = 32;

    // Setup buffer to have bytes_per_row > width * 4
    size_t bytes_per_row = (width * 4 + linesize_alignment - 1) / linesize_alignment * linesize_alignment;
    size_t buffer_size = bytes_per_row * height;

    // Allocate buffer
    uint8_t *buffer = new uint8_t[buffer_size];

    // Setup image and image2 to share the same memory
    Image image(linesize_alignment);
    image.assign(buffer, width, height);
    image.enableReallocation(false);

    Image image2(linesize_alignment);
    image2.assign(buffer, width, height);

    // When image is changed, image2 should be too!
    image = createImage(width, height, 0);
    EXPECT_EQ(image2, createImage(width, height, 0));

    // Test again!
    image = createImage(width, height, 1);
    EXPECT_NE(image2, createImage(width, height, 0));
    EXPECT_EQ(image2, createImage(width, height, 1));

    // Clean up
    delete [] buffer;
}

TEST(Image, SaveAndLoad)
{
    int width = 256;
    int height = 256;

    Image test_image = createImage(width, height, 0, 32);

    QTemporaryFile temp_file;
    temp_file.open();

    QString file_name = temp_file.fileName();
    test_image.toQImage().save(file_name, "PNG");

    Image loaded_image(file_name);

    EXPECT_EQ(test_image, loaded_image);
}

TEST(Image, Screenshot)
{
    int width = 254;
    int height = 256;
    int linesize_alignment = 32;

    // In the first step, capture the screen and verify that the dimensions are correct

    // Create image
    Image image(linesize_alignment);

    // Capture screen area
    QRect screenRect = {100, 100, width, height};
    image.captureRect(screenRect);

    // Retrieve dimensions from screenshot
    int image_width = image.width();
    int image_height = image.height();

    // Verify that image is not empty
    EXPECT_NE(image.size(), QSize());

    // Verify that image has at least the dimensions of the area on the screen
    // This is particularly relevant for MacOS, because the screenshot image can
    // be bigger than the screen area
    EXPECT_GE(image_width, width);
    EXPECT_GE(image_height, height);

    // In the second step, allocate a buffer and write image into the buffer
    // Verify that something was written into the allocated buffer

    // Setup buffer to have bytes_per_row > width * 4
    size_t bytes_per_row = (image_width * 4 + linesize_alignment - 1) / linesize_alignment * linesize_alignment;
    size_t buffer_size = bytes_per_row * image_height;

    // Allocate buffer
    uint8_t *buffer = new uint8_t[buffer_size];
    memset(buffer, 0, buffer_size);

    // Create another buffer for comparison
    uint8_t *cmp_buffer = new uint8_t[buffer_size];
    memset(cmp_buffer, 0, buffer_size);

    // Assign buffer to image
    image.assign(buffer, image_width, image_height);
    image.enableReallocation(false);

    image.captureRect(screenRect);

    // See if anything was written into the buffer
    // -> validates that the screenshot function has done something!
    EXPECT_NE(memcmp(buffer, cmp_buffer, buffer_size), 0);

    // Clean up
    delete [] buffer;
    delete [] cmp_buffer;
}

#endif // TEST_IMAGE_H
