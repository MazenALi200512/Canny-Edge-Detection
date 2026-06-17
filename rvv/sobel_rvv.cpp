#include <riscv_vector.h>
#include "sobel.hpp"

// ============================================================================
// RVV Sobel Gradient (Phase 6)
// - Vector-length agnostic (strip mining with vsetvl)
// - Produces identical Gx/Gy output to scalar implementation
// - Zero-padding boundary handling
// - SoA output layout (grad.gx / grad.gy) as required by project guide
// - Uses widening u8 -> i16 accumulation
// ============================================================================

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

static inline void scalar_pixel(
    const Image& input,
    int x,
    int y,
    int16_t& gx_out,
    int16_t& gy_out)
{
    int32_t gx = 0;
    int32_t gy = 0;

    for(int ky = -1; ky <= 1; ky++)
    {
        for(int kx = -1; kx <= 1; kx++)
        {
            int nx = x + kx;
            int ny = y + ky;

            if(nx < 0 || nx >= input.width)  continue;
            if(ny < 0 || ny >= input.height) continue;

            uint8_t p = input.at(nx, ny);

            gx += p * SOBEL_X[ky + 1][kx + 1];
            gy += p * SOBEL_Y[ky + 1][kx + 1];
        }
    }

    gx_out = (int16_t)gx;
    gy_out = (int16_t)gy;
}

static void rvv_row_interior(
    const Image& input,
    Gradient& grad,
    int y)
{
    const int W = input.width;

    int x = 1;
    const int x_end = W - 1;

    while(x < x_end)
    {
        size_t vl = __riscv_vsetvl_e8m1((size_t)(x_end - x));

        vint16m2_t gx_acc = __riscv_vmv_v_x_i16m2(0, vl);
        vint16m2_t gy_acc = __riscv_vmv_v_x_i16m2(0, vl);

        for(int ky = -1; ky <= 1; ky++)
        {
            const uint8_t* row =
                input.data.data() + (y + ky) * W;

            for(int kx = -1; kx <= 1; kx++)
            {
                vuint8m1_t px8 =
                    __riscv_vle8_v_u8m1(row + x + kx, vl);

                vuint16m2_t px16u =
                    __riscv_vzext_vf2_u16m2(px8, vl);

                vint16m2_t px16 =
                    __riscv_vreinterpret_v_u16m2_i16m2(px16u);

                gx_acc = __riscv_vadd_vv_i16m2(
                    gx_acc,
                    __riscv_vmul_vx_i16m2(
                        px16,
                        SOBEL_X[ky + 1][kx + 1],
                        vl),
                    vl);

                gy_acc = __riscv_vadd_vv_i16m2(
                    gy_acc,
                    __riscv_vmul_vx_i16m2(
                        px16,
                        SOBEL_Y[ky + 1][kx + 1],
                        vl),
                    vl);
            }
        }

        int idx = y * W + x;

        __riscv_vse16_v_i16m2(
            grad.gx.data() + idx,
            gx_acc,
            vl);

        __riscv_vse16_v_i16m2(
            grad.gy.data() + idx,
            gy_acc,
            vl);

        x += (int)vl;
    }
}

Gradient sobel_rvv(const Image& input)
{
    Gradient grad(input.width, input.height);

    const int W = input.width;
    const int H = input.height;

    for(int y = 0; y < H; y++)
    {
        if(y == 0 || y == H - 1)
        {
            for(int x = 0; x < W; x++)
            {
                scalar_pixel(
                    input, x, y,
                    grad.gx[y * W + x],
                    grad.gy[y * W + x]);
            }
            continue;
        }

        scalar_pixel(
            input, 0, y,
            grad.gx[y * W],
            grad.gy[y * W]);

        rvv_row_interior(input, grad, y);

        scalar_pixel(
            input, W - 1, y,
            grad.gx[y * W + (W - 1)],
            grad.gy[y * W + (W - 1)]);
    }

    return grad;
}