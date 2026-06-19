#pragma once
#include "image.hpp"
#include "sobel.hpp"

void magnitudeL1_inplace (const Gradient& grad, Image& out);
void magnitudeL2_inplace (const Gradient& grad, Image& out);