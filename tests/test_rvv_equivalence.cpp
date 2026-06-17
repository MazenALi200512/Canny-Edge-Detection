#include <cassert>
#include <cstdlib>
#include <cmath>
#include <iostream>

#include "image.hpp"
#include "gaussian.hpp"
#include "sobel.hpp"
#include "magnitude.hpp"

int main()
{
    const int W = 100;
    const int H = 75;
    Image img(W, H);
    for(int y = 0; y < H; y++)
        for(int x = 0; x < W; x++)
            img.at(x, y) = static_cast<uint8_t>((x * 7 + y * 13) & 255);

    // =====================================================
    // Gaussian
    // =====================================================

    Image g_scalar = gaussianBlur(img);
    Image g_rvv    = gaussianBlur_rvv(img);
    for(int i = 0; i < W * H; i++)
    {
        assert(std::abs((int)g_scalar.data[i] - (int)g_rvv.data[i]) <= 1);
    }
    std::cout << "Gaussian OK\n";

    // =====================================================
    // Sobel
    // =====================================================

    Gradient s_scalar = sobel(g_scalar);
    Gradient s_rvv    = sobel_rvv(g_scalar);
    for(int i = 0; i < W * H; i++)
    {
        assert(std::abs((int)s_scalar.gx[i] - (int)s_rvv.gx[i]) <= 1);
        assert(std::abs((int)s_scalar.gy[i] - (int)s_rvv.gy[i]) <= 1);
    }
    std::cout << "Sobel OK\n";

    // =====================================================
    // Magnitude L1
    // =====================================================

    Image m_scalar = magnitudeL1(s_scalar);
    Image m_rvv    = magnitudeL1_rvv(s_scalar);
    for(int i = 0; i < W * H; i++)
        assert(std::abs((int)m_scalar.data[i] - (int)m_rvv.data[i]) <= 1);
    std::cout << "Magnitude OK\n";
    std::cout << "\nALL RVV EQUIVALENCE TESTS PASSED\n";

    return 0;
}