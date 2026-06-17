#include <riscv_vector.h>
#include "gaussian.hpp"
#include "image.hpp"

// ============================================================================
// RVV Gaussian Blur — 5x5 kernel, integer arithmetic, zero-padding boundary
// ============================================================================
//
// The 5x5 Gaussian kernel (same as scalar gaussianBlur):
//
//   1  4  7  4  1
//   4 16 26 16  4
//   7 26 41 26  7      sum = 273
//   4 16 26 16  4
//   1  4  7  4  1
//
// Algorithm structure (matches doctor's Phase 6 specification):
//   Outer loops:  over output rows (y) and kernel rows/columns (ky, kx) — scalar
//   Inner loop:   over output columns in strip-mined chunks — VECTORIZED
//
// For each chunk of output pixels at columns [x, x+vl):
//   For each kernel tap (ky, kx):
//     1. Load vl input pixels (u8m1)
//     2. Zero-extend u8m1 → u16m2  (vzext_vf2)
//     3. Reinterpret u16m2 → i16m2 (vreinterpret, zero cost)
//     4. Widening multiply i16m2 × scalar coeff → i32m4  (vwmul_vx)
//     5. Accumulate into i32m4 accumulator  (vadd_vv)
//   After all 25 taps:
//     6. Divide by 273 using vmulhu trick (see below) — exact, no vdiv
//     7. Narrow i32m4 → u16m2 → u8m1  (two vnclipu steps)
//     8. Store vl bytes to output
//
// Division by 273 — vmulhu trick (exact, avoids slow vdiv):
//   vdiv is a scalar-speed operation in QEMU (not pipelined) and was measured
//   at 276,120 µs vs 146,004 µs scalar. The fix: precompute M = ceil(2^32/273)
//   = 15,732,481, then (acc * M) >> 32 == acc / 273 exactly for all acc in
//   [0, 69615] (verified across full range, zero errors).
//   vmulhu_vx_u32m4 computes the upper 32 bits of a 32×32 unsigned multiply —
//   equivalent to (acc * M) >> 32 — without widening to i64 or touching LMUL.
//   This is the standard compiler technique for constant integer division.
//
// LMUL choice — e8m1 base, widening to i32m4 accumulator:
//   Widening doubles LMUL: u8m1 → u16m2 → i32m4.
//   LMUL=4 on i32 gives:
//     vlen=128 → 16 pixels/iter   vlen=256 → 32   vlen=512 → 64
//   LMUL=8 would give twice as many pixels per iter but leaves only 4 logical
//   registers — not enough for acc + temporaries across 25 kernel taps.
//   LMUL=4 is the practical sweet spot for this kernel.
//
// Boundary handling: zero-padding (identical to scalar).
//   Border rows/columns (within 2 of any edge): scalar fallback.
//   Interior [2, W-2) × [2, H-2): fully vectorized, no per-tap bounds check.
//
// VLA: vsetvl_e8m1 called once per strip — correct at vlen=128, 256, 512.
// ============================================================================

static const int16_t GAUSS_KERNEL[25] =
{
    1,  4,  7,  4,  1,
    4, 16, 26, 16,  4,
    7, 26, 41, 26,  7,
    4, 16, 26, 16,  4,
    1,  4,  7,  4,  1
};

// Fixed-point reciprocal for division by 273.
// M = ceil(2^32 / 273) = 15,732,481.
// (acc * M) >> 32 == acc / 273 for all acc in [0, 69615]. Verified exhaustively.
static constexpr uint32_t INV273 = 15732481u;

// ---------------------------------------------------------------------------
// Scalar fallback for a single pixel — used for border rows/columns.
// Identical to convolve2D logic in convolution.hpp.
// ---------------------------------------------------------------------------
static uint8_t scalar_pixel(const Image& input, int cx, int cy)
{
    int32_t sum = 0;
    for(int ky = -2; ky <= 2; ky++)
    {
        for(int kx = -2; kx <= 2; kx++)
        {
            int nx = cx + kx;
            int ny = cy + ky;
            if(nx < 0 || nx >= input.width)  continue;
            if(ny < 0 || ny >= input.height) continue;
            int ki = (ky + 2) * 5 + (kx + 2);
            sum += (int32_t)input.at(nx, ny) * GAUSS_KERNEL[ki];
        }
    }
    sum /= 273;
    if(sum < 0)   sum = 0;
    if(sum > 255) sum = 255;
    return (uint8_t)sum;
}

