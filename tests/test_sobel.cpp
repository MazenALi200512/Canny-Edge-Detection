#include <gtest/gtest.h>
#include <cstdlib>
#include "image.hpp"
#include "sobel.hpp"

// Uniform image must produce zero gradient everywhere (interior)
// Non-power-of-two size: 100x75
TEST(Sobel, UniformImage)
{
    Image img(100, 75);
    for(auto& p : img.data) p = 100;

    Gradient g = sobel(img);

    for(int y = 1; y < 74; y++)
        for(int x = 1; x < 99; x++)
        {
            int i = y * 100 + x;
            EXPECT_EQ(g.gx[i], 0);
            EXPECT_EQ(g.gy[i], 0);
        }
}

// Vertical edge (left=black, right=white): large |Gx|, small |Gy|
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

// Horizontal edge (top=black, bottom=white): large |Gy|, small |Gx|
TEST(Sobel, HorizontalEdge)
{
    Image img(32, 32);
    for(int y = 16; y < 32; y++)
        for(int x = 0; x < 32; x++)
            img.at(x, y) = 255;

    Gradient g = sobel(img);

    int idx = 16 * 32 + 16;
    EXPECT_GT(std::abs(g.gy[idx]), 100);
    EXPECT_LT(std::abs(g.gx[idx]), 20);
}

// Diagonal edge (top-left=black, bottom-right=white):
// both Gx and Gy must be significant at the edge pixels
TEST(Sobel, DiagonalEdge)
{
    Image img(32, 32);

    // Fill bottom-right triangle with white
    for(int y = 0; y < 32; y++)
        for(int x = 0; x < 32; x++)
            if(x + y >= 32)
                img.at(x, y) = 255;

    Gradient g = sobel(img);

    // Sample a pixel on the diagonal edge, away from image border
    int idx = 15 * 32 + 15;
    EXPECT_GT(std::abs(g.gx[idx]), 50);
    EXPECT_GT(std::abs(g.gy[idx]), 50);
}