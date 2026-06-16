#!/usr/bin/env bash
# Build an AppDir bundle (portable layout; requires Qt6 libs on target system)
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT/build}"
APPDIR="$BUILD_DIR/ZAP-DESK.AppDir"
VERSION="$(grep -oP 'project\(ZAP-DESK VERSION \K[0-9.]+' "$ROOT/CMakeLists.txt" || echo "0.7.0")"

mkdir -p "$BUILD_DIR"
cmake -S "$ROOT" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR" -j"$(nproc)"

rm -rf "$APPDIR"
mkdir -p "$APPDIR/usr/bin" \
         "$APPDIR/usr/share/zap-desk/scripts" \
         "$APPDIR/usr/share/applications" \
         "$APPDIR/usr/share/icons/hicolor/scalable/apps"

cp "$BUILD_DIR/zap-desk" "$APPDIR/usr/bin/"
cp -a "$ROOT/reconner" "$APPDIR/usr/share/zap-desk/"
cp "$ROOT/scripts/zap-launch.sh" "$ROOT/scripts/update-zap.sh" "$APPDIR/usr/share/zap-desk/scripts/"
chmod +x "$APPDIR/usr/share/zap-desk/scripts/"*.sh
cp "$ROOT/packaging/zap-desk.desktop" "$APPDIR/usr/share/applications/"
cp "$ROOT/packaging/icons/zap-desk.svg" "$APPDIR/usr/share/icons/hicolor/scalable/apps/"
cp "$ROOT/packaging/icons/zap-desk.svg" "$APPDIR/zap-desk.svg"

cat > "$APPDIR/AppRun" <<'EOF'
#!/bin/sh
HERE="$(dirname "$(readlink -f "$0")")"
export PATH="$HERE/usr/bin:$PATH"
exec "$HERE/usr/bin/zap-desk" "$@"
EOF
chmod +x "$APPDIR/AppRun"

if command -v appimagetool >/dev/null 2>&1; then
  ARCH=x86_64 appimagetool "$APPDIR" "$BUILD_DIR/ZAP-DESK-${VERSION}-x86_64.AppImage"
  echo ">> AppImage: $BUILD_DIR/ZAP-DESK-${VERSION}-x86_64.AppImage"
else
  echo ">> AppDir ready at $APPDIR"
  echo ">> Install appimagetool to produce a single AppImage file."
fi
