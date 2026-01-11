BUILD_DIR = build

CMAKE_FLAGS = -S . -B $(BUILD_DIR) \
              -DCMAKE_BUILD_TYPE=Debug \
              -DUSE_CPM=ON

NPROCS = $(shell nproc 2>/dev/null || echo 1)

.PHONY: all clean configure build run format

# Default: clean -> configure -> build
all: clean configure build

clean:
	@echo "[Make] Cleaning build directory..."
	@rm -rf $(BUILD_DIR)

configure:
	@echo "[Make] Configuring with CPM..."
	@mkdir -p $(BUILD_DIR)
	cmake $(CMAKE_FLAGS)

build:
	@echo "[Make] Building project..."
	cmake --build $(BUILD_DIR) -- -j$(NPROCS)

run:
	@echo "[Make] Running simulation..."
	./$(BUILD_DIR)/testbench/tb_uart

format:
	cmake --build $(BUILD_DIR) --target run_clang_format