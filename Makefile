.PHONY: dev build test test-unit test-reconner test-integration test-domain validate-phase2 validate-phase4 validate-phase7 lint install-deps install-reconner install-all package-deb package-appimage update-zap clean

BUILD_DIR := build
CMAKE_FLAGS := -DCMAKE_BUILD_TYPE=Release

dev: build
	./$(BUILD_DIR)/zap-desk

build:
	@mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake .. $(CMAKE_FLAGS) && cmake --build . -j$$(nproc)

test: build test-unit test-reconner

test-unit: build
	cd $(BUILD_DIR) && ctest --output-on-failure -R zap-desk-unit-tests

test-reconner:
	cd reconner && python3 -m pytest tests/ -q $$(python3 -c "import pytest_cov" 2>/dev/null && echo "--cov=reconner --cov-report=term-missing")

test-integration: build
	cd $(BUILD_DIR) && cmake .. $(CMAKE_FLAGS) -DZAP_DESK_ENABLE_INTEGRATION_TESTS=ON && cmake --build . -j$$(nproc)
	cd $(BUILD_DIR) && ctest --output-on-failure -R zap-desk-integration-tests || true

test-domain: test-unit

lint:
	@command -v ruff >/dev/null && cd reconner && ruff check reconner/ || true
	@command -v clang-format >/dev/null && find src -name '*.cpp' -o -name '*.hpp' | xargs clang-format --dry-run -Werror || true

install-deps:
	./scripts/install-deps.sh

install-reconner:
	./scripts/install-reconner.sh

install-all:
	chmod +x scripts/install-all.sh
	./scripts/install-all.sh

package-deb:
	chmod +x scripts/package-deb.sh
	./scripts/package-deb.sh

package-appimage:
	chmod +x scripts/package-appimage.sh
	./scripts/package-appimage.sh

update-zap:
	./scripts/update-zap.sh

validate-phase2:
	./scripts/validate-phase2.sh

validate-phase4:
	chmod +x scripts/validate-phase4.sh
	./scripts/validate-phase4.sh

validate-phase7:
	chmod +x scripts/validate-phase7.sh
	./scripts/validate-phase7.sh

clean:
	rm -rf $(BUILD_DIR)
