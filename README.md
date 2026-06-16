# ZAP-DESK

Desktop security terminal for Linux — 90s hacker theme.

Unifies **OWASP ZAP** (spider, active scan, alerts) with **Reconner** (automated recon pipeline) in a single Qt6 interface.

```
╔══════════════════════════════════════════════════════════╗
║  ZAP-DESK // RECONNER — SECURITY TERMINAL v0.3.0         ║
║  [ OWASP ZAP + RECON PIPELINE ]  :: LINUX EDITION ::    ║
╚══════════════════════════════════════════════════════════╝
```

## Features

- Boot/stop OWASP ZAP in daemon mode
- AJAX Spider and Active Scan via REST API
- Reconner pipeline: subfinder → httpx → nmap → whatweb → gobuster → nuclei
- Automatic URL feed from `summary.json` into ZAP
- Full Pipeline: ZAP → Recon → Feed → Active Scan
- Check and install OWASP ZAP updates automatically
- Phosphorescent green CRT theme (90s)

## Requirements

- Linux (Ubuntu/Debian/Pop!_OS recommended)
- CMake 3.16+, GCC with C++20
- Qt6 (Widgets + Network)
- Python 3.10+
- **OWASP ZAP** — [complete Linux installation guide](docs/ZAP-INSTALL-LINUX.md)
- Recon tools (subfinder, httpx, nmap, etc.) — see `reconner/install-tools.sh`

## Quick install

```bash
git clone <your-repo> ZAP-DESK
cd ZAP-DESK

make install-deps      # Qt6, cmake, build-essential
make install-reconner  # pip install reconner
make build
make test              # GoogleTest + reconner pytest
make dev               # run zap-desk
```

## Environment variables

| Variable | Default | Description |
|----------|---------|-------------|
| `ZAP_DESK_HOME` | auto-detect | Project root |
| `ZAP_HOME` | `~/.local/share/zap-desk/zap` | ZAP installation |
| `ZAP_LAUNCH_SCRIPT` | `scripts/zap-launch.sh` | ZAP launcher |
| `ZAP_API_PORT` | `8080` | ZAP API port |
| `RECONNER_DIR` | `<root>/reconner` | Recon engine |
| `ZAP_DESK_RESULTS` | XDG AppData | Results directory |

## Structure

```
ZAP-DESK/
├── reconner/          # Python engine (Reconner v2.0.0)
├── src/               # Qt6/C++20 app
├── scripts/           # Build, ZAP launcher, update
├── docs/              # Architecture and integration
└── .cursor/rules/     # Cursor Agent rules
```

## Documentation

- [Install and configure OWASP ZAP on Linux](docs/ZAP-INSTALL-LINUX.md)
- [Architecture](docs/ARCHITECTURE.md)
- [Stack](docs/STACK.md)
- [ZAP + Reconner integration](docs/INTEGRATION.md)
- [Development roadmap](docs/ROADMAP.md)
- [Phase 4 testing](docs/PHASE4-TESTING.md)
- [Phase 2 validation](docs/PHASE2-VALIDATION.md)

## Update OWASP ZAP

See also: [docs/ZAP-INSTALL-LINUX.md](docs/ZAP-INSTALL-LINUX.md#updating-zap)

```bash
make update-zap
# or via UI: CHECK ZAP UPDATE
```

## Legal notice

This software is for **authorized security testing** only. The operator must have written authorization before scanning any target.

## License

ZAP-DESK: MIT (TBD)
Reconner: MIT — see `reconner/LICENSE`
