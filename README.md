# Canny Edge Detection on RISC-V

Embedded Systems project implementing the Canny edge detection pipeline in C++,
targeting RISC-V (rv64gcv) with hand-optimized RVV intrinsic kernels.

## Pipeline Stages

1. Gaussian blur (5×5, integer arithmetic, zero-padding)
2. Sobel gradient (Gx and Gy, SoA layout)
3. Gradient magnitude (L1 and L2)
4. Gradient direction (quantized to 0 / 45 / 90 / 135°, integer arithmetic)

## Prerequisites

- RISC-V GNU toolchain built with `--with-arch=rv64gcv`
  (`riscv64-unknown-elf-g++` must be on your PATH)
- QEMU built for `riscv64-linux-user` (`qemu-riscv64` must be on your PATH)
- GoogleTest installed (`libgtest-dev` or built from source)
- Native `g++` with C++17 support

## Build

```bash
# Host (x86/ARM) — uses scalar stubs for RVV functions
make

# RISC-V (cross-compiled with RVV intrinsics)
make canny_rv
```

## Run

```bash
# Run on QEMU at VLEN=128
make run

# Or manually at a different VLEN
qemu-riscv64 -cpu rv64,v=true,vlen=256 build/riscv/canny_rv
qemu-riscv64 -cpu rv64,v=true,vlen=512 build/riscv/canny_rv
```

## Tests

### Host-side unit tests (GoogleTest)

Covers all four pipeline stages with 19 test cases. Uses non-power-of-two
image sizes (100×75, 21×21) to exercise strip-mining tail cases.

```bash
make test
```

Test cases per stage:

| Suite     | Tests |
|-----------|-------|
| Gaussian  | UniformImage, BlackImage, ImpulseResponse, ReducesPeak |
| Sobel     | UniformImage, VerticalEdge, HorizontalEdge, DiagonalEdge |
| Magnitude | L1AndL2NonZero, L1GreaterOrEqualL2, PureHorizontalGradientL1EqualsL2, ZeroGradient, Saturation |
| Direction | VerticalEdge, HorizontalEdge, DiagonalEdge45, DiagonalEdge135, NearVertical, NearHorizontal |

### QEMU-side RVV equivalence tests

Verifies that each RVV kernel produces identical output to the scalar
reference on a 100×75 image. Runs automatically at VLEN=128, 256, and 512
to confirm vector-length-agnostic behaviour.

```bash
make test_equiv
```

## Clean

```bash
make clean
```
