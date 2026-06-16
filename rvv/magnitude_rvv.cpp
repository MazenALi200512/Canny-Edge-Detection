#include <riscv_vector.h>
#include "magnitude.hpp"

static void rvv_l1_kernel(const Gradient& grad, Image& output)
{
    int total = grad.width * grad.height;
    int i     = 0;

    while(i < total)
    {
        // How many 16-bit elements to process this iteration.
        // vlen=128 → 32,  vlen=256 → 64,  vlen=512 → 128
        size_t vl = __riscv_vsetvl_e16m4(total - i);

        // Load gx and gy as signed 16-bit vectors (SoA layout: contiguous)
        vint16m4_t gx = __riscv_vle16_v_i16m4(grad.gx.data() + i, vl);
        vint16m4_t gy = __riscv_vle16_v_i16m4(grad.gy.data() + i, vl);

        // Absolute value: abs(x) = max(x, -x)
        // RVV 1.0 has no integer vabs; this two-instruction idiom is standard.
        gx = __riscv_vmax_vv_i16m4(gx, __riscv_vneg_v_i16m4(gx, vl), vl);
        gy = __riscv_vmax_vv_i16m4(gy, __riscv_vneg_v_i16m4(gy, vl), vl);

        // L1 magnitude: |gx| + |gy|
        // Max value: 255 + 255 = 510, fits in int16_t (max 32,767) — no overflow
        vint16m4_t mag = __riscv_vadd_vv_i16m4(gx, gy, vl);

        // Clamp to 255 (scalar broadcast then element-wise min)
        mag = __riscv_vmin_vv_i16m4(mag,
                __riscv_vmv_v_x_i16m4(255, vl), vl);

        // Narrow i16m4 -> u8m2 and store directly to output (no temp buffer).
        // vreinterpret is a compile-time type rename — zero runtime cost.
        // vnclipu shift=0 keeps the lower 8 bits; RDN rounding mode.
        vuint16m4_t mag_u  = __riscv_vreinterpret_v_i16m4_u16m4(mag);
        vuint8m2_t  mag_u8 = __riscv_vnclipu_wx_u8m2(
                                mag_u, 0, __RISCV_VXRM_RDN, vl);

        // Store vl bytes directly into the output image — no scalar loop needed
        __riscv_vse8_v_u8m2(output.data.data() + i, mag_u8, vl);

        i += (int)vl;
    }
}

// Returning wrapper — convenience for pipeline use (allocates its own buffer).
Image magnitudeL1_rvv(const Gradient& grad)
{
    Image output(grad.width, grad.height);
    rvv_l1_kernel(grad, output);
    return output;
}

// Inplace overload — for benchmarking: caller pre-allocates, no heap activity
// inside the timed region, so measurements reflect only the vector kernel.
void magnitudeL1_rvv_inplace(const Gradient& grad, Image& out)
{
    rvv_l1_kernel(grad, out);
}