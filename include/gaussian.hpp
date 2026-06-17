#pragma once

#include "image.hpp"

Image gaussianBlur(const Image& input);
Image gaussianBlurSeparable(const Image& input);
Image gaussianBlur_rvv(const Image& input);