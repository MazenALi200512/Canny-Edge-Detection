#include <iostream>
#include <cmath>
#include <chrono>
#include <filesystem>

#include "image.hpp"
#include "gaussian.hpp"
#include "sobel.hpp"
#include "magnitude.hpp"
#include "direction.hpp"


int main()
{
    // std::cout<< std::filesystem::current_path()<< std::endl;

    // Image img = loadRawImage("images/input/horizontal.raw", 1000, 1000);
    Image img(1000,1000);
    for(int y = 0; y < 1000; y++)
    {
        for(int x = 0; x < 1000; x++)
        {
            if(y < 500)
                img.at(x,y) = 0;
            else
                img.at(x,y) = 255;
        }
    }
    // Image img = loadRawImage("/mnt/d/Projects/Project_Embedded/project/images/input/horizontal.raw", 1000, 1000);

    auto start = std::chrono::high_resolution_clock::now();
    Image blur = gaussianBlur(img);
    saveRawImage("images/output/blur.raw", blur);
    // saveRawImage("/mnt/d/Projects/Project_Embedded/project/images/output/blur_rv.raw", blur);

    Gradient grad = sobel(blur);
    Image gxImg(1000, 1000);
    Image gyImg(1000, 1000);
    for(int y = 0; y < 1000; y++)
    {
        for(int x = 0; x < 1000; x++)
        {
            int idx = y * 1000 + x;
            int gx = std::abs(grad.gx[idx]);
            int gy = std::abs(grad.gy[idx]);
            if(gx > 255) gx = 255;
            if(gy > 255) gy = 255;
            gxImg.at(x,y) = static_cast<uint8_t>(gx);
            gyImg.at(x,y) = static_cast<uint8_t>(gy);
        }
    }

    Image magL1 = magnitudeL1(grad);

    Image magRVV = magnitudeL1_rvv(grad);
    int mismatches = 0;
    for(size_t i = 0; i < magL1.data.size(); i++)
    {
        if(magL1.data[i] != magRVV.data[i])
            mismatches++;
    }
    std::cout<< "Magnitude RVV mismatches = "<< mismatches << std::endl;

    Image magL2 = magnitudeL2(grad);

    Image dir = gradientDirection(grad);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout<< "Pipeline Time = "<< duration.count()<< " us"<< std::endl;

    std::cout<< "Center Gx = "<< grad.gx[500 * 1000 + 500]<< std::endl;
    std::cout<< "Center Gy = "<< grad.gy[500 * 1000 + 500]<< std::endl;
    std::cout<< "Center MagL1 = "<< (int)magL1.at(500,500)<< std::endl;
    std::cout<< "Center MagL2 = "<< (int)magL2.at(500,500)<< std::endl;
    std::cout<< "Center Direction = "<< (int)dir.at(500,500)<< std::endl;

    return 0;
}