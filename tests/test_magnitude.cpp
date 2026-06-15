#include <gtest/gtest.h>

#include "image.hpp"
#include "magnitude.hpp"

TEST(Magnitude, L1AndL2NonZero)
{
    Gradient g(8,8);

    for(size_t i=0;i<g.gx.size();i++)
    {
        g.gx[i]=50;
        g.gy[i]=20;
    }

    Image l1 = magnitudeL1(g);
    Image l2 = magnitudeL2(g);

    EXPECT_GT(l1.at(0,0),0);
    EXPECT_GT(l2.at(0,0),0);
}

TEST(Magnitude, L1GreaterThanL2)
{
    Gradient g(1,1);

    g.gx[0]=100;
    g.gy[0]=100;

    Image l1 = magnitudeL1(g);
    Image l2 = magnitudeL2(g);

    EXPECT_GE(l1.at(0,0), l2.at(0,0));
}