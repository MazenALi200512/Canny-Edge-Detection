#include "gaussian.hpp"
#include "convolution.hpp"

static const int16_t GAUSS_KERNEL[25] =
{
    1,  4,  7,  4,  1,
    4, 16, 26, 16,  4,
    7, 26, 41, 26,  7,
    4, 16, 26, 16,  4,
    1,  4,  7,  4,  1
};

static constexpr int32_t KERNEL_SUM = 273;

Image gaussianBlur(const Image& input)
{
    return convolve2D<uint8_t, int32_t, int16_t>(
        input, GAUSS_KERNEL, 5, 5, KERNEL_SUM);
}