#!/usr/bin/env bash
# Phase 2 Integration Layer — automated validation
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

PASS=0
FAIL=0

ok()   { echo "  [PASS] $1"; PASS=$((PASS + 1)); }
bad()  { echo "  [FAIL] $1"; FAIL=$((FAIL + 1)); }

section() { echo ""; echo "== $1 =="; }

section "Project structure"
for f in \
  src/services/ReconRunner.cpp \
  src/services/ReconBridge.cpp \
  src/services/ReconSummary.cpp \
  src/services/ReconPreflight.cpp \
  src/services/ZapUpdater.cpp \
  scripts/zap-launch.sh \
  scripts/update-zap.sh \
  reconner/reconner/runner.py \
  docs/ROADMAP.md \
  docs/PHASE2-VALIDATION.md; do
  if [ -f "$f" ]; then ok "$f"; else bad "missing $f"; fi
done

section "AppConfig defaults (no /data/dev hardcode in source)"
if grep -q '/data/dev/tools/zap' src/config/AppConfig.cpp; then
  bad "AppConfig.cpp still hardcodes /data/dev/tools/zap"
else
  ok "AppConfig uses XDG dataDir"
fi

section "ZAP launcher defaults"
if grep -q '${HOME}/.local/share/zap-desk' scripts/zap-launch.sh; then
  ok "zap-launch.sh uses ~/.local/share/zap-desk"
else
  bad "zap-launch.sh missing XDG default"
fi

section "Reconner ZAP_DESK_MODE progress hook"
if grep -q '_desk_phase' reconner/reconner/runner.py && grep -q 'ZAP_DESK_MODE' reconner/reconner/runner.py; then
  ok "runner.py emits desk progress"
else
  bad "runner.py missing ZAP_DESK_MODE progress"
fi

section "Python / Reconner"
if command -v python3 >/dev/null; then
  ok "python3 found"
  if python3 -c "import sys; sys.path.insert(0,'$ROOT/reconner'); import reconner" 2>/dev/null; then
    ok "reconner importable"
  else
    bad "reconner not importable — run: make install-reconner"
  fi
  if [ -d "$ROOT/reconner/tests" ]; then
    if (cd reconner && python3 -m pytest tests/ -q --tb=no 2>/dev/null); then
      ok "reconner pytest"
    else
      bad "reconner pytest failed"
    fi
  fi
else
  bad "python3 not found"
fi

section "Preflight script logic"
PREFLIGHT_OUT=$(python3 -c "
import sys
sys.path.insert(0, '$ROOT/reconner')
from reconner.utils import check_tool_exists
tools = ['subfinder','httpx','nmap','whatweb','gobuster','nuclei']
missing = [t for t in tools if not check_tool_exists(t)[0]]
print('MISSING:' + ','.join(missing) if missing else 'OK')
" 2>/dev/null || echo "ERROR")
if [ "$PREFLIGHT_OUT" = "OK" ]; then
  ok "all recon tools on PATH"
elif [[ "$PREFLIGHT_OUT" == MISSING:* ]]; then
  echo "  [WARN] Some tools missing: ${PREFLIGHT_OUT#MISSING:}"
  echo "         Install via: cd reconner && ./install-tools.sh"
  ok "preflight script runs (tools may be missing on CI)"
else
  bad "preflight script error: $PREFLIGHT_OUT"
fi

section "Summary fixture"
if [ -f tests/fixtures/summary-sample.json ]; then
  ok "summary fixture exists"
  if python3 -c "
import json
d=json.load(open('tests/fixtures/summary-sample.json'))
assert d['statistics']['total_subdomains']==3
assert d['live_hosts']['total']==2
"; then
    ok "summary fixture valid JSON"
  else
    bad "summary fixture invalid"
  fi
else
  bad "missing tests/fixtures/summary-sample.json"
fi

section "CMake build"
if command -v cmake >/dev/null && command -v g++ >/dev/null; then
  if pkg-config --exists Qt6Widgets 2>/dev/null || [ -d /usr/include/x86_64-linux-gnu/qt6 ]; then
    mkdir -p build
    if (cd build && cmake .. -DCMAKE_BUILD_TYPE=Release >/dev/null && cmake --build . -j"$(nproc)" >/dev/null); then
      ok "cmake build"
      if [ -x build/zap-desk ]; then ok "zap-desk binary"; else bad "binary missing"; fi
    else
      bad "cmake build failed"
    fi
  else
    echo "  [SKIP] Qt6 dev packages not installed"
  fi
else
  echo "  [SKIP] cmake/g++ not installed"
fi

section "ZAP installation (optional)"
if [ -x "${ZAP_HOME:-$HOME/.local/share/zap-desk/zap}/zap.sh" ]; then
  ok "ZAP installed at ${ZAP_HOME:-$HOME/.local/share/zap-desk/zap}"
else
  echo "  [WARN] ZAP not installed — run: make update-zap"
fi

echo ""
echo "================================"
echo "Phase 2 validation: $PASS passed, $FAIL failed"
echo "================================"

if [ "$FAIL" -gt 0 ]; then
  exit 1
fi
