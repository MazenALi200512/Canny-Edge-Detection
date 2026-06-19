#pragma once

#include "image.hpp"
#include <vector>
#include <cstdint>

struct Gradient
{
    std::vector<int16_t> gx;
    std::vector<int16_t> gy;
    int width;
    int height;
    Gradient(int w, int h): gx(w*h), gy(w*h), width(w), height(h) {}
};

Gradient sobel(const Image& input);