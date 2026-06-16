#include <gtest/gtest.h>
#include "image.hpp"
#include "gaussian.hpp"

// Uniform image: interior pixels must stay at 128 (±1 for integer rounding)
// Uses non-power-of-two size (100x75) to exercise strip-mining tail cases
TEST(Gaussian, UniformImage)
{
    Image img(100, 75);
    for(auto& p : img.data) p = 128;

    Image out = gaussianBlur(img);

    for(int y = 2; y < 73; y++)
        for(int x = 2; x < 98; x++)
            EXPECT_NEAR(out.at(x, y), 128, 1);
}

// All-black image must produce all-black output
TEST(Gaussian, BlackImage)
{
    Image img(100, 75);
    Image out = gaussianBlur(img);
    for(auto p : out.data) EXPECT_EQ(p, 0);
}

// Impulse response: single bright pixel must spread symmetrically
// Image size 21x21 (non-power-of-two in both dimensions)
TEST(Gaussian, ImpulseResponse)
{
    Image img(21, 21);
    img.at(10, 10) = 255;

    Image out = gaussianBlur(img);

    // Center must be nonzero
    EXPECT_GT(out.at(10, 10), 0);

    // Must be horizontally symmetric around center
    EXPECT_EQ(out.at(9, 10), out.at(11, 10));

    // Must be vertically symmetric around center
    EXPECT_EQ(out.at(10, 9), out.at(10, 11));

    // Must be diagonally symmetric
    EXPECT_EQ(out.at(9, 9), out.at(11, 11));
}

// Blurring must reduce peak value (spread energy outward)
TEST(Gaussian, ReducesPeak)
{
    Image img(21, 21);
    img.at(10, 10) = 255;

    Image out = gaussianBlur(img);

    EXPECT_LT(out.at(10, 10), 255);
}