#include <gtest/gtest.h>

#include "image.hpp"
#include "gaussian.hpp"

TEST(Gaussian, UniformImage)
{
    Image img(32,32);

    for(auto& p : img.data)
        p = 128;

    Image out = gaussianBlur(img);

    for(int y=2;y<30;y++)
    {
        for(int x=2;x<30;x++)
        {
            EXPECT_NEAR(out.at(x,y),128,1);
        }
    }
}

TEST(Gaussian, BlackImage)
{
    Image img(32,32);

    Image out = gaussianBlur(img);

    for(auto p : out.data)
        EXPECT_EQ(p,0);
}

TEST(Gaussian, ImpulseResponse)
{
    Image img(21,21);

    img.at(10,10)=255;

    Image out = gaussianBlur(img);

    EXPECT_GT(out.at(10,10),0);

    EXPECT_EQ(
        out.at(9,10),
        out.at(11,10));
}