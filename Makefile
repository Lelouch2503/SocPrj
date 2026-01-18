BUILD_DIR = build

CMAKE_FLAGS = -S . -B $(BUILD_DIR) \
              -G "Unix Makefiles" \
              -DCMAKE_BUILD_TYPE=Debug \
              -DUSE_CONAN=ON \
              -DUSE_CPM=OFF \
              -DCMAKE_TOOLCHAIN_FILE=$(BUILD_DIR)/build/Debug/generators/conan_toolchain.cmake

NPROCS = $(shell nproc 2>/dev/null || echo 1)

.PHONY: all clean configure build run format conan_d conan_r

# Default: clean -> configure -> build
all: clean configure build

clean:
	@echo "[Make] Cleaning build directory..."
	@rm -rf $(BUILD_DIR)

conan_d:
	@echo "[Make] Installing Conan dependencies (Debug)..."
	rm -rf $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)
	mkdir -p build_log
	rm -f build_log/build_result.txt
	bash -c 'set -o pipefail; (cd $(BUILD_DIR) && conan install .. -s build_type=Debug --output-folder=. --build missing -s compiler.cppstd=17) 2>&1 | tee build_log/build_result.txt'

conan_r:
	@echo "[Make] Installing Conan dependencies (Release)..."
	rm -rf $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)
	(cd $(BUILD_DIR) && conan install .. -s build_type=Release --output-folder=. --build missing -s compiler.cppstd=17)

configure: conan_d
	@echo "[Make] Configuring CMake..."
	cmake $(CMAKE_FLAGS)

build:
	@echo "[Make] Building project..."
	cmake --build $(BUILD_DIR) -- -j$(NPROCS)

run:
	@echo "[Make] Running simulation..."
	./$(BUILD_DIR)/testbench/tb_uart

format:
	cmake --build $(BUILD_DIR) --target run_clang_format