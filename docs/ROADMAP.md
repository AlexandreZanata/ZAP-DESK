# ZAP-DESK Development Roadmap

Phased plan for building the unified ZAP-DESK + Reconner security terminal.

**Current phase: Phase 3 (Domain & Architecture)** — Integration Layer complete; validate with [PHASE2-VALIDATION.md](PHASE2-VALIDATION.md).

---

## Phase Overview

| Phase | Name | Status | Goal |
|-------|------|--------|------|
| 0 | Foundation | ✅ Done | Monorepo, git, reconner vendored, docs scaffold |
| 1 | ZAP Desktop Shell | ✅ Done | Qt UI, ZapClient, ZapDaemon, 90s hacker theme |
| 2 | Integration Layer | ✅ Done | ReconRunner, ReconBridge, ZapUpdater, Full Pipeline |
| 3 | Domain & Architecture | 🔄 Current | Clean architecture, Result type, entities |
| 4 | Testing & CI | ⏳ Pending | GoogleTest, expanded pytest, coverage gates |
| 5 | UI Components | ⏳ Pending | Reusable widgets, unified findings view |
| 6 | Security Hardening | ⏳ Pending | API key support, secure defaults |
| 7 | Packaging & Release | ⏳ Pending | AppImage/deb, installer, auto-update polish |

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

## Phase 3 — Domain & Architecture 🔄 (CURRENT)

**Planned:**
- `src/domain/` — `Scan`, `Target`, `Alert` entities with invariants
- `src/application/` — use cases (`StartReconUseCase`, `FeedZapUseCase`)
- `src/shared/result/` — `Result<T, E>` type (no throw in use cases)
- `src/infrastructure/` — repository implementations
- Refactor `MainWindow` to call use cases only (thin controller)

**Exit criteria:** No business logic in UI slots; domain layer testable without Qt GUI.

---

## Phase 4 — Testing & CI ⏳

**Planned:**
- `tests/unit/` — GoogleTest for ZapClient mocks, ReconBridge JSON parsing
- `tests/integration/` — ZAP API integration (optional, needs ZAP in CI)
- Expand `reconner/tests/` coverage
- Coverage thresholds in CI (80% target)
- `make test` runs C++ and Python suites

**Exit criteria:** CI green on every PR; core services have unit tests.

---

## Phase 5 — UI Components ⏳

**Planned:**
- `src/components/` — `AlertTable`, `LogConsole`, `ScanProgressBar`, `ReconSummaryPanel`
- Unified findings view (ZAP alerts + nuclei results from `summary.json`)
- Settings dialog (ZAP path, port, proxy, results dir)
- Scan history sidebar
- Optional: scanline CRT overlay effect

**Exit criteria:** Modular UI; operator can review all findings in one place.

---

## Phase 6 — Security Hardening ⏳

**Planned:**
- ZAP API key support in `ZapClient` and settings UI
- Remove `api.disablekey=true` from production profile
- Secure credential storage (optional keyring integration)
- Audit log for scan actions
- Rate limiting on recon launches

**Exit criteria:** Safe defaults for non-localhost use; API key documented.

---

## Phase 7 — Packaging & Release ⏳

**Planned:**
- AppImage or `.deb` package
- Desktop entry (`zap-desk.desktop`)
- Bundled install script for ZAP + recon tools
- Release workflow on GitHub (tag → build → attach artifacts)
- Subdomain discovery guide (English) in `docs/`

**Exit criteria:** One-command install for end users on Ubuntu/Pop!_OS.

---

## What to work on next (Phase 3 backlog)

Priority order:

1. **`src/shared/result/`** — `Result<T, E>` type
2. **`src/domain/`** — `Scan`, `Target`, `Alert` entities
3. **`src/application/`** — `StartReconUseCase`, `FeedZapUseCase`
4. Refactor `MainWindow` to thin controller
5. Operator sign-off: FULL PIPELINE on authorized target (closes Phase 2)

---

## Related docs

- [Architecture](ARCHITECTURE.md)
- [ZAP Install (Linux)](ZAP-INSTALL-LINUX.md)
- [Integration](INTEGRATION.md)
- [Stack](STACK.md)
