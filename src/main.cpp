#include <iostream>
#include "image.hpp"
#include "gaussian.hpp"

int main()
{
    Image img = loadRawImage("images/input/horizontal.raw", 100, 100);
    Image blur = gaussianBlur(img);
    saveRawImage("images/output/blur.raw", blur);

    return 0;
}