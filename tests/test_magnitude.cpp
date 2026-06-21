#include <gtest/gtest.h>
#include <cmath>
#include "image.hpp"
#include "magnitude.hpp"

TEST(Magnitude, L1AndL2NonZero)
{
    Gradient g(8, 8);
    for(size_t i = 0; i < g.gx.size(); i++)
    {
        g.gx[i] = 50;
        g.gy[i] = 20;
    }
    Image l1(8,8);
    Image l2(8,8);
    magnitudeL1_inplace(g,l1);
    magnitudeL2_inplace(g,l2);
    EXPECT_GT(l1.at(0, 0), 0);
    EXPECT_GT(l2.at(0, 0), 0);
}

TEST(Magnitude, ZeroGradient)
{
    Gradient g(100, 75);
    Image l1(100,75);
    Image l2(100,75);
    magnitudeL1_inplace(g,l1);
    magnitudeL2_inplace(g,l2);
    for(int i = 0; i < l1.width * l1.height; i++)
        EXPECT_EQ(l1.data[i], 0);
    for(int i = 0; i < l2.width * l2.height; i++)
        EXPECT_EQ(l2.data[i], 0);
}

TEST(Magnitude, Saturation)
{
    Gradient g(1, 1);
    g.gx[0] = 1000;
    g.gy[0] = 1000;
    Image l1(1,1);
    Image l2(1,1);
    magnitudeL1_inplace(g,l1);
    magnitudeL2_inplace(g,l2);
    EXPECT_EQ(l1.at(0, 0), 255);
    EXPECT_EQ(l2.at(0, 0), 255);
}
