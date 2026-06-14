HOST_CXX = g++
RV_CXX   = riscv64-unknown-elf-g++

HOST_FLAGS = -O2 -std=c++17 -Iinclude
RV_FLAGS   = -O2 -std=c++17 -march=rv64gcv -Iinclude

SRC = \
src/main.cpp \
src/image.cpp \
src/gaussian.cpp \
src/sobel.cpp \
src/magnitude.cpp \
src/direction.cpp

HOST_TARGET = build/host/canny
RV_TARGET   = build/riscv/canny_rv

all:
	mkdir -p build/host
	$(HOST_CXX) $(HOST_FLAGS) $(SRC) -o $(HOST_TARGET)

canny_rv:
	mkdir -p build/riscv
	$(RV_CXX) $(RV_FLAGS) $(SRC) -o $(RV_TARGET)

run:
	qemu-riscv64 \
	-cpu rv64,v=true,vlen=128 \
	$(RV_TARGET)

clean:
	rm -rf build

test:
	@echo "GoogleTest will be added in Phase 3"