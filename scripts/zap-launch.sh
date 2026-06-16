#!/usr/bin/env bash
# OWASP ZAP launcher — configurável via variáveis de ambiente
set -euo pipefail

ZAP_HOME="${ZAP_HOME:-/data/dev/tools/zap}"
ZAP_CONFIG_DIR="${ZAP_CONFIG_DIR:-/data/dev/tools/config/zap/home}"
CHROMEDRIVER="${CHROMEDRIVER:-$ZAP_CONFIG_DIR/webdriver/linux/64/chromedriver}"

if [ -x /snap/chromium/current/usr/lib/chromium-browser/chrome ]; then
  CHROME_BIN="/snap/chromium/current/usr/lib/chromium-browser/chrome"
elif [ -x /snap/bin/chromium ]; then
  CHROME_BIN="/snap/bin/chromium"
else
  CHROME_BIN="${CHROME_BIN:-/usr/bin/google-chrome}"
fi

mkdir -p "$ZAP_CONFIG_DIR"

export PATH="/data/dev/tools/chromedriver:${PATH:-}"

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
