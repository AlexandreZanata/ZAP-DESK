#!/usr/bin/env bash
# One-command installer for ZAP-DESK on Ubuntu/Debian/Pop!_OS
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

echo "╔══════════════════════════════════════════════════════════╗"
echo "║  ZAP-DESK — full install (deps + reconner + ZAP + build)  ║"
echo "╚══════════════════════════════════════════════════════════╝"

"$ROOT/scripts/install-deps.sh"
"$ROOT/scripts/install-reconner.sh"

if [ ! -x "${ZAP_HOME:-$HOME/.local/share/zap-desk/zap}/zap.sh" ]; then
  echo ">> Installing OWASP ZAP..."
  "$ROOT/scripts/update-zap.sh"
else
  echo ">> OWASP ZAP already present at ${ZAP_HOME:-$HOME/.local/share/zap-desk/zap}"
fi

make build

echo ""
echo ">> Install complete."
echo "   Run from source:  make dev"
echo "   Or install .deb:  make package-deb && sudo dpkg -i build/zap-desk_*.deb"
echo "   Desktop entry:    packaging/zap-desk.desktop (installed with .deb)"
echo ""
echo "   See docs/ZAP-INSTALL-LINUX.md and docs/SUBDOMAIN-DISCOVERY.md"
