#include <gtest/gtest.h>

#include "image.hpp"
#include "direction.hpp"

TEST(Direction, HorizontalGradient)
{
    Gradient g(1,1);

    g.gx[0] = 0;
    g.gy[0] = 100;

    Image d = gradientDirection(g);

    EXPECT_EQ(d.at(0,0),90);
}