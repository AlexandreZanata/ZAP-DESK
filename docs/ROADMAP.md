# ZAP-DESK Development Roadmap

Phased plan for building the unified ZAP-DESK + Reconner security terminal.

**Current phase: Phase 8 complete** — v0.8.0 (keyring + Flatpak).

---

## Phase Overview

| Phase | Name | Status | Goal |
|-------|------|--------|------|
| 0 | Foundation | ✅ Done | Monorepo, git, reconner vendored, docs scaffold |
| 1 | ZAP Desktop Shell | ✅ Done | Qt UI, ZapClient, ZapDaemon, 90s hacker theme |
| 2 | Integration Layer | ✅ Done | ReconRunner, ReconBridge, ZapUpdater, Full Pipeline |
| 3 | Domain & Architecture | ✅ Done | Clean architecture, Result type, use cases, facade |
| 4 | Testing & CI | ✅ Done | GoogleTest, pytest coverage, CI pipeline |
| 5 | UI Components | ✅ Done | Reusable widgets, unified findings view |
| 6 | Security Hardening | ✅ Done | API key support, secure defaults |
| 7 | Packaging & Release | ✅ Done | AppImage/deb, installer, release workflow |
| 8 | Distribution & Secrets | ✅ Done | Keyring storage, Flatpak bundle |

---

## Phase 0 — Foundation ✅

**Delivered:**
- Git repo connected to `git@github.com:AlexandreZanata/ZAP-DESK.git`
- Reconner v2.0.0 vendored in `reconner/`
- CMake build, Makefile, CI workflow
- `.cursor/rules/` and `docs/` structure
- English-only documentation policy

---

## Phase 1 — ZAP Desktop Shell ✅

**Delivered:**
- `ZapClient` — OWASP ZAP REST API (version, spider, active scan, alerts)
- `ZapDaemon` — process lifecycle via `scripts/zap-launch.sh`
- `MainWindow` — hacker CRT theme, ZAP tab controls
- `AppConfig` — env-based paths (XDG)

**Exit criteria:** App builds, boots ZAP, runs scans, displays alerts.

---

## Phase 2 — Integration Layer ✅

**Delivered:**
- `ReconRunner` — spawns `python3 -m reconner` with proxy + `ZAP_DESK_MODE` progress
- `ReconBridge` — reads `summary.json`, seeds ZAP site tree
- `ReconSummary` — parses and displays scan stats in UI
- `ReconPreflight` — checks Python deps and external tools before recon
- `ZapUpdater` — GitHub release check + `scripts/update-zap.sh`
- RECON tab, FULL PIPELINE, authorization checkbox
- XDG defaults (`~/.local/share/zap-desk/`) in AppConfig and scripts
- [ZAP install guide](ZAP-INSTALL-LINUX.md)
- [Phase 2 validation checklist](PHASE2-VALIDATION.md) + `scripts/validate-phase2.sh`

**Completed items:**
- [x] Fix default `ZAP_HOME` paths (XDG `dataDir()`)
- [x] Recon progress indicator (`@@ZAP-DESK@@PHASE` protocol)
- [x] Display recon results summary in UI
- [x] Preflight error handling for missing deps/tools
- [x] End-to-end manual test checklist documented
- [ ] Verify FULL PIPELINE on real authorized target *(operator sign-off)*

**Validate:** `make validate-phase2` then follow [PHASE2-VALIDATION.md](PHASE2-VALIDATION.md).

**Exit criteria:** Full pipeline works reliably on a clean Linux install following the install guide.

---

## Phase 3 — Domain & Architecture ✅

**Delivered:**
- `shared/result/Result.hpp` + `AppError` / `DomainError`
- Domain: `TargetUrl`, `ScanId`, `RiskLevel`, `Scan`, `SecurityAlert`
- Ports: `IReconGateway`, `IZapGateway`, `IPreflightGateway`, `IReconSummaryReader`
- Use cases: `StartRecon`, `FeedZap`, `RunFullPipeline`, `StartZapScan`
- Infrastructure: `QtReconGateway`, `QtZapGateway`, `QtPreflightGateway`, `JsonReconSummaryReader`
- `ApplicationFacade` — presentation layer; `MainWindow` is thin controller
- Domain unit tests: `tests/unit/test_domain.cpp`
- [Phase 3 architecture guide](PHASE3-ARCHITECTURE.md)

