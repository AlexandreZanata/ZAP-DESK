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

if ! flatpak remote-list | grep -q flathub; then
  echo ">> Adding flathub remote..."
  flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo
fi

KDE_RUNTIME_VERSION="${KDE_RUNTIME_VERSION:-}"
if [ -z "$KDE_RUNTIME_VERSION" ]; then
  KDE_RUNTIME_VERSION=$(grep 'runtime-version:' "$MANIFEST" | sed -E 's/.*"([^"]+)".*/\1/')
fi

echo ">> Installing KDE Flatpak runtime ${KDE_RUNTIME_VERSION} (if needed)..."
flatpak install -y flathub "org.kde.Platform//${KDE_RUNTIME_VERSION}" "org.kde.Sdk//${KDE_RUNTIME_VERSION}" \
  2>/dev/null || flatpak install -y flathub org.kde.Platform//"${KDE_RUNTIME_VERSION}" org.kde.Sdk//"${KDE_RUNTIME_VERSION}"

mkdir -p "$BUILD_DIR"
flatpak-builder --force-clean --repo="$REPO_DIR" "$BUILD_DIR/build" "$MANIFEST"
flatpak build-bundle "$REPO_DIR" "$BUNDLE" com.zapdesk.ZAPDesk

echo ""
echo ">> Flatpak bundle: $BUNDLE"
echo ">> First install:  flatpak install --user \"$BUNDLE\""
echo ">> Update/reinstall: flatpak install --user --reinstall \"$BUNDLE\""
echo ">> Run:              flatpak run com.zapdesk.ZAPDesk"
