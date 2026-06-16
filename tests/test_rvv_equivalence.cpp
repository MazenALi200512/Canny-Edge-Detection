#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "image.hpp"
#include "gaussian.hpp"
#include "magnitude.hpp"
#include "sobel.hpp"

// ---- helpers ---------------------------------------------------------------

static int check(const char* name, const uint8_t* ref, const uint8_t* rvv, int total, int tolerance = 0)
{
    int mismatches = 0;
    for(int i = 0; i < total; i++)
    {
        int diff = (int)ref[i] - (int)rvv[i];
        if(diff < 0) diff = -diff;
        if(diff > tolerance)
            mismatches++;
    }
    if(mismatches == 0)
        printf("  PASS  %s  (0 mismatches)\n", name);
    else
        printf("  FAIL  %s  (%d mismatches > tolerance %d)\n", name, mismatches, tolerance);
    return mismatches;
}

// Build a synthetic test image: non-power-of-two (100x75)
// top half black, bottom half white — exercises the gradient transition
static Image make_test_image()
{
    Image img(100, 75);
    for(int y = 0; y < 75; y++)
        for(int x = 0; x < 100; x++)
            img.at(x, y) = (y < 37) ? 0 : 255;
    return img;
}

// ---- main ------------------------------------------------------------------

int main()
{
    printf("=== RVV Equivalence Test ===\n");
    printf("Image size: 100x75 (non-power-of-two, exercises strip-mining tail)\n\n");

    int total_failures = 0;
    const int W = 100, H = 75, N = W * H;

    Image img = make_test_image();

    // ---- Gaussian blur -----------------------------------------------------
    printf("[ Gaussian blur ]\n");
    {
        Image scalar = gaussianBlur(img);
        Image rvv    = gaussianBlur_rvv(img);

        // Exact match required (same integer arithmetic, same divisor)
        total_failures += check("gaussianBlur_rvv vs scalar",
                                scalar.data.data(), rvv.data.data(), N, 0);
    }

    // ---- Magnitude L1 ------------------------------------------------------
    printf("[ Magnitude L1 ]\n");
    {
        Gradient grad = sobel(gaussianBlur(img));

        Image scalar = magnitudeL1(grad);
        Image rvv    = magnitudeL1_rvv(grad);

        // Exact match required (L1 is integer, no rounding)
        total_failures += check("magnitudeL1_rvv vs scalar",
                                scalar.data.data(), rvv.data.data(), N, 0);
    }

    // ---- Summary -----------------------------------------------------------
    printf("\n");
    if(total_failures == 0)
    {
        printf("ALL TESTS PASSED\n");
        return 0;
    }
    else
    {
        printf("FAILED: %d test(s) failed\n", total_failures);
        return 1;
    }
}