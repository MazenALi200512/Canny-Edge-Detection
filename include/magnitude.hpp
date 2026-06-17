#pragma once
#include "image.hpp"
#include "sobel.hpp"

/*Image magnitudeL1(const Gradient& grad);
Image magnitudeL2(const Gradient& grad);*/

// Image magnitudeL1_rvv(const Gradient& grad);

void magnitudeL1_inplace    (const Gradient& grad, Image& out);
void magnitudeL2_inplace    (const Gradient& grad, Image& out);
// void magnitudeL1_rvv_inplace(const Gradient& grad, Image& out);