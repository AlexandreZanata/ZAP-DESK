# Reconner + OWASP ZAP Integration

## Overview

ZAP-DESK unifies two complementary engines:

| Engine | Role | Depth |
|--------|------|-------|
| **Reconner** | Surface mapping | Broad and fast |
| **OWASP ZAP** | Application testing | Deep (spider, active scan) |

## Reconner Location

Reconner v2.0.0 is vendored in `reconner/` (origin: THE-ULTMATE-RECONNER).

```
reconner/
├── reconner/cli.py      # CLI entry point
├── reconner/runner.py   # ToolRunner — orchestration
├── reconner/reporter.py # summary.json
└── setup.py
```

## How ZAP-DESK Invokes Reconner

`ReconRunner` (C++) runs:

```bash
cd $RECONNER_DIR
PYTHONPATH=$RECONNER_DIR python3 -m reconner \
  -t <target> \
  -o <results_dir> \
  --quiet \
  [--fast] [--skip-nuclei] \
  --proxy http://127.0.0.1:8080
```

The `--quiet` flag skips the interactive prompt (the UI already requires an authorization checkbox).

## Bridge summary.json → ZAP

`ReconBridge` reads `summary.json` and extracts URLs from:

- `live_hosts.list`
- `subdomains.list`
- `gobuster_results[].url`

For each URL, it calls `ZapClient::accessUrl()` to populate the ZAP site tree.

## Operation Modes

### 1. Isolated recon
**[RECON]** tab → RUN RECON → results in `~/.local/share/ZAP-DESK/results/`

### 2. Manual feed
After recon → FEED ZAP → ACTIVE SCAN

### 3. Full Pipeline
BOOT ZAP → RECON (via proxy) → FEED → automatic ACTIVE SCAN

## Proxy Mode

With `--proxy http://127.0.0.1:8080`, HTTP traffic from httpx, gobuster, and nuclei passes through ZAP, enabling parallel manual inspection.

## ZAP Updates

1. **CHECK ZAP UPDATE** button on the ZAP tab
2. Queries GitHub API `zaproxy/zap/releases/latest`
3. Script `scripts/update-zap.sh` downloads and installs into `$ZAP_HOME`

Complete installation and configuration guide: **[ZAP-INSTALL-LINUX.md](ZAP-INSTALL-LINUX.md)**

Recommended: check for updates weekly (OWASP ZAP release cycle).

## summary.json Schema

```json
{
  "scan_info": { "timestamp", "target", "tool_versions" },
  "subdomains": { "total", "list" },
  "live_hosts": { "total", "list" },
  "gobuster_results": [{ "url", "path", "status" }],
  "nuclei_results": [{ "template", "severity", "url" }],
  "statistics": { "total_subdomains", "total_live_hosts" }
}
```

## Integration Roadmap

Planned work is tracked in **[ROADMAP.md](ROADMAP.md)** (Phase 2 and beyond). Highlights:

- [ ] Native ZAP phase in Python ToolRunner
- [ ] Merge ZAP + nuclei alerts in the UI
- [ ] Unified PDF export
- [ ] Scan scheduling
- [ ] Local D-Bus API for automation
