#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="$ROOT/build"

echo "==> Dependências: cmake, qt6-base-dev, build-essential"
echo "    sudo apt install cmake qt6-base-dev qt6-tools-dev-tools build-essential"

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j"$(nproc)"

echo ""
echo "Build concluído: $BUILD_DIR/zap-desk"
echo "Execute: $BUILD_DIR/zap-desk"
