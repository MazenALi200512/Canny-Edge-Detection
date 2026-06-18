#include <iostream>
#include <cmath>
#include <chrono>

#include "image.hpp"
#include "gaussian.hpp"
#include "sobel.hpp"
#include "magnitude.hpp"
#include "direction.hpp"

using Clock     = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::time_point<Clock>;

static long long us(TimePoint a, TimePoint b)
{
    return std::chrono::duration_cast<std::chrono::microseconds>(b - a).count();
}

int main()
{
    Image img(100, 100);
    for(int y = 0; y < 100; y++)
        for(int x = 0; x < 100; x++)
            img.at(x, y) = (y < 50) ? 0 : 255;

    // --- Gaussian blur (scalar,seperable,rvv) ---
    /*auto t0  = Clock::now();
    Image blur = gaussianBlur(img);
    auto t1  = Clock::now();
    long long t_gauss_scalar = us(t0, t1);*/

    Image blurSep(img.width, img.height);
    auto ts0 = Clock::now();
    for(int i = 0; i < 100; i++)
        blurSep = gaussianBlurSeparable(img);
    auto ts1 = Clock::now();
    long long t_gauss_sep = us(ts0, ts1)/100;

    /*auto t2 = Clock::now();
    Image blurRVV = gaussianBlur_rvv(img);
    auto t3 = Clock::now();
    long long t_gauss_rvv = us(t2, t3);*/

    Image blurRVV(img.width, img.height);
    auto t2 = Clock::now();
    for(int i = 0; i < 100; i++)
        blurRVV = gaussianBlur_rvv(img);
    auto t3 = Clock::now();
    long long t_gauss_rvv = us(t2, t3)/100;

    // Correctness check
    /*int gaussMismatch_scalar_sep = 0;
    for(size_t i = 0; i < static_cast<size_t>(blur.width) * blur.height; i++)
        if(blur.data[i] != blurSep.data[i]) gaussMismatch_scalar_sep++;*/
    int gaussMismatch_scalar_rvv = 0;
    for(size_t i = 0; i < static_cast<size_t>(blurSep.width) * blurSep.height; i++)
        if(blurSep.data[i] != blurRVV.data[i]) gaussMismatch_scalar_rvv++;

    Gradient grad(blurSep.width, blurSep.height);
    auto t4 = Clock::now();
    for(int i=0;i<100;i++)
        grad = sobel(blurSep);
    auto t5 = Clock::now();
    long long t_sobel = us(t4,t5)/100;

    Image magL1(grad.width, grad.height);
    auto t6 = Clock::now();
    for(int i=0;i<100;i++)
        magnitudeL1_inplace(grad, magL1);
    auto t7 = Clock::now();
    long long t_mag_l1 = us(t6, t7)/100;

    // --- Magnitude L2 ---
    /*auto t10  = Clock::now();
    Image magL2 = magnitudeL2(grad);
    auto t11  = Clock::now();
    long long t_mag_l2 = us(t10, t11);*/

    Image magL2(grad.width, grad.height);
    auto t10  = Clock::now();
    for(int i=0;i<100;i++)
        magnitudeL2_inplace(grad, magL2);
    auto t11  = Clock::now();
    long long t_mag_l2 = us(t10, t11)/100;

    // --- Direction ---
    /*auto t12 = Clock::now();
    Image dir = gradientDirection(grad);
    auto t13 = Clock::now();
    long long t_dir = us(t12, t13);*/

    Image dir(grad.width, grad.height);
    auto t12 = Clock::now();
    for(int i=0;i<100;i++)
        dir = gradientDirection(grad);
    auto t13 = Clock::now();
    long long t_dir = us(t12, t13)/100;

    // -------------------------------------------------------------------------
    // Per-stage timing report
    // -------------------------------------------------------------------------
    long long t_total = t_gauss_sep + t_sobel + t_mag_l1 + t_mag_l2 + t_dir;
    auto pct = [&](long long t) -> double {return 100.0 * t / t_total;};

    std::cout << "\n=== Per-Stage Timing (scalar pipeline) ===\n";
    std::cout << "Gaussian blur   : " << t_gauss_sep    << " us  ("<< pct(t_gauss_sep) << "%)\n";
    std::cout << "Sobel Gx/Gy     : " << t_sobel        << " us  ("<< pct(t_sobel)        << "%)\n";
    std::cout << "Magnitude L1    : " << t_mag_l1       << " us  ("<< pct(t_mag_l1)   << "%)\n";
    std::cout << "Magnitude L2    : " << t_mag_l2       << " us  ("<< pct(t_mag_l2)   << "%)\n";
    std::cout << "Direction       : " << t_dir          << " us  ("<< pct(t_dir)          << "%)\n";
    std::cout << "Total (scalar)  : " << t_total        << " us\n\n";

    std::cout << "\n=== RVV vs Scalar ===\n";
    // std::cout << "Gaussian scalar : " << t_gauss_scalar << " us\n";
    std::cout << "Gaussian Separable : " << t_gauss_sep << " us\n";
    std::cout << "Gaussian RVV    : " << t_gauss_rvv    << " us\n";
    // std::cout << "Gaussian_Scalar_Seperable mismatches: " << gaussMismatch_scalar_sep << "\n";
    std::cout << "Gaussian_Scalar_RVV mismatches: " << gaussMismatch_scalar_rvv << "\n\n";

    std::cout << "\n=== Sanity Checks ===\n";
    std::cout << "Center Gx        : " << grad.gx[img.height/2 * img.width + img.width/2] << "\n";
    std::cout << "Center Gy        : " << grad.gy[img.height/2 * img.width + img.width/2] << "\n";
    std::cout << "Center MagL1     : " << (int)magL1.at(img.width/2, img.height/2)   << "\n";
    std::cout << "Center MagL2     : " << (int)magL2.at(img.width/2, img.height/2)   << "\n";
    std::cout << "Center Direction : " << (int)dir.at(img.width/2, img.height/2)     << "\n";

    /*saveRawImage("images/output/blur.raw", blur);
    saveRawImage("images/output/blursep.raw", blurSep);*/

    return 0;
}