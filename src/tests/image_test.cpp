#include <gtest/gtest.h>

#include "images/image.hpp"

TEST(ImageTest, StoresDimensionsCorrectly) {
    Image image(10, 5, RGB8);

    EXPECT_EQ(image.format(), RGB8);
    EXPECT_EQ(image.width(), 10);
    EXPECT_EQ(image.height(), 5);
}

TEST(ImageTest, CalculatesGray8StrideCorrectly) {
    Image image(10, 5, Gray8);

    EXPECT_EQ(image.stride(), 10);
}

TEST(ImageTest, CalculatesRGB8StrideCorrectly) {
    Image image(10, 5, RGB8);

    EXPECT_EQ(image.stride(), 30);
}

TEST(ImageTest, CalculatesRGBA8StrideCorrectly) {
    Image image(10, 5, RGBA8);

    EXPECT_EQ(image.stride(), 40);
}

TEST(ImageTest, CalculatesGray8ByteSizeCorrectly) {
    Image image(10, 5, Gray8);

    EXPECT_EQ(image.byteSize(), 50);
}

TEST(ImageTest, CalculatesRGB8ByteSizeCorrectly) {
    Image image(10, 5, RGB8);

    EXPECT_EQ(image.byteSize(), 150);
}

TEST(ImageTest, CalculatesRGBA8ByteSizeCorrectly) {
    Image image(10, 5, RGBA8);

    EXPECT_EQ(image.byteSize(), 200);
}

TEST(ImageTest, CreatesNullImage) {
    Image image;

    EXPECT_TRUE(image.isNull());

    EXPECT_EQ(image.width(), 0);
    EXPECT_EQ(image.height(), 0);
    EXPECT_EQ(image.stride(), 0);
    EXPECT_EQ(image.byteSize(), 0);
}

TEST(ImageTest, InitializesPixelsWithZeros) {
    Image image(2, 2, RGB8);

    for (size_t i = 0; i < image.byteSize(); i++) {
        EXPECT_EQ(image.data()[i], 0);
    }
}

TEST(ImageTest, WritesAndReadsPixel) {
    Image image(2, 2, RGB8);

    uint8_t* pixels = image.data();

    int offset = image.stride() + 3;

    pixels[offset] = 255;
    pixels[offset + 1] = 128;
    pixels[offset + 2] = 64;

    uint8_t red = pixels[offset];
    uint8_t green = pixels[offset + 1];
    uint8_t blue = pixels[offset + 2];

    EXPECT_EQ(image.byteSize(), 12);
    EXPECT_EQ(red, 255);
    EXPECT_EQ(green, 128);
    EXPECT_EQ(blue, 64);
}