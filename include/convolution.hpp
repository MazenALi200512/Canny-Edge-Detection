#pragma once

#include "image.hpp"
#include <cstdint>
#include <vector>
#include <algorithm>

/*template<typename PixelType, typename AccumType, typename KernelType>
Image convolve2D(
    const Image&       input,
    const KernelType*  kernel,    // flat [kH * kW], row-major
    int                kW,        // kernel width  (must be odd)
    int                kH,        // kernel height (must be odd)
    AccumType          kernelSum) // divisor after accumulation
{
    Image output(input.width, input.height);

    int halfW = kW / 2;
    int halfH = kH / 2;

    for(int y = 0; y < input.height; y++)
    {
        for(int x = 0; x < input.width; x++)
        {
            AccumType sum = 0;

            for(int ky = -halfH; ky <= halfH; ky++)
            {
                for(int kx = -halfW; kx <= halfW; kx++)
                {
                    int nx = x + kx;
                    int ny = y + ky;

                    // Zero-padding: skip out-of-bounds (contributes 0)
                    if(nx < 0 || nx >= input.width)  continue;
                    if(ny < 0 || ny >= input.height) continue;

                    int ki = (ky + halfH) * kW + (kx + halfW);
                    sum += static_cast<AccumType>(static_cast<PixelType>(input.at(nx, ny))) * static_cast<AccumType>(kernel[ki]);
                }
            }

            sum /= kernelSum;
            sum = std::max(sum, static_cast<AccumType>(0));
            sum = std::min(sum, static_cast<AccumType>(255));

            output.at(x, y) = static_cast<uint8_t>(sum);
        }
    }

    return output;
}*/

template<typename PixelType, typename AccumType, typename KernelType>
Image convolveSeparable(
    const Image&      input,
    const KernelType* kernel,
    int               kernelSize,
    AccumType         kernelSum)
{
    int radius = kernelSize / 2;
    Image temp(input.width, input.height);
    Image output(input.width, input.height);

    // Horizontal pass
    for(int y = 0; y < input.height; y++)
    {
        for(int x = 0; x < input.width; x++)
        {
            AccumType sum = 0;
            for(int k = -radius; k <= radius; k++)
            {
                int nx = x + k;
                if(nx < 0 || nx >= input.width)
                    continue;
                sum += static_cast<AccumType>(static_cast<PixelType>(input.at(nx, y))) * static_cast<AccumType>(kernel[k + radius]);
            }
            sum /= kernelSum;
            sum = std::max(sum, static_cast<AccumType>(0));
            sum = std::min(sum, static_cast<AccumType>(255));
            temp.at(x, y) = static_cast<uint8_t>(sum);
        }
    }

    // Vertical pass
    for(int y = 0; y < input.height; y++)
    {
        for(int x = 0; x < input.width; x++)
        {
            AccumType sum = 0;
            for(int k = -radius; k <= radius; k++)
            {
                int ny = y + k;
                if(ny < 0 || ny >= input.height)
                    continue;
                sum += static_cast<AccumType>(temp.at(x, ny)) * static_cast<AccumType>(kernel[k + radius]);
            }
            sum /= kernelSum;
            sum = std::max(sum, static_cast<AccumType>(0));
            sum = std::min(sum, static_cast<AccumType>(255));
            output.at(x, y) = static_cast<uint8_t>(sum);
        }
    }

    return output;
}