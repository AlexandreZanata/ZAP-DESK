#!/usr/bin/env bash
# Phase 7 packaging validation (structure + scripts, no full dpkg required)
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

PASS=0
FAIL=0

check() {
  if eval "$2"; then
    echo "  OK   $1"
    PASS=$((PASS + 1))
  else
    echo "  FAIL $1"
    FAIL=$((FAIL + 1))
  fi
}

echo ">> Phase 7 validation"
echo ""

check "desktop entry exists" "[ -f packaging/zap-desk.desktop ]"
check "desktop entry references zap-desk" "grep -q 'Exec=zap-desk' packaging/zap-desk.desktop"
check "icon exists" "[ -f packaging/icons/zap-desk.svg ]"
check "debian postinst exists" "[ -f packaging/debian/postinst ]"
check "install-all.sh exists" "[ -x scripts/install-all.sh ] || chmod +x scripts/install-all.sh"
check "package-deb.sh exists" "[ -f scripts/package-deb.sh ]"
check "package-appimage.sh exists" "[ -f scripts/package-appimage.sh ]"
check "release workflow exists" "[ -f .github/workflows/release.yml ]"
check "subdomain guide exists" "[ -f docs/SUBDOMAIN-DISCOVERY.md ]"
check "CMake installs desktop file" "grep -q 'packaging/zap-desk.desktop' CMakeLists.txt"
check "CMake CPack DEB enabled" "grep -q 'CPACK_GENERATOR' CMakeLists.txt"

echo ""
echo ">> $PASS passed, $FAIL failed"
[ "$FAIL" -eq 0 ]
