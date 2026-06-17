#include <gtest/gtest.h>
#include <cstdlib>
#include "image.hpp"
#include "sobel.hpp"

TEST(Sobel, UniformImage)
{
    Image img(100, 75);
    for(int i = 0; i < img.width * img.height; i++)
        img.data[i] = 100;
    Gradient g = sobel(img);
    for(int y = 1; y < 74; y++)
        for(int x = 1; x < 99; x++)
        {
            int i = y * 100 + x;
            EXPECT_EQ(g.gx[i], 0);
            EXPECT_EQ(g.gy[i], 0);
        }
}

TEST(Sobel, VerticalEdge)
{
    Image img(32, 32);
    for(int y = 0; y < 32; y++)
        for(int x = 16; x < 32; x++)
            img.at(x, y) = 255;
    Gradient g = sobel(img);
    int idx = 16 * 32 + 16;
    EXPECT_GT(std::abs(g.gx[idx]), 100);
    EXPECT_LT(std::abs(g.gy[idx]), 20);
}

TEST(Sobel, HorizontalEdge)
{
    Image img(32, 32);
    for(int y = 16; y < 32; y++)
        for(int x = 0; x < 32; x++)
            img.at(x, y) = 255;
    Gradient g = sobel(img);
    int idx = 15 * 32 + 14;
    EXPECT_GT(std::abs(g.gy[idx]), 100);
    EXPECT_LT(std::abs(g.gx[idx]), 20);
}

TEST(Sobel, DiagonalEdge)
{
    Image img(32, 32);
    for(int y = 0; y < 32; y++)
        for(int x = 0; x < 32; x++)
            if(x + y >= 32)
                img.at(x, y) = 255;
    Gradient g = sobel(img);
    int idx = 15 * 32 + 15;
    EXPECT_GT(std::abs(g.gx[idx]), 50);
    EXPECT_GT(std::abs(g.gy[idx]), 50);
}