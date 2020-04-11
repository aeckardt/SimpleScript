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

#endif // TEST_IMAGE_H
