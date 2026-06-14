#include "direction.hpp"
#include <cmath>
#include <cstdint>

Image gradientDirection(const Gradient& grad)
{
    Image output(grad.width, grad.height);
    for(int y = 0; y < grad.height; y++)
    {
        for(int x = 0; x < grad.width; x++)
        {
            int index = y * grad.width + x;
            double gx = grad.gx[index];
            double gy = grad.gy[index];
            double angle = std::atan2(gy, gx) * 180.0 / M_PI;
            if(angle < 0)
                angle += 180.0;

            uint8_t quantized;
            if(angle < 22.5 || angle >= 157.5)
                quantized = 0;
            else if(angle < 67.5)
                quantized = 45;
            else if(angle < 112.5)
                quantized = 90;
            else
                quantized = 135;

            output.at(x,y) = quantized;
        }
    }
    return output;
}
