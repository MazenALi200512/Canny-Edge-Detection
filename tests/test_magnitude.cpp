#include <gtest/gtest.h>
#include <cmath>
#include "image.hpp"
#include "magnitude.hpp"

// Both L1 and L2 must produce nonzero output when gradients are nonzero
TEST(Magnitude, L1AndL2NonZero)
{
    Gradient g(8, 8);
    for(size_t i = 0; i < g.gx.size(); i++)
    {
        g.gx[i] = 50;
        g.gy[i] = 20;
    }

    Image l1 = magnitudeL1(g);
    Image l2 = magnitudeL2(g);

    EXPECT_GT(l1.at(0, 0), 0);
    EXPECT_GT(l2.at(0, 0), 0);
}

// L1 >= L2 always (L1 is an overestimate for diagonal directions)
// Worst case: Gx=Gy=100 → L1=200, L2=141
TEST(Magnitude, L1GreaterOrEqualL2)
{
    Gradient g(1, 1);
    g.gx[0] = 100;
    g.gy[0] = 100;

    Image l1 = magnitudeL1(g);
    Image l2 = magnitudeL2(g);

    EXPECT_GE(l1.at(0, 0), l2.at(0, 0));
}

// Pure horizontal gradient: L1 and L2 must agree (no diagonal component)
// Gx=100, Gy=0 → L1=100, L2=100
TEST(Magnitude, PureHorizontalGradientL1EqualsL2)
{
    Gradient g(1, 1);
    g.gx[0] = 100;
    g.gy[0] = 0;

    Image l1 = magnitudeL1(g);
    Image l2 = magnitudeL2(g);

    EXPECT_EQ(l1.at(0, 0), l2.at(0, 0));
}

// Zero gradient must produce zero magnitude
TEST(Magnitude, ZeroGradient)
{
    Gradient g(100, 75);   // non-power-of-two

    Image l1 = magnitudeL1(g);
    Image l2 = magnitudeL2(g);

    for(auto p : l1.data) EXPECT_EQ(p, 0);
    for(auto p : l2.data) EXPECT_EQ(p, 0);
}

// Saturation: very large gradient must clamp to 255, not overflow
TEST(Magnitude, Saturation)
{
    Gradient g(1, 1);
    g.gx[0] = 1000;
    g.gy[0] = 1000;

    Image l1 = magnitudeL1(g);
    Image l2 = magnitudeL2(g);

    EXPECT_EQ(l1.at(0, 0), 255);
    EXPECT_EQ(l2.at(0, 0), 255);
}