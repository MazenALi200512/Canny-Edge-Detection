#include "sobel.hpp"
static const int16_t SOBEL_X[3][3] =
{
    {-1, 0, 1},
    {-2, 0, 2},
    {-1, 0, 1}
};
static const int16_t SOBEL_Y[3][3] =
{
    {-1,-2,-1},
    { 0, 0, 0},
    { 1, 2, 1}
};
Gradient sobel(const Image& input)
{
    Gradient grad(input.width, input.height);
    
    for(int y = 0; y < input.height; y++)
    {
        for(int x = 0; x < input.width; x++)
        {
            int32_t gx = 0;
            int32_t gy = 0;
            for(int ky = -1; ky <= 1; ky++)
            {
                for(int kx = -1; kx <= 1; kx++)
                {
                    int nx = x + kx;
                    int ny = y + ky;

                    if(nx < 0 || nx >= input.width)
                        continue;
                    if(ny < 0 || ny >= input.height)
                        continue;

                    uint8_t pixel = input.at(nx, ny);
                    gx += pixel * SOBEL_X[ky + 1][kx + 1];
                    gy += pixel * SOBEL_Y[ky + 1][kx + 1];
                }
            }
            int index = y * input.width + x;
            grad.gx[index] = static_cast<int16_t>(gx);
            grad.gy[index] = static_cast<int16_t>(gy);
        }
    }
    return grad;
}