**Exit criteria:** No business logic in UI slots; domain testable without Qt GUI. ✅

---

## Phase 4 — Testing & CI ✅

**Delivered:**
- GoogleTest via CMake FetchContent (`zap-desk-unit-tests`)
- Unit tests: `TargetUrl`, use cases, `Scan`, `RiskLevel`, `JsonReconSummaryReader`
- Optional ZAP integration test (`ZAP_DESK_ENABLE_INTEGRATION_TESTS`)
- Reconner: `test_zap_desk_runner.py` + `pytest.ini` coverage gate (70%)
- CI: `test-cpp` (ctest), `test-reconner` (coverage), `validate-phase2`
- `make test`, `make test-unit`, `make validate-phase4`
- [Phase 4 testing guide](PHASE4-TESTING.md)

**Exit criteria:** CI green on every PR; core services have unit tests. ✅

---

## Phase 5 — UI Components ✅

**Delivered:**
- `src/components/` — `LogConsole`, `FindingsTable`, `ScanProgressBar`, `ReconSummaryPanel`, `StatusBanner`, `ScanHistorySidebar`, `SettingsDialog`, `CrtOverlay`, `HackerTheme`
- Unified findings view (ZAP alerts + nuclei from `summary.json`)
- Settings dialog (ZAP home, API port, results/reconner paths, CRT toggle)
- Scan history sidebar
- CRT scanline overlay (optional)
- [Phase 5 UI guide](PHASE5-UI.md)

**Exit criteria:** Modular UI; operator can review all findings in one place. ✅

---

## Phase 6 — Security Hardening ✅

**Delivered:**
- `ZapClient` API key on all REST calls (`apikey` query param)
- `CredentialStore` — env `ZAP_API_KEY` + QSettings persistence
- `ZapDaemon` production profile — `api.disablekey=true` only in dev mode
- Settings UI: API key, dev mode toggle, recon rate limit
- `AuditLogger` — `~/.local/share/zap-desk/audit.log`
- `ReconRateLimiter` — configurable cooldown (default 30s)
- Unit tests: `test_zap_client_auth`, `test_audit_logger`, `test_recon_rate_limiter`
- [Phase 6 security guide](PHASE6-SECURITY.md)

**Exit criteria:** Safe defaults for non-localhost use; API key documented. ✅

---

## Phase 7 — Packaging & Release ✅

**Delivered:**
- `.deb` package via CPack (`make package-deb`)
- AppDir bundle (`make package-appimage`)
- Desktop entry `packaging/zap-desk.desktop` + SVG icon
- `scripts/install-all.sh` — one-command install from source
- `AppConfig` FHS layout detection (`/usr/share/zap-desk`)
- GitHub release workflow (tag `v*` → `.deb` + AppDir archive)
- [Subdomain discovery guide](SUBDOMAIN-DISCOVERY.md)
- [Phase 7 packaging guide](PHASE7-PACKAGING.md)

**Exit criteria:** One-command install for end users on Ubuntu/Pop!_OS. ✅

---

## Phase 8 — Distribution & Secrets ✅

**Delivered:**
- `KeyringBackend` — libsecret via `secret-tool` (GNOME Keyring / KWallet)
- `CredentialStore` — keyring + QSettings with auto-migration
- Settings UI: **Store API key in system keyring**
- Flatpak manifest `packaging/flatpak/com.zapdesk.ZAPDesk.yml`
- `make package-flatpak`, `make validate-phase8`
- [Phase 8 distribution guide](PHASE8-DISTRIBUTION.md)

**Exit criteria:** API key storable outside plaintext config; Flatpak build documented. ✅

---

## Roadmap complete

Phases 0–8 delivered. Future work: bundled Qt AppImage, Flathub publish, operator E2E sign-off.

---

## Related docs

- [Architecture](ARCHITECTURE.md)
- [ZAP Install (Linux)](ZAP-INSTALL-LINUX.md)
- [Integration](INTEGRATION.md)
- [Phase 5 UI](PHASE5-UI.md)
- [Phase 6 Security](PHASE6-SECURITY.md)
- [Phase 7 Packaging](PHASE7-PACKAGING.md)
- [Phase 8 Distribution](PHASE8-DISTRIBUTION.md)
- [Subdomain discovery](SUBDOMAIN-DISCOVERY.md)
- [Stack](STACK.md)
