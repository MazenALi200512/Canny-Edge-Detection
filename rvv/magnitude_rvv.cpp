#include <riscv_vector.h>
#include "magnitude.hpp"

Image magnitudeL1_rvv(const Gradient& grad)
{
    Image output(grad.width, grad.height);

    int total = grad.width * grad.height;

    for(int i = 0; i < total;)
    {
        size_t vl = __riscv_vsetvl_e16m1(total - i);

        vint16m1_t gx =
            __riscv_vle16_v_i16m1(
                grad.gx.data() + i,
                vl);

        vint16m1_t gy =
            __riscv_vle16_v_i16m1(
                grad.gy.data() + i,
                vl);

        vint16m1_t gx_neg =
            __riscv_vneg_v_i16m1(gx, vl);

        vint16m1_t gy_neg =
            __riscv_vneg_v_i16m1(gy, vl);

        gx =
            __riscv_vmax_vv_i16m1(
                gx,
                gx_neg,
                vl);

        gy =
            __riscv_vmax_vv_i16m1(
                gy,
                gy_neg,
                vl);

        vint16m1_t mag =
            __riscv_vadd_vv_i16m1(
                gx,
                gy,
                vl);

        vint16m1_t limit =
            __riscv_vmv_v_x_i16m1(
                255,
                vl);

        mag =
            __riscv_vmin_vv_i16m1(
                mag,
                limit,
                vl);

        std::vector<int16_t> temp(vl);

        __riscv_vse16_v_i16m1(
            temp.data(),
            mag,
            vl);

        for(size_t j = 0; j < vl; j++)
        {
            output.data[i + j] =
                static_cast<uint8_t>(temp[j]);
        }

        i += vl;
    }

    return output;
}