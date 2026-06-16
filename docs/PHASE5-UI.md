# Phase 5 — UI Components

Modular Qt widgets extracted from `MainWindow` for maintainability and a unified operator view.

## Components (`src/components/`)

| Component | Role |
|-----------|------|
| `HackerTheme` | Shared CRT stylesheet |
| `StatusBanner` | ASCII header + connection status |
| `LogConsole` | Timestamped system log |
| `FindingsTable` | Unified ZAP + nuclei findings table |
| `ScanProgressBar` | Recon phase label + progress bar |
| `ReconSummaryPanel` | Post-recon statistics line |
| `ScanHistorySidebar` | Recent scan targets with OK/FAIL |
| `SettingsDialog` | ZAP home, API port, paths, CRT toggle |
| `CrtOverlay` | Optional scanline overlay |

## Unified findings

`ApplicationFacade::refreshFindings()` loads:

1. **ZAP alerts** via REST API (`source: ZAP`)
2. **nuclei results** from the last `summary.json` (`source: nuclei`)

The **REFRESH FINDINGS** button and automatic refresh after recon populate `FindingsTable`.

## Settings persistence

Settings are stored in `QSettings` (`~/.config/ZAP-DESK/zap-desk.conf` on Linux). Environment variables still override saved values.

| Key | Purpose |
|-----|---------|
| `zap/home` | ZAP installation directory |
| `zap/port` | ZAP API port (default 8080) |
| `paths/results` | Recon output directory |
| `paths/reconner` | Reconner package root |
| `ui/crtOverlay` | Enable/disable scanline effect |

Open **File → Settings** in the menu bar.

## Exit criteria

- [x] Modular UI components in `src/components/`
- [x] Unified findings view (ZAP + nuclei)
- [x] Settings dialog
- [x] Scan history sidebar
- [x] CRT scanline overlay (toggle in settings)

**App version:** v0.5.0
