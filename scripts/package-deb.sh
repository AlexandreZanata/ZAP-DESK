#!/usr/bin/env bash
# Build a .deb package for ZAP-DESK
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT/build}"
CMAKE_FLAGS="${CMAKE_FLAGS:--DCMAKE_BUILD_TYPE=Release}"

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

cmake "$ROOT" $CMAKE_FLAGS
cmake --build . -j"$(nproc)"
cpack -G DEB

echo ""
echo ">> Package built:"
ls -1 "$BUILD_DIR"/*.deb
echo ""
echo ">> Install: sudo dpkg -i $BUILD_DIR/zap-desk_*.deb"
