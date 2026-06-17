#pragma once

#include <cstdint>
#include <cstdlib>
#include <string>

struct Image
{
    int width;
    int height;
    uint8_t* data;

    Image(int w, int h) : width(w),height(h)
    {
        size_t bytes = static_cast<size_t>(w) * h;
        size_t aligned_size = ((bytes + 63) / 64) * 64;
        data = static_cast<uint8_t*>(aligned_alloc(64, aligned_size));
    }

    ~Image()
    {
        free(data);
    }

    uint8_t& at(int x, int y)
    {
        return data[y * width + x];
    }

    const uint8_t& at(int x, int y) const
    {
        return data[y * width + x];
    }

    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;

    Image(Image&& other) noexcept : width(other.width), height(other.height), data(other.data)
    {
        other.data = nullptr;
    }

    Image& operator=(Image&& other) noexcept
    {
        if(this != &other)
        {
            free(data);
            width  = other.width;
            height = other.height;
            data   = other.data;
            other.data = nullptr;
        }
        return *this;
    }
};

Image loadRawImage(const std::string& path, int width, int height);
void saveRawImage(const std::string& path, const Image& img);