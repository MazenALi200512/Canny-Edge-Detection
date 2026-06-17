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
    Image img(1000, 1000);
    for(int y = 0; y < 1000; y++)
        for(int x = 0; x < 1000; x++)
            img.at(x, y) = (y < 500) ? 0 : 255;

    // --- Gaussian blur (scalar,seperable,rvv) ---
    auto t0  = Clock::now();
    Image blur = gaussianBlur(img);
    auto t1  = Clock::now();
    long long t_gauss_scalar = us(t0, t1);

    auto ts0 = Clock::now();
    Image blurSep = gaussianBlurSeparable(img);
    auto ts1 = Clock::now();
    long long t_gauss_sep = us(ts0, ts1);

    auto t2 = Clock::now();
    Image blurRVV = gaussianBlur_rvv(img);
    auto t3 = Clock::now();
    long long t_gauss_rvv = us(t2, t3);

    // Correctness check
    int gaussMismatch_scalar_sep = 0;
    for(size_t i = 0; i < static_cast<size_t>(blur.width) * blur.height; i++)
        if(blur.data[i] != blurSep.data[i]) gaussMismatch_scalar_sep++;
    int gaussMismatch_scalar_rvv = 0;
    for(size_t i = 0; i < static_cast<size_t>(blur.width) * blur.height; i++)
        if(blur.data[i] != blurRVV.data[i]) gaussMismatch_scalar_rvv++;

    // --- Sobel (scalar,rvv) ---
    auto t4 = Clock::now();
    Gradient grad = sobel(blur);
    auto t5 = Clock::now();
    long long t_sobel = us(t4, t5);

    auto t4rvv = Clock::now();
    Gradient gradRVV = sobel_rvv(blur);
    auto t5rvv = Clock::now();
    long long t_sobel_rvv = us(t4rvv, t5rvv);

    // Correctness check
    int sobelMismatch = 0;
    for(size_t i = 0; i < grad.gx.size(); i++)
    {
        if(grad.gx[i] != gradRVV.gx[i])
            sobelMismatch++;
        if(grad.gy[i] != gradRVV.gy[i])
            sobelMismatch++;
    }

    // --- Magnitude L1 (scalar,rvv)---
    Image magL1(grad.width, grad.height);
    auto t6 = Clock::now();
    magnitudeL1_inplace(grad, magL1);
    auto t7 = Clock::now();
    long long t_mag_scalar = us(t6, t7);

    Image magRVV(grad.width, grad.height);
    auto t8 = Clock::now();
    magnitudeL1_rvv_inplace(grad, magRVV);
    auto t9 = Clock::now();
    long long t_mag_rvv = us(t8, t9);

    int magMismatch = 0;
    for(size_t i = 0; i < static_cast<size_t>(magL1.width) * magL1.height; i++)
        if(magL1.data[i] != magRVV.data[i]) magMismatch++;

    // --- Magnitude L2 ---
    auto t10  = Clock::now();
    Image magL2 = magnitudeL2(grad);
    auto t11  = Clock::now();
    long long t_mag_l2 = us(t10, t11);

    // --- Direction ---
    auto t12 = Clock::now();
    Image dir = gradientDirection(grad);
    auto t13 = Clock::now();
    long long t_dir = us(t12, t13);

    // -------------------------------------------------------------------------
    // Per-stage timing report
    // -------------------------------------------------------------------------
    long long t_total = t_gauss_scalar + t_sobel + t_mag_scalar + t_dir;
    auto pct = [&](long long t) -> double {return 100.0 * t / t_total;};

    std::cout << "\n=== Per-Stage Timing (scalar pipeline) ===\n";
    std::cout << "Gaussian blur   : " << t_gauss_scalar << " us  ("<< pct(t_gauss_scalar) << "%)\n";
    std::cout << "Sobel Gx/Gy     : " << t_sobel        << " us  ("<< pct(t_sobel)        << "%)\n";
    std::cout << "Magnitude L1    : " << t_mag_scalar   << " us  ("<< pct(t_mag_scalar)   << "%)\n";
    std::cout << "Direction       : " << t_dir          << " us  ("<< pct(t_dir)          << "%)\n";
    std::cout << "Total (scalar)  : " << t_total        << " us\n\n";

    std::cout << "\n=== RVV vs Scalar ===\n";
    std::cout << "Gaussian scalar : " << t_gauss_scalar << " us\n";
    std::cout << "Gaussian Separable : " << t_gauss_sep << " us\n";
    std::cout << "Gaussian RVV    : " << t_gauss_rvv    << " us\n";
    std::cout << "Gaussian_Scalar_Seperable mismatches: " << gaussMismatch_scalar_sep << "\n";
    std::cout << "Gaussian_Scalar_RVV mismatches: " << gaussMismatch_scalar_rvv << "\n\n";

    std::cout << "Sobel scalar    : " << t_sobel << " us\n";
    std::cout << "Sobel RVV       : " << t_sobel_rvv << " us\n";
    std::cout << "Sobel mismatches: " << sobelMismatch << "\n\n";

    std::cout << "Magnitude scalar: " << t_mag_scalar   << " us\n";
    std::cout << "Magnitude RVV   : " << t_mag_rvv      << " us\n";
    std::cout << "Magnitude mismatches: " << magMismatch << "\n\n";

    std::cout << "\n=== Sanity Checks ===\n";
    std::cout << "Center Gx        : " << grad.gx[500 * 1000 + 500] << "\n";
    std::cout << "Center Gy        : " << grad.gy[500 * 1000 + 500] << "\n";
    std::cout << "Center MagL1     : " << (int)magL1.at(500, 500)   << "\n";
    std::cout << "Center MagL2     : " << (int)magL2.at(500, 500)   << "\n";
    std::cout << "Center Direction : " << (int)dir.at(500, 500)     << "\n";

    saveRawImage("images/output/blur.raw", blur);
    saveRawImage("images/output/blursep.raw", blurSep);

    return 0;
}