// ---------------------------------------------------------------------------
// RVV interior row — processes columns [2, W-2) for a given output row y.
// All 25 taps guaranteed in-bounds — no per-tap boundary check.
// ---------------------------------------------------------------------------
static void rvv_row_interior(const Image& input, Image& output, int y)
{
    const int W     = input.width;
    const int x_end = W - 2;
    int       x     = 2;

    while(x < x_end)
    {
        // Set vl for this strip.
        // vsetvl_e8m1: base element = 8-bit, LMUL=1.
        // After widening the accumulator will be i32m4 (LMUL doubles twice).
        // vlen=128→vl=16, vlen=256→vl=32, vlen=512→vl=64.
        size_t vl = __riscv_vsetvl_e8m1((size_t)(x_end - x));

        // Initialise i32m4 accumulator to zero.
        // vmv_v_x_i32m4: broadcast scalar 0 into vl lanes of a 4-register group.
        vint32m4_t acc = __riscv_vmv_v_x_i32m4(0, vl);

        for(int ky = -2; ky <= 2; ky++)
        {
            const uint8_t* row = input.data + (y + ky) * W;

            for(int kx = -2; kx <= 2; kx++)
            {
                int16_t coeff = GAUSS_KERNEL[(ky + 2) * 5 + (kx + 2)];

                // Step 1: load vl pixels.
                // vle8_v_u8m1: vector load 8-bit unsigned, LMUL=1.
                vuint8m1_t px8 = __riscv_vle8_v_u8m1(row + x + kx, vl);

                // Step 2: zero-extend u8m1 → u16m2.
                // vzext_vf2_u16m2: each 8-bit element zero-extended to 16 bits.
                // LMUL doubles: m1 → m2.
                vuint16m2_t px16u = __riscv_vzext_vf2_u16m2(px8, vl);

                // Step 3: reinterpret u16m2 → i16m2 (zero runtime cost).
                // Pixel values 0–255 are non-negative; bit pattern is identical.
                vint16m2_t px16 = __riscv_vreinterpret_v_u16m2_i16m2(px16u);

                // Step 4: widening multiply i16m2 × scalar coeff → i32m4.
                // vwmul_vx_i32m4: each i16 element multiplied by i16 scalar,
                // result widened to i32. LMUL doubles: m2 → m4.
                // Max single-tap value: 255 × 41 = 10,455 — fits in i32.
                vint32m4_t prod = __riscv_vwmul_vx_i32m4(px16, coeff, vl);

                // Step 5: accumulate.
                // vadd_vv_i32m4: element-wise add, i32m4 + i32m4 → i32m4.
                // Max accumulated value after 25 taps: 255×273 = 69,615 — fits in i32.
                acc = __riscv_vadd_vv_i32m4(acc, prod, vl);
            }
        }

        // Step 6: divide by 273 using vmulhu trick.
        // Reinterpret i32m4 as u32m4 for unsigned multiply-high.
        // vreinterpret: zero cost. Accumulator is non-negative so safe.
        vuint32m4_t acc_u = __riscv_vreinterpret_v_i32m4_u32m4(acc);

        // vmulhu_vx_u32m4: compute upper 32 bits of (acc_u × INV273).
        // Equivalent to (acc_u * 15732481) >> 32 — exact division by 273
        // for all acc in [0, 69615]. Verified exhaustively (zero errors).
        // Why not vdiv: vdiv measured 276,120 µs (1.9× SLOWER than scalar)
        // on QEMU because QEMU emulates integer division without pipelining.
        // vmulhu is a single fast multiply instruction — no emulation penalty.
        // On real hardware vmulhu is also faster than vdiv.
        // If VLEN changes, this result is identical — INV273 is a constant.
        vuint32m4_t div_u = __riscv_vmulhu_vx_u32m4(acc_u, INV273, vl);

        // Step 6b: clamp to [0, 255].
        // Result of vmulhu is already in [0, 255] for valid inputs (max=69615/273=255)
        // but vmin is kept to mirror scalar behaviour exactly.
        div_u = __riscv_vminu_vx_u32m4(div_u, 255u, vl);

        // Step 7a: narrow u32m4 → u16m2.
        // vnclipu_wx_u16m2: shift=0, saturating narrow. LMUL: m4 → m2.
        // Values already ≤255 so saturation never fires.
        vuint16m2_t n16 = __riscv_vnclipu_wx_u16m2(div_u, 0, __RISCV_VXRM_RDN, vl);

        // Step 7b: narrow u16m2 → u8m1.
        // vnclipu_wx_u8m1: shift=0. LMUL: m2 → m1.
        vuint8m1_t out8 = __riscv_vnclipu_wx_u8m1(n16, 0, __RISCV_VXRM_RDN, vl);

        // Step 8: store vl bytes to output row y.
        // vse8_v_u8m1: vector store 8-bit, LMUL=1.
        __riscv_vse8_v_u8m1(output.data + y * W + x, out8, vl);

        x += (int)vl;
    }
}

// ---------------------------------------------------------------------------
// Public entry point
// ---------------------------------------------------------------------------
Image gaussianBlur_rvv(const Image& input)
{
    Image output(input.width, input.height);
    const int W = input.width;
    const int H = input.height;

    for(int y = 0; y < H; y++)
    {
        if(y < 2 || y >= H - 2)
        {
            for(int x = 0; x < W; x++)
                output.at(x, y) = scalar_pixel(input, x, y);
        }
        else
        {
            for(int x = 0; x < 2; x++)
                output.at(x, y) = scalar_pixel(input, x, y);

            rvv_row_interior(input, output, y);

            for(int x = W - 2; x < W; x++)
                output.at(x, y) = scalar_pixel(input, x, y);
        }
    }

    return output;
}