#include "magnitude.hpp"

#include <cmath>
#include <cstdint>
#include <algorithm>

static void l1_kernel(const Gradient& grad, Image& output)
{
    int total = grad.width * grad.height;
    int maxMag = 0;

    // Pass 1: find maximum magnitude
    for(int i = 0; i < total; i++)
    {
        int mag = std::abs(grad.gx[i]) + std::abs(grad.gy[i]);
        if(mag > maxMag)
            maxMag = mag;
    }

    if(maxMag == 0)
        maxMag = 1;

    // Pass 2: normalize to [0,255]
    for(int i = 0; i < total; i++)
    {
        int mag = std::abs(grad.gx[i]) + std::abs(grad.gy[i]);
        int norm = (mag * 255) / maxMag;
        output.data[i] = static_cast<uint8_t>(norm);
    }
}

/*Image magnitudeL1(const Gradient& grad)
{
    Image output(grad.width, grad.height);
    l1_kernel(grad, output);
    return output;
}*/

void magnitudeL1_inplace(const Gradient& grad, Image& out)
{
    l1_kernel(grad, out);
}

/*Image magnitudeL2(const Gradient& grad)
{
    Image output(grad.width, grad.height);
    int total = grad.width * grad.height;
    double maxMag = 0.0;

    // Pass 1: find maximum magnitude
    for(int i = 0; i < total; i++)
    {
        double gx = grad.gx[i];
        double gy = grad.gy[i];
        double mag = std::sqrt(gx * gx + gy * gy);
        if(mag > maxMag)
            maxMag = mag;
    }

    if(maxMag == 0.0)
        maxMag = 1.0;

    // Pass 2: normalize to [0,255]
    for(int i = 0; i < total; i++)
    {
        double gx = grad.gx[i];
        double gy = grad.gy[i];
        double mag = std::sqrt(gx * gx + gy * gy);
        int norm = static_cast<int>((mag * 255.0) / maxMag);
        output.data[i] = static_cast<uint8_t>(norm);
    }

    return output;
}*/

void magnitudeL2_inplace(const Gradient& grad, Image& out)
{
    int total = grad.width * grad.height;
    double maxMag = 0.0;

    // Pass 1: find maximum magnitude
    for(int i = 0; i < total; i++)
    {
        double gx = grad.gx[i];
        double gy = grad.gy[i];
        double mag = std::sqrt(gx * gx + gy * gy);

        if(mag > maxMag)
            maxMag = mag;
    }

    if(maxMag == 0.0)
        maxMag = 1.0;

    // Pass 2: normalize to [0,255]
    for(int i = 0; i < total; i++)
    {
        double gx = grad.gx[i];
        double gy = grad.gy[i];
        double mag = std::sqrt(gx * gx + gy * gy);

        int norm = static_cast<int>((mag * 255.0) / maxMag);
        out.data[i] = static_cast<uint8_t>(norm);
    }
}