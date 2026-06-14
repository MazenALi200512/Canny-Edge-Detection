#include "image.hpp"

#include <fstream>
#include <stdexcept>

Image loadRawImage(
    const std::string& path,
    int width,
    int height)
{
    Image img(width, height);

    std::ifstream file(path, std::ios::binary);

    if (!file)
        throw std::runtime_error("Cannot open image");

    file.read(
        reinterpret_cast<char*>(img.data.data()),
        width * height);

    return img;
}

void saveRawImage(
    const std::string& path,
    const Image& img)
{
    std::ofstream file(path, std::ios::binary);

    file.write(
        reinterpret_cast<const char*>(img.data.data()),
        img.data.size());
}