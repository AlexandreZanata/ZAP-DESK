#!/usr/bin/env bash
# Download and install the latest OWASP ZAP release for Linux
set -euo pipefail

ZAP_HOME="${ZAP_HOME:-/data/dev/tools/zap}"
TMP_DIR="${TMPDIR:-/tmp}/zap-desk-update"
API_URL="https://api.github.com/repos/zaproxy/zap/releases/latest"

echo ">> ZAP-DESK: checking latest OWASP ZAP release..."

mkdir -p "$TMP_DIR" "$ZAP_HOME"

LATEST_JSON=$(curl -fsSL "$API_URL")
TAG=$(echo "$LATEST_JSON" | grep -oP '"tag_name":\s*"\K[^"]+')
DOWNLOAD_URL=$(echo "$LATEST_JSON" | grep -oP '"browser_download_url":\s*"\K[^"]+Linux[^"]+\.tar\.gz')

if [ -z "$DOWNLOAD_URL" ]; then
  echo ">> ERROR: Linux package not found in release $TAG"
  exit 1
fi

ARCHIVE="$TMP_DIR/zap-${TAG}.tar.gz"
echo ">> Downloading $TAG from $DOWNLOAD_URL"
curl -fsSL "$DOWNLOAD_URL" -o "$ARCHIVE"

BACKUP="${ZAP_HOME}.bak.$(date +%Y%m%d%H%M%S)"
if [ -d "$ZAP_HOME" ] && [ "$(ls -A "$ZAP_HOME" 2>/dev/null)" ]; then
  echo ">> Backup at $BACKUP"
  cp -a "$ZAP_HOME" "$BACKUP"
fi

EXTRACT_DIR="$TMP_DIR/extract"
rm -rf "$EXTRACT_DIR"
mkdir -p "$EXTRACT_DIR"
tar -xzf "$ARCHIVE" -C "$EXTRACT_DIR"

ZAP_DIR=$(find "$EXTRACT_DIR" -maxdepth 2 -name "zap.sh" -printf '%h\n' | head -1)
if [ -z "$ZAP_DIR" ]; then
  echo ">> ERROR: zap.sh not found in package"
  exit 1
fi

rm -rf "$ZAP_HOME"
mkdir -p "$(dirname "$ZAP_HOME")"
cp -a "$ZAP_DIR" "$ZAP_HOME"
chmod +x "$ZAP_HOME/zap.sh"

echo ">> OWASP ZAP $TAG installed at $ZAP_HOME"
rm -rf "$TMP_DIR"
