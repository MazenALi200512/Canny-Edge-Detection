#include <riscv_vector.h>
#include "magnitude.hpp"

static void rvv_l1_kernel(const Gradient& grad, Image& output)
{
    int total = grad.width * grad.height;
    int i = 0;

    while(i < total)
    {
        size_t vl = __riscv_vsetvl_e16m4(total - i);
        vint16m4_t gx = __riscv_vle16_v_i16m4(grad.gx.data() + i, vl);
        vint16m4_t gy = __riscv_vle16_v_i16m4(grad.gy.data() + i, vl);
        gx = __riscv_vmax_vv_i16m4(gx, __riscv_vneg_v_i16m4(gx, vl), vl);
        gy = __riscv_vmax_vv_i16m4(gy, __riscv_vneg_v_i16m4(gy, vl), vl);
        vint16m4_t mag = __riscv_vadd_vv_i16m4(gx, gy, vl);
        mag = __riscv_vmin_vv_i16m4(mag, __riscv_vmv_v_x_i16m4(255, vl), vl);
        vuint16m4_t mag_u  = __riscv_vreinterpret_v_i16m4_u16m4(mag);
        vuint8m2_t  mag_u8 = __riscv_vnclipu_wx_u8m2(mag_u, 0, __RISCV_VXRM_RDN, vl);
        __riscv_vse8_v_u8m2(output.data + i, mag_u8, vl);
        i += (int)vl;
    }
}

Image magnitudeL1_rvv(const Gradient& grad)
{
    Image output(grad.width, grad.height);
    rvv_l1_kernel(grad, output);
    return output;
}

void magnitudeL1_rvv_inplace(const Gradient& grad, Image& out)
{
    rvv_l1_kernel(grad, out);
}