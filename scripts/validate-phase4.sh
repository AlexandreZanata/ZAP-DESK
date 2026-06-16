#!/usr/bin/env bash
# Phase 4 — Testing & CI validation
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

PASS=0
FAIL=0

ok()  { echo "  [PASS] $1"; PASS=$((PASS + 1)); }
bad() { echo "  [FAIL] $1"; FAIL=$((FAIL + 1)); }

section() { echo ""; echo "== $1 =="; }

section "Test layout"
for f in \
  tests/unit/test_target_url.cpp \
  tests/unit/test_usecases.cpp \
  tests/unit/test_scan_entity.cpp \
  tests/unit/test_risk_level.cpp \
  tests/unit/test_summary_reader.cpp \
  tests/integration/test_zap_api.cpp \
  reconner/tests/test_zap_desk_runner.py \
  reconner/pytest.ini; do
  if [ -f "$f" ]; then ok "$f"; else bad "missing $f"; fi
done

section "CMake + GoogleTest"
if command -v cmake >/dev/null && command -v g++ >/dev/null; then
  if pkg-config --exists Qt6Widgets 2>/dev/null || [ -d /usr/include/x86_64-linux-gnu/qt6 ]; then
    mkdir -p build
    if (cd build && cmake .. -DCMAKE_BUILD_TYPE=Debug >/dev/null && cmake --build . -j"$(nproc)" >/dev/null); then
      ok "cmake build (with unit tests)"
    else
      bad "cmake build failed"
    fi

    if [ -x build/zap-desk-unit-tests ]; then
      if (cd build && ctest --output-on-failure -R zap-desk-unit-tests); then
        ok "ctest unit suite"
      else
        bad "ctest unit suite failed"
      fi
    else
      bad "zap-desk-unit-tests binary missing"
    fi
  else
    echo "  [SKIP] Qt6 dev packages not installed"
  fi
else
  echo "  [SKIP] cmake/g++ not installed"
fi

section "Reconner pytest + coverage"
if command -v python3 >/dev/null; then
  if (cd reconner && python3 -m pip install -e ".[dev]" -q 2>/dev/null); then
    ok "reconner installed"
  else
    bad "reconner install failed"
  fi

  if python3 -c "import pytest_cov" 2>/dev/null; then
    COV_ARGS="--cov=reconner --cov-report=term-missing"
  else
    COV_ARGS=""
    echo "  [WARN] pytest-cov not installed — running without coverage"
  fi

  if (cd reconner && python3 -m pytest tests/ -q $COV_ARGS); then
    ok "reconner pytest"
  else
    bad "reconner pytest failed"
  fi
else
  bad "python3 not found"
fi

section "Makefile test target"
if grep -q 'ctest' Makefile && grep -q 'test-domain' Makefile; then
  ok "Makefile test targets"
else
  bad "Makefile missing test targets"
fi

echo ""
echo "================================"
echo "Phase 4 validation: $PASS passed, $FAIL failed"
echo "================================"

if [ "$FAIL" -gt 0 ]; then
  exit 1
fi
