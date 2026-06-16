#!/usr/bin/env bash
# OWASP ZAP launcher — configurable via environment variables
set -euo pipefail

ZAP_HOME="${ZAP_HOME:-${HOME}/.local/share/zap-desk/zap}"
ZAP_CONFIG_DIR="${ZAP_CONFIG_DIR:-${HOME}/.local/share/zap-desk/config/home}"
CHROMEDRIVER="${CHROMEDRIVER:-$ZAP_CONFIG_DIR/webdriver/linux/64/chromedriver}"

if [ -x /snap/chromium/current/usr/lib/chromium-browser/chrome ]; then
  CHROME_BIN="/snap/chromium/current/usr/lib/chromium-browser/chrome"
elif [ -x /snap/bin/chromium ]; then
  CHROME_BIN="/snap/bin/chromium"
else
  CHROME_BIN="${CHROME_BIN:-/usr/bin/google-chrome}"
fi

mkdir -p "$ZAP_CONFIG_DIR"

export PATH="${PATH:-}"

if [ ! -x "$ZAP_HOME/zap.sh" ]; then
  echo ">> ERROR: ZAP not found at $ZAP_HOME/zap.sh" >&2
  echo ">> Run: make update-zap  (or set ZAP_HOME)" >&2
  exit 1
fi

exec "$ZAP_HOME/zap.sh" \
  -dir "$ZAP_CONFIG_DIR" \
  -config ajaxSpider.browserId=chrome \
  -config "selenium.chromeBinary=$CHROME_BIN" \
  -config "selenium.chromedriver=$CHROMEDRIVER" \
  -config "selenium.chromeArgs.arg(0).argument=--no-sandbox" \
  -config "selenium.chromeArgs.arg(1).argument=--disable-dev-shm-usage" \
  -config "selenium.chromeArgs.arg(3).argument=--disable-software-rasterizer" \
  -config connection.proxyChain.enabled=false \
  "$@"
