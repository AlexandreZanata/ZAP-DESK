#!/usr/bin/env bash
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

echo ">> Phase 8 validation"
echo ""

check "KeyringBackend exists" "[ -f src/security/KeyringBackend.hpp ]"
check "CredentialStore keyring API" "grep -q 'useKeyring' src/security/CredentialStore.hpp"
check "Settings keyring toggle" "grep -q 'm_useKeyring' src/components/SettingsDialog.hpp"
check "Flatpak manifest exists" "[ -f packaging/flatpak/com.zapdesk.ZAPDesk.yml ]"
check "package-flatpak.sh exists" "[ -f scripts/package-flatpak.sh ]"
check "credential store unit test" "[ -f tests/unit/test_credential_store.cpp ]"
check "Phase 8 docs exist" "[ -f docs/PHASE8-DISTRIBUTION.md ]"

echo ""
echo ">> $PASS passed, $FAIL failed"
[ "$FAIL" -eq 0 ]
