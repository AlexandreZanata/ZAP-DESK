.PHONY: dev build test lint install-deps install-reconner update-zap validate-phase2 clean

BUILD_DIR := build
CMAKE_FLAGS := -DCMAKE_BUILD_TYPE=Release

dev: build
	./$(BUILD_DIR)/zap-desk

build:
	@mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake .. $(CMAKE_FLAGS) && cmake --build . -j$$(nproc)

test:
	cd reconner && python3 -m pytest tests/ -q

lint:
	@command -v ruff >/dev/null && cd reconner && ruff check reconner/ || true
	@command -v clang-format >/dev/null && find src -name '*.cpp' -o -name '*.hpp' | xargs clang-format --dry-run -Werror || true

install-deps:
	./scripts/install-deps.sh

install-reconner:
	./scripts/install-reconner.sh

update-zap:
	./scripts/update-zap.sh

validate-phase2:
	./scripts/validate-phase2.sh

clean:
	rm -rf $(BUILD_DIR)
