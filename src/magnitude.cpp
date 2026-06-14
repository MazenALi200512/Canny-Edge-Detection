#include "magnitude.hpp"
#include <cmath>
#include <cstdint>

Image magnitudeL1(const Gradient& grad)
{
    Image output(grad.width, grad.height);
    for(int y = 0; y < grad.height; y++)
    {
        for(int x = 0; x < grad.width; x++)
        {
            int index = y * grad.width + x;
            int mag = std::abs(grad.gx[index]) + std::abs(grad.gy[index]);
            if(mag > 255)
                mag = 255;
            output.at(x,y) = static_cast<uint8_t>(mag);
        }
    }
    return output;
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
