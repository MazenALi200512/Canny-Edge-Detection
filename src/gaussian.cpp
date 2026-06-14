#include "gaussian.hpp"

static const int kernel[5][5] =
{
    {1, 4, 7, 4, 1},
    {4,16,26,16,4},
    {7,26,41,26,7},
    {4,16,26,16,4},
    {1, 4, 7, 4, 1}
};

static constexpr int KERNEL_SUM = 273;

Image gaussianBlur(const Image& input)
{
    Image output(input.width, input.height);

    for(int y = 0; y < input.height; y++)
    {
        for(int x = 0; x < input.width; x++)
        {
            int32_t sum = 0;
            for(int ky = -2; ky <= 2; ky++)
            {
                for(int kx = -2; kx <= 2; kx++)
                {
                    int nx = x + kx;
                    int ny = y + ky;

                    if(nx < 0 || nx >= input.width)
                        continue;
                    if(ny < 0 || ny >= input.height)
                        continue;

                    uint8_t pixel = input.at(nx, ny);
                    int weight = kernel[ky + 2][kx + 2];

                    sum += pixel * weight;
                }
            }
            sum /= KERNEL_SUM;

            if(sum < 0)
                sum = 0;
            if(sum > 255)
                sum = 255;

            output.at(x,y) = static_cast<uint8_t>(sum);
        }
    }

    return output;
}