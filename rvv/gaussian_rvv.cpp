#include "gaussian.hpp"
#include <riscv_vector.h>
#include <cstdint>
#include <algorithm>

// ============================================================================
// RVV Gaussian Blur — SEPARABLE, matches scalar gaussianBlurSeparable() exactly
// ============================================================================
// Scalar baseline (gaussian.cpp):
//   kernel  = GAUSS_SEPARABLE = {1, 4, 7, 4, 1}
//   sum     = SEPARABLE_SUM   = 17
//   2 passes: horizontal (input->temp), vertical (temp->output)
//   boundary: zero-padding (skip out-of-bounds taps)
//
// This RVV version mirrors that exactly so equivalence tests give 0 mismatches.
// ============================================================================

static const int16_t K[5] = {1, 4, 7, 4, 1};
static constexpr int32_t KSUM = 17;

// Fixed-point reciprocal for division by 17.
// M = ceil(2^32 / 17) = 252,645,136.
// (acc * M) >> 32 == acc / 17 exactly for all acc in [0, 255*17] = [0, 4335].
static constexpr uint32_t INV17 = 252645136u;

// ---------------------------------------------------------------------------
// Scalar fallback for a single output pixel along a 1D line (used for
// borders). Matches the inner loop of convolveSeparable() exactly.
// horizontal=true  -> walk along x (neighbors are (x+k, y))
// horizontal=false -> walk along y (neighbors are (x, y+k))
// ---------------------------------------------------------------------------
static uint8_t scalar_tap_h(const Image& src, int x, int y)
{
    int32_t sum = 0;
    for(int k = -2; k <= 2; k++)
    {
        int nx = x + k;
        if(nx < 0 || nx >= src.width) continue;
        sum += static_cast<int32_t>(src.at(nx, y)) * static_cast<int32_t>(K[k + 2]);
    }
    sum /= KSUM;
    sum = std::max(sum, (int32_t)0);
    sum = std::min(sum, (int32_t)255);
    return (uint8_t)sum;
}

static uint8_t scalar_tap_v(const Image& src, int x, int y)
{
    int32_t sum = 0;
    for(int k = -2; k <= 2; k++)
    {
        int ny = y + k;
        if(ny < 0 || ny >= src.height) continue;
        sum += static_cast<int32_t>(src.at(x, ny)) * static_cast<int32_t>(K[k + 2]);
    }
    sum /= KSUM;
    sum = std::max(sum, (int32_t)0);
    sum = std::min(sum, (int32_t)255);
    return (uint8_t)sum;
}

// ---------------------------------------------------------------------------
// RVV interior row, horizontal pass. Processes columns [2, W-2) — all 5
// taps guaranteed in-bounds, no per-tap check needed.
// ---------------------------------------------------------------------------
static void rvv_horizontal_interior(const Image& input, Image& temp, int y)
{
    const int W     = input.width;
    const int x_end = W - 2;
    int       x     = 2;

    while(x < x_end)
    {
        // Ask the hardware how many 8-bit elements it can take this round.
        size_t vl = __riscv_vsetvl_e8m1((size_t)(x_end - x));

        // Accumulator: base e8m1 widened once -> u16m2.
        // Max value per tap: 255 * 7 = 1785; max total: 255*17=4335 -> fits u16.
        vuint16m2_t acc = __riscv_vmv_v_x_u16m2(0, vl);

        for(int k = -2; k <= 2; k++)
        {
            const uint8_t* src = input.data  + y * W + (x + k);

            // Step 1: load vl pixels (8-bit).
            vuint8m1_t px8 = __riscv_vle8_v_u8m1(src, vl);

            // Step 2: zero-extend u8m1 -> u16m2.
            vuint16m2_t px16 = __riscv_vzext_vf2_u16m2(px8, vl);

            // Step 3: multiply-accumulate by scalar kernel weight.
            acc = __riscv_vmacc_vx_u16m2(acc, (uint16_t)K[k + 2], px16, vl);
        }

        // Step 4: divide by 17 via vmulhu trick (avoid slow vdiv on QEMU).
        // Widen acc (u16m2) -> u32m4 so the multiply-high has matching width.
        vuint32m4_t acc32 = __riscv_vzext_vf2_u32m4(acc, vl);
        vuint32m4_t div32 = __riscv_vmulhu_vx_u32m4(acc32, INV17, vl);
        div32 = __riscv_vminu_vx_u32m4(div32, 255u, vl);

        // Step 5: narrow u32m4 -> u16m2 -> u8m1.
        vuint16m2_t n16 = __riscv_vnclipu_wx_u16m2(div32, 0, __RISCV_VXRM_RDN, vl);
        vuint8m1_t  n8  = __riscv_vnclipu_wx_u8m1(n16,  0, __RISCV_VXRM_RDN, vl);

        __riscv_vse8_v_u8m1(temp.data  + y * W + x, n8, vl);

        x += (int)vl;
    }
}

