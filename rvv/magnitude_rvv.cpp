#include "magnitude.hpp"
#include <riscv_vector.h>
#include <cstdint>
#include <algorithm>

void magnitudeL1_inplace_rvv(const Gradient& grad, Image& out)
{
    int total = grad.width * grad.height;

    int maxMag = 0;

    // ==========================
    // PASS 1 (scalar max)
    // ==========================
    for(int i = 0; i < total; i++)
    {
        int mag = std::abs(grad.gx[i]) + std::abs(grad.gy[i]);
        if(mag > maxMag)
            maxMag = mag;
    }

    if(maxMag == 0)
        maxMag = 1;

    float scale = 255.0f / (float)maxMag;

    // ==========================
    // PASS 2 (RVV)
    // ==========================
    int i = 0;

    while(i < total)
    {
        size_t vl = __riscv_vsetvl_e16m1(total - i);

        // load
        vint16m1_t gx = __riscv_vle16_v_i16m1(&grad.gx[i], vl);
        vint16m1_t gy = __riscv_vle16_v_i16m1(&grad.gy[i], vl);

        // abs
        vint16m1_t ax = __riscv_vmax_vx_i16m1(gx, 0, vl);
        vint16m1_t ay = __riscv_vmax_vx_i16m1(gy, 0, vl);

        // widen to int32 BEFORE sum
        vint32m2_t ax32 = __riscv_vwcvt_x_x_v_i32m2(ax, vl);
        vint32m2_t ay32 = __riscv_vwcvt_x_x_v_i32m2(ay, vl);

        vint32m2_t sum = __riscv_vadd_vv_i32m2(ax32, ay32, vl);

        // float conversion (NOW correct type)
        vfloat32m2_t fsum = __riscv_vfcvt_f_x_v_f32m2(sum, vl);

        // scale
        vfloat32m2_t scaled = __riscv_vfmul_vf_f32m2(fsum, scale, vl);

        // back to int
        vint32m2_t norm = __riscv_vfcvt_x_f_v_i32m2(scaled, vl);

        norm = __riscv_vmin_vx_i32m2(norm, 255, vl);

        // narrow properly
        vint16m1_t tmp = __riscv_vncvt_x_x_w_i16m1(norm, vl);

        vuint8m1_t out8 =
            __riscv_vreinterpret_v_i8m1_u8m1(
                __riscv_vreinterpret_v_i16m1_i8m1(tmp)
            );

        __riscv_vse8_v_u8m1(out.data + i, out8, vl);

        i += vl;
    }
}