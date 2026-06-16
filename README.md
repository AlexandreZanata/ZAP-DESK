# ZAP-DESK

Desktop security terminal for Linux — 90s hacker theme.

Unifies **OWASP ZAP** (spider, active scan, alerts) with **Reconner** (automated recon pipeline) in a single Qt6 interface.

```
╔══════════════════════════════════════════════════════════╗
║  ZAP-DESK // RECONNER — SECURITY TERMINAL v0.7.0         ║
║  [ OWASP ZAP + RECON PIPELINE ]  :: LINUX EDITION ::    ║
╚══════════════════════════════════════════════════════════╝
```

## Features

- Boot/stop OWASP ZAP in daemon mode
- AJAX Spider and Active Scan via REST API
- Reconner pipeline: subfinder → httpx → nmap → whatweb → gobuster → nuclei
- Automatic URL feed from `summary.json` into ZAP
- Full Pipeline: ZAP → Recon → Feed → Active Scan
- Unified findings view (ZAP + nuclei)
- API key support, audit log, recon rate limiting
- `.deb` package and desktop entry for Ubuntu/Pop!_OS
- Phosphorescent green CRT theme (90s)

## Requirements

- Linux (Ubuntu/Debian/Pop!_OS recommended)
- CMake 3.16+, GCC with C++20
- Qt6 (Widgets + Network)
- Python 3.10+
- **OWASP ZAP** — [complete Linux installation guide](docs/ZAP-INSTALL-LINUX.md)
- Recon tools (subfinder, httpx, nmap, etc.) — see `reconner/install-tools.sh`

## Quick install

### One command (recommended)

```bash
git clone git@github.com:AlexandreZanata/ZAP-DESK.git
cd ZAP-DESK
make install-all   # deps + reconner + ZAP + build
make dev           # run zap-desk
```

### From .deb package

```bash
make package-deb
sudo dpkg -i build/zap-desk_*.deb
zap-desk   # or launch from application menu
```

### Manual (development)

```bash
make install-deps
make install-reconner
make build
make test
make dev
```

## Environment variables

| Variable | Default | Description |
|----------|---------|-------------|
| `ZAP_DESK_HOME` | auto-detect | Project / share root |
| `ZAP_HOME` | `~/.local/share/zap-desk/zap` | ZAP installation |
| `ZAP_LAUNCH_SCRIPT` | `scripts/zap-launch.sh` | ZAP launcher |
| `ZAP_API_KEY` | — | ZAP REST API key |
| `ZAP_API_PORT` | `8080` | ZAP API port |
| `RECONNER_DIR` | `<root>/reconner` | Recon engine |
| `ZAP_DESK_RESULTS` | XDG AppData | Results directory |

## Structure

```
ZAP-DESK/
├── reconner/          # Python engine (Reconner v2.0.0)
├── src/               # Qt6/C++20 app
├── packaging/         # .desktop, icons, deb postinst
├── scripts/           # Build, install, package
├── docs/              # Architecture and integration
└── .cursor/rules/     # Cursor Agent rules
```

## Documentation

- [Install and configure OWASP ZAP on Linux](docs/ZAP-INSTALL-LINUX.md)
- [Subdomain discovery guide](docs/SUBDOMAIN-DISCOVERY.md)
- [Packaging & release](docs/PHASE7-PACKAGING.md)
- [Architecture](docs/ARCHITECTURE.md)
- [Stack](docs/STACK.md)
- [ZAP + Reconner integration](docs/INTEGRATION.md)
- [Development roadmap](docs/ROADMAP.md)

## Update OWASP ZAP

```bash
make update-zap
# or via UI: CHECK ZAP UPDATE
```

## Legal notice

This software is for **authorized security testing** only. The operator must have written authorization before scanning any target.

## License

ZAP-DESK: MIT (TBD)
Reconner: MIT — see `reconner/LICENSE`
