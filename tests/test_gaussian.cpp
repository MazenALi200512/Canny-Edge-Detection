#include <gtest/gtest.h>
#include "image.hpp"
#include "gaussian.hpp"

TEST(Gaussian, UniformImage)
{
    Image img(100, 75);
    for(int i = 0; i < img.width * img.height; i++)
        img.data[i] = 128;
    Image out = gaussianBlur(img);
    for(int y = 2; y < 73; y++)
        for(int x = 2; x < 98; x++)
            EXPECT_NEAR(out.at(x, y), 128, 1);
}

TEST(Gaussian, BlackImage)
{
    Image img(100, 75);
    for(int i = 0; i < img.width * img.height; i++)
        img.data[i] = 0;
    Image out = gaussianBlur(img);
    for(int i = 0; i < out.width * out.height; i++)
        EXPECT_EQ(out.data[i], 0);
}

TEST(Gaussian, ImpulseResponse)
{
    Image img(21, 21);
    img.at(10, 10) = 255;
    Image out = gaussianBlur(img);
    EXPECT_GT(out.at(10, 10), 0);
    EXPECT_LT(out.at(10, 10), 255);
    EXPECT_EQ(out.at(9, 10), out.at(11, 10));
    EXPECT_EQ(out.at(10, 9), out.at(10, 11));
    EXPECT_EQ(out.at(9, 9), out.at(11, 11));
}