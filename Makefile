HOST_CXX = g++
RV_CXX   = riscv64-unknown-elf-g++

OPT ?= -O2
HOST_FLAGS = $(OPT) -std=c++17 -Iinclude 

RV_FLAGS   = $(OPT) -std=c++17 -march=rv64gcv -Iinclude

SRC = \
src/main.cpp \
src/image.cpp \
src/gaussian.cpp \
src/sobel.cpp \
src/magnitude.cpp \
src/direction.cpp

RVV_SRC = \
rvv/magnitude_rvv.cpp \
rvv/gaussian_rvv.cpp

HOST_STUB = rvv/rvv_stub.cpp

HOST_TARGET = build/host/canny
RV_TARGET   = build/riscv/canny_rv

all:
	mkdir -p build/host
	$(HOST_CXX) $(HOST_FLAGS) $(SRC) $(HOST_STUB) -o $(HOST_TARGET)

canny_rv:
	mkdir -p build/riscv
	$(RV_CXX) $(RV_FLAGS) $(SRC) $(RVV_SRC) -o $(RV_TARGET)

run:
	qemu-riscv64 \
	-cpu rv64,v=true,vlen=256 \
	$(RV_TARGET)

# QEMU-side RVV equivalence test — verifies scalar vs RVV at VLEN 128/256/512
EQUIV_TARGET = build/riscv/test_rvv_equiv

test_equiv:
	mkdir -p build/riscv
	$(RV_CXX) $(RV_FLAGS) \
	tests/test_rvv_equivalence.cpp \
	src/image.cpp src/gaussian.cpp src/sobel.cpp \
	src/magnitude.cpp src/direction.cpp \
	$(RVV_SRC) \
	-o $(EQUIV_TARGET)
	@echo "--- VLEN=128 ---"
	qemu-riscv64 -cpu rv64,v=true,vlen=128 $(EQUIV_TARGET)
	@echo "--- VLEN=256 ---"
	qemu-riscv64 -cpu rv64,v=true,vlen=256 $(EQUIV_TARGET)
	@echo "--- VLEN=512 ---"
	qemu-riscv64 -cpu rv64,v=true,vlen=512 $(EQUIV_TARGET)

clean:
	rm -rf build

TEST_SRC = \
tests/test_gaussian.cpp \
tests/test_sobel.cpp \
tests/test_magnitude.cpp \
tests/test_direction.cpp

test:
	mkdir -p build/tests
	$(HOST_CXX) $(HOST_FLAGS) \
	$(TEST_SRC) \
	src/gaussian.cpp \
	src/sobel.cpp \
	src/magnitude.cpp \
	src/direction.cpp \
	src/image.cpp \
	$(HOST_STUB) \
	-lgtest -lgtest_main -pthread \
	-o build/tests/tests

	./build/tests/tests