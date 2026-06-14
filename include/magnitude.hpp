#pragma once
#include "image.hpp"
#include "sobel.hpp"

Image magnitudeL1(const Gradient& grad);
Image magnitudeL2(const Gradient& grad);
