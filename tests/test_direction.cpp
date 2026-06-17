#include <gtest/gtest.h>
#include "image.hpp"
#include "direction.hpp"

TEST(Direction, VerticalEdge)
{
    Gradient g(1, 1);
    g.gx[0] = 100;
    g.gy[0] = 0;
    Image d = gradientDirection(g);
    EXPECT_EQ(d.at(0, 0), 0);
}

TEST(Direction, HorizontalEdge)
{
    Gradient g(1, 1);
    g.gx[0] = 0;
    g.gy[0] = 100;
    Image d = gradientDirection(g);
    EXPECT_EQ(d.at(0, 0), 90);
}

TEST(Direction, DiagonalEdge45)
{
    Gradient g(1, 1);
    g.gx[0] = 100;
    g.gy[0] = 100;
    Image d = gradientDirection(g);
    EXPECT_EQ(d.at(0, 0), 45);
}

TEST(Direction, DiagonalEdge135)
{
    Gradient g(1, 1);
    g.gx[0] = 100;
    g.gy[0] = -100;
    Image d = gradientDirection(g);
    EXPECT_EQ(d.at(0, 0), 135);
}

TEST(Direction, NearVertical)
{
    Gradient g(1, 1);
    g.gx[0] = 10;
    g.gy[0] = 100;
    Image d = gradientDirection(g);
    EXPECT_EQ(d.at(0, 0), 90);
}

TEST(Direction, NearHorizontal)
{
    Gradient g(1, 1);
    g.gx[0] = 100;
    g.gy[0] = 10;
    Image d = gradientDirection(g);
    EXPECT_EQ(d.at(0, 0), 0);
}