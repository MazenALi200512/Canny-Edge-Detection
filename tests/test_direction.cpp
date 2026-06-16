#include <gtest/gtest.h>
#include "image.hpp"
#include "direction.hpp"

// Pure vertical gradient (Gx>0, Gy=0): angle = 0° → sector 0
TEST(Direction, VerticalEdge)
{
    Gradient g(1, 1);
    g.gx[0] = 100;
    g.gy[0] = 0;

    Image d = gradientDirection(g);
    EXPECT_EQ(d.at(0, 0), 0);
}

// Pure horizontal gradient (Gx=0, Gy>0): angle = 90° → sector 90
TEST(Direction, HorizontalEdge)
{
    Gradient g(1, 1);
    g.gx[0] = 0;
    g.gy[0] = 100;

    Image d = gradientDirection(g);
    EXPECT_EQ(d.at(0, 0), 90);
}

// Diagonal gradient same sign (Gx>0, Gy>0): angle = 45° → sector 45
TEST(Direction, DiagonalEdge45)
{
    Gradient g(1, 1);
    g.gx[0] = 100;
    g.gy[0] = 100;

    Image d = gradientDirection(g);
    EXPECT_EQ(d.at(0, 0), 45);
}

// Diagonal gradient opposite sign (Gx>0, Gy<0): angle = 135° → sector 135
TEST(Direction, DiagonalEdge135)
{
    Gradient g(1, 1);
    g.gx[0] = 100;
    g.gy[0] = -100;

    Image d = gradientDirection(g);
    EXPECT_EQ(d.at(0, 0), 135);
}

// Steep gradient close to vertical (large Gy, small Gx): must be sector 90
TEST(Direction, NearVertical)
{
    Gradient g(1, 1);
    g.gx[0] = 10;
    g.gy[0] = 100;

    Image d = gradientDirection(g);
    EXPECT_EQ(d.at(0, 0), 90);
}

// Shallow gradient close to horizontal (large Gx, small Gy): must be sector 0
TEST(Direction, NearHorizontal)
{
    Gradient g(1, 1);
    g.gx[0] = 100;
    g.gy[0] = 10;

    Image d = gradientDirection(g);
    EXPECT_EQ(d.at(0, 0), 0);
}