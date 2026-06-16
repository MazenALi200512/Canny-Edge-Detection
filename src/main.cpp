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
    // Synthetic test image: 1000x1000, top half black / bottom half white.
    // Produces a strong horizontal edge at y=500 for meaningful gradient output.
    Image img(1000, 1000);
    for(int y = 0; y < 1000; y++)
        for(int x = 0; x < 1000; x++)
            img.at(x, y) = (y < 500) ? 0 : 255;

    // -------------------------------------------------------------------------
    // Phase 5 profiling: each stage timed independently so percentages are real.
    // -------------------------------------------------------------------------

    // --- Gaussian blur (scalar) ---
    auto t0  = Clock::now();
    Image blur = gaussianBlur(img);
    auto t1  = Clock::now();
    long long t_gauss_scalar = us(t0, t1);

    // --- Gaussian blur (RVV) — averaged over 10 runs ---
    // Pre-allocate output once and warmup before the timer.
    // blurRVV is reused each iteration so zero allocation happens inside
    // the timed region — same pattern as magnitude inplace benchmarks.
    Image blurRVV(img.width, img.height);
    { Image _w = gaussianBlur_rvv(img); (void)_w; }  // warmup: page-fault here
    auto t2 = Clock::now();
    for(int r = 0; r < 10; r++) blurRVV = gaussianBlur_rvv(img);
    auto t3 = Clock::now();
    long long t_gauss_rvv = us(t2, t3) / 10;

    // Correctness check
    int gaussMismatch = 0;
    for(size_t i = 0; i < blur.data.size(); i++)
        if(blur.data[i] != blurRVV.data[i]) gaussMismatch++;

    // --- Sobel ---
    auto t4  = Clock::now();
    Gradient grad = sobel(blur);
    auto t5  = Clock::now();
    long long t_sobel = us(t4, t5);

    // --- Magnitude L1 (scalar) — averaged over 10 runs ---
    // Pre-allocate output once. Warmup run touches all pages so the OS maps
    // physical memory before the timer starts. Timed loop reuses the same
    // buffer — zero heap activity inside the measured region.
    Image magL1(grad.width, grad.height);
    magnitudeL1_inplace(grad, magL1);          // warmup: page-fault here, not below
    auto t6 = Clock::now();
    for(int r = 0; r < 10; r++) magnitudeL1_inplace(grad, magL1);
    auto t7 = Clock::now();
    long long t_mag_scalar = us(t6, t7) / 10;

    // --- Magnitude L1 (RVV) — averaged over 10 runs ---
    // Same pattern: pre-allocate + warmup outside the timer, then pure kernel
    // in the loop. This is an apples-to-apples comparison with the scalar above.
    Image magRVV(grad.width, grad.height);
    magnitudeL1_rvv_inplace(grad, magRVV);     // warmup
    auto t8 = Clock::now();
    for(int r = 0; r < 10; r++) magnitudeL1_rvv_inplace(grad, magRVV);
    auto t9 = Clock::now();
    long long t_mag_rvv = us(t8, t9) / 10;

    // Correctness check (last iteration of both loops written into magL1 / magRVV)
    int magMismatch = 0;
    for(size_t i = 0; i < magL1.data.size(); i++)
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

    auto pct = [&](long long t) -> double {
        return 100.0 * t / t_total;
    };

    std::cout << "\n=== Per-Stage Timing (scalar pipeline) ===\n";
    std::cout << "Gaussian blur   : " << t_gauss_scalar << " us  ("
            << pct(t_gauss_scalar) << "%)\n";
    std::cout << "Sobel Gx/Gy     : " << t_sobel        << " us  ("
            << pct(t_sobel)        << "%)\n";
    std::cout << "Magnitude L1    : " << t_mag_scalar   << " us  ("
            << pct(t_mag_scalar)   << "%)\n";
    std::cout << "Direction       : " << t_dir          << " us  ("
            << pct(t_dir)          << "%)\n";
    std::cout << "Total (scalar)  : " << t_total        << " us\n";

    std::cout << "\n=== RVV vs Scalar ===\n";
    std::cout << "Gaussian scalar : " << t_gauss_scalar << " us\n";
    std::cout << "Gaussian RVV    : " << t_gauss_rvv    << " us\n";
    std::cout << "Gaussian mismatches: " << gaussMismatch << "\n";
    std::cout << "Magnitude scalar: " << t_mag_scalar   << " us\n";
    std::cout << "Magnitude RVV   : " << t_mag_rvv      << " us\n";
    std::cout << "Magnitude mismatches: " << magMismatch << "\n";

    std::cout << "\n=== Sanity Checks ===\n";
    std::cout << "Center Gx        : " << grad.gx[500 * 1000 + 500] << "\n";
    std::cout << "Center Gy        : " << grad.gy[500 * 1000 + 500] << "\n";
    std::cout << "Center MagL1     : " << (int)magL1.at(500, 500)   << "\n";
    std::cout << "Center MagL2     : " << (int)magL2.at(500, 500)   << "\n";
    std::cout << "Center Direction : " << (int)dir.at(500, 500)     << "\n";

    saveRawImage("images/output/blur.raw", blur);

    return 0;
}