// ---------------------------------------------------------------------------
// RVV interior column-strip, vertical pass. Called only for rows where
// [y-2, y+2] is fully inside [0, H). Processes the whole row in one strip.
// ---------------------------------------------------------------------------
static void rvv_vertical_interior(const Image& temp, Image& output, int y)
{
    const int W = output.width;
    int       x = 0;

    while(x < W)
    {
        size_t vl = __riscv_vsetvl_e8m1((size_t)(W - x));

        vuint16m2_t acc = __riscv_vmv_v_x_u16m2(0, vl);

        for(int k = -2; k <= 2; k++)
        {
            const uint8_t* src = temp.data  + (y + k) * W + x;
            vuint8m1_t  px8  = __riscv_vle8_v_u8m1(src, vl);
            vuint16m2_t px16 = __riscv_vzext_vf2_u16m2(px8, vl);
            acc = __riscv_vmacc_vx_u16m2(acc, (uint16_t)K[k + 2], px16, vl);
        }

        vuint32m4_t acc32 = __riscv_vzext_vf2_u32m4(acc, vl);
        vuint32m4_t div32 = __riscv_vmulhu_vx_u32m4(acc32, INV17, vl);
        div32 = __riscv_vminu_vx_u32m4(div32, 255u, vl);

        vuint16m2_t n16 = __riscv_vnclipu_wx_u16m2(div32, 0, __RISCV_VXRM_RDN, vl);
        vuint8m1_t  n8  = __riscv_vnclipu_wx_u8m1(n16,  0, __RISCV_VXRM_RDN, vl);

        __riscv_vse8_v_u8m1(output.data  + y * W + x, n8, vl);

        x += (int)vl;
    }
}

// ---------------------------------------------------------------------------
// Public entry point
// ---------------------------------------------------------------------------
Image gaussianBlur_rvv(const Image& input)
{
    Image temp(input.width, input.height);
    Image output(input.width, input.height);
    const int W = input.width;
    const int H = input.height;

    // ============================================================
    // HORIZONTAL PASS: input -> temp
    // ============================================================
    for(int y = 0; y < H; y++)
    {
        // Left border (x=0,1): scalar
        for(int x = 0; x < 2 && x < W; x++)
            temp.at(x, y) = scalar_tap_h(input, x, y);

        // Interior [2, W-2): vectorized
        if(W > 4)
            rvv_horizontal_interior(input, temp, y);

        // Right border (x=W-2,W-1): scalar
        for(int x = std::max(2, W - 2); x < W; x++)
            temp.at(x, y) = scalar_tap_h(input, x, y);
    }

    // ============================================================
    // VERTICAL PASS: temp -> output
    // ============================================================
    for(int y = 0; y < H; y++)
    {
        if(y < 2 || y >= H - 2)
        {
            // Entire row is a top/bottom border row: scalar
            for(int x = 0; x < W; x++)
                output.at(x, y) = scalar_tap_v(temp, x, y);
        }
        else
        {
            // Fully interior row: vectorized across all columns
            rvv_vertical_interior(temp, output, y);
        }
    }

    return output;
}