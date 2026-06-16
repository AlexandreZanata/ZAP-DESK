#!/usr/bin/env bash
# Build a Flatpak bundle (requires flatpak + flatpak-builder + KDE runtime)
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
MANIFEST="$ROOT/packaging/flatpak/com.zapdesk.ZAPDesk.yml"
BUILD_DIR="${BUILD_DIR:-$ROOT/build/flatpak}"
REPO_DIR="$BUILD_DIR/repo"
BUNDLE="$BUILD_DIR/ZAP-DESK.flatpak"

if ! command -v flatpak-builder >/dev/null 2>&1; then
  echo ">> ERROR: flatpak-builder not found. Install: sudo apt install flatpak-builder" >&2
  exit 1
fi

echo ">> Installing KDE Flatpak runtime (if needed)..."
flatpak install -y flathub org.kde.Platform//6.7 org.kde.Sdk//6.7 2>/dev/null || true

mkdir -p "$BUILD_DIR"
flatpak-builder --force-clean --repo="$REPO_DIR" "$BUILD_DIR/build" "$MANIFEST"
flatpak build-bundle "$REPO_DIR" "$BUNDLE" com.zapdesk.ZAPDesk

echo ""
echo ">> Flatpak bundle: $BUNDLE"
echo ">> Install: flatpak install --user \"$BUNDLE\""
echo ">> Run:     flatpak run com.zapdesk.ZAPDesk"
