#include <iostream>
#include <cmath>
#include "image.hpp"
#include "gaussian.hpp"
#include "sobel.hpp"
#include "magnitude.hpp"
#include "direction.hpp"

int main()
{
    Image img = loadRawImage("images/input/horizontal.raw", 100, 100);
    Image blur = gaussianBlur(img);
    saveRawImage("images/output/blur.raw", blur);
    Gradient grad = sobel(blur);
    Image gxImg(100, 100);
    Image gyImg(100, 100);
    for(int y = 0; y < 100; y++)
    {
        for(int x = 0; x < 100; x++)
        {
            int idx = y * 100 + x;

            int gx = std::abs(grad.gx[idx]);
            int gy = std::abs(grad.gy[idx]);

            if(gx > 255) gx = 255;
            if(gy > 255) gy = 255;

            gxImg.at(x,y) = static_cast<uint8_t>(gx);
            gyImg.at(x,y) = static_cast<uint8_t>(gy);
        }
    }
    saveRawImage("images/output/gx.raw",gxImg);
    saveRawImage("images/output/gy.raw",gyImg);

    std::cout<< "Center Gx = "<< grad.gx[50 * 100 + 50]<< std::endl;
    std::cout<< "Center Gy = "<< grad.gy[50 * 100 + 50]<< std::endl;

    Gradient grad = sobel(blur);

    Image gxImg(100, 100);
    Image gyImg(100, 100);
    for(int y = 0; y < 100; y++)
    {
        for(int x = 0; x < 100; x++)
        {
            int idx = y * 100 + x;
            int gx = std::abs(grad.gx[idx]);
            int gy = std::abs(grad.gy[idx]);
            if(gx > 255) gx = 255;
            if(gy > 255) gy = 255;
            gxImg.at(x,y) = static_cast<uint8_t>(gx);
            gyImg.at(x,y) = static_cast<uint8_t>(gy);
        }
    }
    saveRawImage("images/output/gx.raw",gxImg);
    saveRawImage("images/output/gy.raw",gyImg);

    Image magL1 = magnitudeL1(grad);
    Image magL2 = magnitudeL2(grad);
    saveRawImage("images/output/mag_l1.raw", magL1);
    saveRawImage("images/output/mag_l2.raw", magL2);

    Image dir = gradientDirection(grad);
    saveRawImage("images/output/direction.raw", dir);

    std::cout<< "Center Gx = "<< grad.gx[50 * 100 + 50]<< std::endl;
    std::cout<< "Center Gy = "<< grad.gy[50 * 100 + 50]<< std::endl;
    std::cout<< "Center MagL1 = "<< (int)magL1.at(50,50)<< std::endl;
    std::cout<< "Center MagL2 = "<< (int)magL2.at(50,50)<< std::endl;
    std::cout<< "Center Direction = "<< (int)dir.at(50,50)<< std::endl;

    return 0;
}
