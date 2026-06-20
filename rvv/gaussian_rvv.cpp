#include "gaussian.hpp"
#include <riscv_vector.h>
#include <cstdint>
#include <algorithm>

static const int16_t K[5] = {1, 4, 7, 4, 1};
static constexpr int KSUM = 17;

// ---------------------------------------------------------------------------
// Division by 17 — vmulhu trick (exact, avoids slow vdiv on QEMU).
// M = ceil(2^32 / 17) = 252,645,136.
// (acc * M) >> 32 == acc / 17 exactly for all acc in [0, 255*17] = [0, 4335].
// ---------------------------------------------------------------------------
static constexpr uint32_t INV17 = 252645136;

Image gaussianBlur_rvv(const Image& input)
{
    Image temp(input.width, input.height);
    Image output(input.width, input.height);
    int w = input.width;
    int h = input.height;

    // ============================================================
    // HORIZONTAL PASS 
    // ============================================================
    for(int y = 0; y < h; y++)
    {
        int x = 0;
        for(; x < w; x++)
        {
            int acc = 0;
            for(int k = -2; k <= 2; k++)
            {
                int nx = x + k;
                if(nx < 0 || nx >= w) continue;   // zero-padding, matches scalar baseline
                acc += input.at(nx, y) * K[k + 2];
            }
            acc /= KSUM;
            acc = std::clamp(acc, 0, 255);
            temp.at(x, y) = (uint8_t)acc;
        }
    }

    // ============================================================
    // VERTICAL PASS (RVV)
    // ============================================================
    for(int y = 0; y < h; y++)
    {
        // Only fully-interior rows can be vectorized without per-tap
        // bounds checks; top/bottom 2 rows fall to scalar.
        bool interiorRow = (y >= 2 && y < h - 2);
        int x = 0;
        if(interiorRow)
        {
            while(x < w)
            {
                size_t vl = __riscv_vsetvl_e8m1((size_t)(w - x));
                vuint16m2_t acc = __riscv_vmv_v_x_u16m2(0, vl);
                for(int k = -2; k <= 2; k++)
                {
                    const uint8_t* src = temp.data  + (y + k) * w + x;
                    vuint8m1_t px8  = __riscv_vle8_v_u8m1(src, vl);
                    vuint16m2_t px16 = __riscv_vzext_vf2_u16m2(px8, vl);
                    acc = __riscv_vmacc_vx_u16m2(acc, (uint16_t)K[k + 2], px16, vl);
                }
                vuint32m4_t acc32 = __riscv_vzext_vf2_u32m4(acc, vl);
                vuint32m4_t div32 = __riscv_vmulhu_vx_u32m4(acc32, INV17, vl);
                div32 = __riscv_vminu_vx_u32m4(div32, 255, vl);
                vuint16m2_t n16 = __riscv_vnclipu_wx_u16m2(div32, 0, __RISCV_VXRM_RDN, vl);
                vuint8m1_t  n8  = __riscv_vnclipu_wx_u8m1(n16,  0, __RISCV_VXRM_RDN, vl);
                __riscv_vse8_v_u8m1(output.data  + y * w + x, n8, vl);
                x += (int)vl;
            }
        }
        else
        {
            // entire row is border (top/bottom 2 rows) -> scalar
            for(; x < w; x++)
            {
                int acc = 0;
                for(int k = -2; k <= 2; k++)
                {
                    int ny = y + k;
                    if(ny < 0 || ny >= h) continue;
                    acc += temp.at(x, ny) * K[k + 2];
                }
                acc /= KSUM;
                acc = std::clamp(acc, 0, 255);
                output.at(x, y) = (uint8_t)acc;
            }
        }
    }
    return output;
}