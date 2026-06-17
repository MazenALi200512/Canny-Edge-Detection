#include "gaussian.hpp"
#include "magnitude.hpp"
#include "sobel.hpp"

Image gaussianBlur_rvv(const Image& input)
{
    return gaussianBlur(input);
}

Gradient sobel_rvv(const Image& input)
{
    return sobel(input);
}

Image magnitudeL1_rvv(const Gradient& grad)
{
    return magnitudeL1(grad);
}

void magnitudeL1_rvv_inplace(const Gradient& grad, Image& out)
{
    magnitudeL1_inplace(grad, out);
}