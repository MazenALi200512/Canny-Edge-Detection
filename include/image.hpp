#pragma once

#include <vector>
#include <string>
#include <cstdint>

std::vector<uint8_t> loadRawImage(
    const std::string& path,
    int width,
    int height);

void saveRawImage(
    const std::string& path,
    const std::vector<uint8_t>& image);