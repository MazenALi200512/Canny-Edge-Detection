#include "gaussian.hpp"
#include <riscv_vector.h>
#include <cstdint>
#include <algorithm>

static const int16_t K[5] = {1, 4, 7, 4, 1};
static constexpr int KSUM = 17;

Image gaussianBlur_rvv(const Image& input)
{
    Image temp(input.width, input.height);
    Image output(input.width, input.height);
    int w = input.width;
    int h = input.height;

    // ============================================================
    // HORIZONTAL PASS (RVV)
    // ============================================================
    for(int y = 0; y < h; y++)
    {
        int x = 0;
        for(; x <= w - 16; x += 16)
        {
            size_t vl = __riscv_vsetvl_e16m2(16);   // set vector length to 16
            vuint16m2_t sum = __riscv_vmv_v_x_u16m2(0, vl);     // set vector values to zeros
            for(int k = -2; k <= 2; k++)
            {
                const uint8_t* src = input.data + (y * w + (x + k));
                vuint8m1_t pix = __riscv_vle8_v_u8m1(src, vl);      // load 16 pixels to pix
                vuint16m2_t wide = __riscv_vwcvtu_x_x_v_u16m2(pix, vl);     // wide the pixel size from 8-bits to 16-bits
                sum = __riscv_vmacc_vx_u16m2(sum, K[k + 2], wide, vl);      // multiply and accumulate (each pixel multiplied by each kernel value and eventually summed)
            }
            sum = __riscv_vdivu_vx_u16m2(sum, KSUM, vl);    // divide by 17
            sum = __riscv_vminu_vx_u16m2(sum, 255, vl);     // clamp 0..255
            vuint8m1_t out8 = __riscv_vncvt_x_x_w_u8m1(sum, vl);    // narrow the pixel size from 16-bits to 8-bits (original pixel size)
            __riscv_vse8_v_u8m1(temp.data + y * w + x, out8, vl);   // store 16 pixels resulting from sum to temp to be passed onto vertical pass
        }

        // tail pixels processing scalar
        for(; x < w; x++)
        {
            int acc = 0;
            for(int k = -2; k <= 2; k++)
            {
                // int nx = std::clamp(x + k, 0, w - 1);
                int nx = x + k; 
                if(nx < 0 || nx >= input.width) continue;
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
        int x = 0;
        for(; x <= w - 16; x += 16)
        {
            size_t vl = __riscv_vsetvl_e16m2(16);
            vuint16m2_t sum = __riscv_vmv_v_x_u16m2(0, vl);
            for(int k = -2; k <= 2; k++)
            {
                const uint8_t* src = temp.data + ((y + k) * w + x);
                vuint8m1_t pix = __riscv_vle8_v_u8m1(src, vl);
                vuint16m2_t wide = __riscv_vwcvtu_x_x_v_u16m2(pix, vl);
                sum = __riscv_vmacc_vx_u16m2(sum, K[k + 2], wide, vl);
            }
            sum = __riscv_vdivu_vx_u16m2(sum, KSUM, vl);
            sum = __riscv_vminu_vx_u16m2(sum, 255, vl);
            vuint8m1_t out8 = __riscv_vncvt_x_x_w_u8m1(sum, vl);
            __riscv_vse8_v_u8m1(output.data + y * w + x, out8, vl);
        }

        // border scalar
        for(; x < w; x++)
        {
            int acc = 0;
            for(int k = -2; k <= 2; k++)
            {
                // int ny = std::clamp(y + k, 0, h - 1);
                int ny = y + k;
                if(ny < 0 || ny >= input.height) continue;
                acc += temp.at(x, ny) * K[k + 2];
            }
            acc /= KSUM;
            acc = std::clamp(acc, 0, 255);
            output.at(x, y) = (uint8_t)acc;
        }
    }
    return output;
}