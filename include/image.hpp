#pragma once

#include <vector>
#include <cstdint>
#include <string>

struct Image
{
    int width;
    int height;

    std::vector<uint8_t> data;

    Image(int w, int h)
        : width(w), height(h), data(w*h)
    {
    }

    uint8_t& at(int x, int y)
    {
        return data[y * width + x];
    }

    const uint8_t& at(int x, int y) const
    {
        return data[y * width + x];
    }
};

Image loadRawImage(
    const std::string& path,
    int width,
    int height);

void saveRawImage(
    const std::string& path,
    const Image& img);