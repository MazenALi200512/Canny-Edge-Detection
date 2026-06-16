#include "magnitude.hpp"
#include <cmath>
#include <cstdint>

// Internal kernel — writes L1 magnitude into an already-allocated buffer.
// Called by both the returning wrapper and the inplace benchmark overload.
static void l1_kernel(const Gradient& grad, Image& output)
{
    int total = grad.width * grad.height;
    for(int i = 0; i < total; i++)
    {
        int mag = std::abs(grad.gx[i]) + std::abs(grad.gy[i]);
        if(mag > 255) mag = 255;
        output.data[i] = static_cast<uint8_t>(mag);
    }
}

Image magnitudeL1(const Gradient& grad)
{
    Image output(grad.width, grad.height);
    l1_kernel(grad, output);
    return output;
}

void magnitudeL1_inplace(const Gradient& grad, Image& out)
{
    l1_kernel(grad, out);
}

Image magnitudeL2(const Gradient& grad)
{
    Image output(grad.width, grad.height);
    for(int y = 0; y < grad.height; y++)
    {
        for(int x = 0; x < grad.width; x++)
        {
            int index = y * grad.width + x;
            double gx = grad.gx[index];
            double gy = grad.gy[index];
            double mag = std::sqrt(gx * gx + gy * gy);
            if(mag > 255.0)
                mag = 255.0;
            output.at(x,y) = static_cast<uint8_t>(mag);
        }
    }
    return output;
}