#include "direction.hpp"
#include <cstdlib>

Image gradientDirection(const Gradient& grad)
{
    Image output(grad.width, grad.height);

    for(int y = 0; y < grad.height; y++)
    {
        for(int x = 0; x < grad.width; x++)
        {
            int index = y * grad.width + x;
            int gx = grad.gx[index];
            int gy = grad.gy[index];
            int ax = std::abs(gx);
            int ay = std::abs(gy);

            uint8_t quantized;

            if(ay * 5 < ax * 2)
            {
                // Angle below 22.5° or above 157.5° → horizontal gradient
                quantized = 0;
            }
            else if(ay * 5 < ax * 12)
            {
                // Angle in [22.5°, 67.5°) or (112.5°, 157.5°]
                // Sign of gx*gy distinguishes the two diagonal sectors:
                //   same sign  (gx*gy >= 0) → 45°
                //   diff sign  (gx*gy <  0) → 135°
                quantized = (gx * gy >= 0) ? 45 : 135;
            }
            else
            {
                // Angle in [67.5°, 112.5°] → vertical gradient
                quantized = 90;
            }

            output.at(x, y) = quantized;
        }
    }

    return output;
}