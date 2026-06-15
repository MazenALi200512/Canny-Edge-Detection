#include <gtest/gtest.h>

#include "image.hpp"
#include "sobel.hpp"

TEST(Sobel, UniformImage)
{
    Image img(32,32);

    for(auto& p : img.data)
        p = 100;

    Gradient g = sobel(img);

    for(int y=1;y<img.height-1;y++)
    {
        for(int x=1;x<img.width-1;x++)
        {
            int i = y * img.width + x;

            EXPECT_EQ(g.gx[i],0);
            EXPECT_EQ(g.gy[i],0);
        }
    }
}

TEST(Sobel, VerticalEdge)
{
    Image img(32,32);

    for(int y=0;y<32;y++)
    {
        for(int x=16;x<32;x++)
            img.at(x,y)=255;
    }

    Gradient g=sobel(img);

    int idx = 16*32 + 16;

    EXPECT_GT(std::abs(g.gx[idx]),100);
    EXPECT_LT(std::abs(g.gy[idx]),20);
}

TEST(Sobel, HorizontalEdge)
{
    Image img(32,32);

    for(int y=16;y<32;y++)
    {
        for(int x=0;x<32;x++)
            img.at(x,y)=255;
    }

    Gradient g=sobel(img);

    int idx = 16*32 + 16;

    EXPECT_GT(std::abs(g.gy[idx]),100);
    EXPECT_LT(std::abs(g.gx[idx]),20);
}