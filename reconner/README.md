<div align="center">

<img src="https://img.shields.io/badge/version-2.0.0-blue?style=for-the-badge" />
<img src="https://img.shields.io/badge/python-3.10+-3776AB?style=for-the-badge&logo=python&logoColor=white" />
<img src="https://img.shields.io/badge/license-MIT-green?style=for-the-badge" />
<img src="https://img.shields.io/badge/platform-Linux%20%7C%20macOS-lightgrey?style=for-the-badge" />
<img src="https://img.shields.io/badge/PRs-welcome-brightgreen?style=for-the-badge" />

# 🔍 Reconner

An automated, modular security reconnaissance orchestrator for authorized penetration testing.

[Features](#-features) · [Installation](#-installation) · [Usage](#️-usage) · [Architecture](#️-architecture) · [Contributing](#-contributing) · [License](#-license)

</div>

---

## ⚠️ Legal Disclaimer

This tool is for **AUTHORIZED security testing ONLY**. Only use Reconner against systems you own or have explicit written permission to test. Unauthorized scanning is illegal and may violate computer fraud laws in your jurisdiction. The authors assume **zero liability** for misuse. You are solely responsible for your actions.

---

## 📖 Overview

Reconner is an open-source, modular security reconnaissance framework that orchestrates industry-standard tools into a single, streamlined workflow. It eliminates manual tool chaining, normalizes output formats, and generates professional reports — letting you focus on findings, not plumbing.

```
Target Domain
     │
     ├── subfinder   → Subdomain discovery
     ├── httpx       → Live host probing
     ├── whatweb     → Technology fingerprinting
     ├── gobuster    → Directory & file enumeration
     └── nuclei      → Vulnerability scanning
                              │
                    ┌─────────▼──────────┐
                    │  Reconner Engine   │
                    │  (Orchestration +  │
                    │   Normalization)   │
                    └─────────┬──────────┘
                              │
          ┌───────────────────┼───────────────────┐
          ▼                   ▼                   ▼
      report.pdf         summary.json         report.md
```

---

## ✨ Features

| Feature | Description |
|---------|-------------|
| 🤖 Smart Orchestration | Runs tools in the correct dependency order automatically |
| 🔄 Parallel Execution | Multi-threaded scanning for maximum speed |
| 📊 Unified Reports | JSON, Markdown, and PDF output from a single run |
| 🛡️ Stealth Mode | Rate-limited, low-noise scanning for sensitive targets |
| 🔌 Modular Design | Enable/disable any tool or phase independently |
| 📡 Proxy Support | Full proxy chain support (Burp Suite, SOCKS5, etc.) |
| 🎯 Multi-Target | Scan single domains or bulk target lists |
| 📈 Live Progress | Real-time progress bars and live result streaming |
| 🔁 Report Regeneration | Re-export reports from cached scan data |

---

## 📁 Project Structure

```
reconner/
├── reconner/                  # Core package
│   ├── __init__.py
│   ├── __main__.py            # Entry point (python -m reconner)
│   ├── cli.py                 # CLI argument parsing
│   ├── runner.py              # Tool orchestration engine
│   ├── parsers.py             # Output parsers (per tool)
│   ├── reporter.py            # Report generation (MD / PDF / JSON)
│   ├── progress.py            # Progress UI (Rich)
│   ├── utils.py               # Helpers & validators
│   └── templates/
│       └── report.md.j2       # Jinja2 report template
│
├── tests/
│   ├── __init__.py
│   └── test_cli_smoke.py
│
├── wordlists/                 # Custom wordlists (or symlinks)
│
├── .github/
│   ├── workflows/
│   │   └── ci.yml             # GitHub Actions CI
│   └── ISSUE_TEMPLATE/
│       ├── bug_report.md
│       └── feature_request.md
│
├── docs/
│   ├── installation.md
│   ├── usage.md
│   └── contributing.md
│
├── example-run.sh             # Quick demo script
├── install-tools.sh           # Dependency installer
├── requirements.txt
├── requirements-dev.txt       # Dev dependencies (pytest, black, etc.)
├── setup.py
├── CHANGELOG.md
├── CONTRIBUTING.md
├── CODE_OF_CONDUCT.md
├── SECURITY.md
└── README.md
```

---

## 🚀 Installation

### Prerequisites

| Requirement | Version | Notes |
|-------------|---------|-------|
| Python | 3.10+ | Core runtime |
| Go | 1.20+ | Required to install security tools |
| Git | Any | For cloning |

### Quick Install

```bash
# 1. Clone the repository
git clone https://github.com/yourusername/reconner.git
cd reconner

# 2. Install security tool dependencies
chmod +x install-tools.sh
./install-tools.sh

# 3. Install Python package
pip install -e .

# 4. Verify
reconner --version
```

### Manual Tool Installation

<details>
<summary>Click to expand</summary>

```bash
# subfinder — Subdomain discovery
go install -v github.com/projectdiscovery/subfinder/v2/cmd/subfinder@latest
sudo mv ~/go/bin/subfinder /usr/local/bin/

# httpx — HTTP probing
go install -v github.com/projectdiscovery/httpx/cmd/httpx@latest
sudo mv ~/go/bin/httpx /usr/local/bin/

# gobuster — Directory enumeration
go install github.com/OJ/gobuster/v3@latest
sudo mv ~/go/bin/gobuster /usr/local/bin/

# nuclei — Vulnerability scanner
go install -v github.com/projectdiscovery/nuclei/v3/cmd/nuclei@latest
sudo mv ~/go/bin/nuclei /usr/local/bin/

# whatweb — Technology fingerprinting
sudo apt install whatweb   # Debian/Ubuntu

# SecLists — Wordlists
sudo apt install seclists
# OR:
sudo git clone --depth 1 https://github.com/danielmiessler/SecLists /usr/share/seclists
```

</details>

### Development Install

```bash
pip install -r requirements-dev.txt
pre-commit install
```

---

## 🛠️ Usage

### Basic Scan

```bash
reconner --target example.com
```

### Common Patterns

```bash
# Fast recon (small wordlists, skip nuclei)
reconner -t example.com --fast

# Full scan with proxy
reconner -t example.com --proxy http://127.0.0.1:8080

# Bulk targets
reconner --input-file targets.txt --output-dir ./results

# Stealth mode (slow, low-noise)
reconner -t example.com --stealth

# Skip vulnerability scanning
reconner -t example.com --skip-nuclei

# Regenerate reports from cached data
reconner --export-only --output-dir ./results
```

### All Options

```
Usage: reconner [OPTIONS]

Options:
  -t, --target TEXT           Single target domain
  -i, --input-file PATH       File containing targets (one per line)
  -o, --output-dir PATH       Output directory  [default: ./results]
  -w, --wordlists-dir PATH    Wordlists directory  [default: /usr/share/seclists]
      --threads INT           Thread count  [default: 20]
      --proxy TEXT            Proxy URL (e.g. http://127.0.0.1:8080)
      --stealth               Low-noise mode (slower, randomized timing)
      --only-live             Skip hosts that don't respond to HTTP
      --skip-nuclei           Skip vulnerability scanning phase
      --fast                  Fast mode (reduced wordlists & templates)
      --export-only           Re-generate reports from existing results
  -v, --verbose               Verbose output
  -q, --quiet                 Suppress progress output
      --version               Show version and exit
      --help                  Show this message and exit
```

---

## 📂 Output

```
results/
└── example.com - 08_05_2026 - 7:30pm/
    ├── summary.json        # Structured scan data (subdomains, hosts, vulns)
    ├── report.md           # Full Markdown report
    ├── report.pdf          # Professional PDF report
    ├── highlights.txt      # Executive summary
    ├── reconner.log        # Full execution log
    └── raw/
        ├── subfinder.json
        ├── httpx.json
        ├── whatweb.json
        ├── gobuster.txt
        └── nuclei.json
```

---

## ⏱️ Estimated Scan Times

| Phase | Fast Mode | Normal | Full |
|-------|-----------|--------|------|
| Subdomain discovery | ~1 min | ~3 min | ~5 min |
| Live host probing | ~1 min | ~5 min | ~10 min |
| Tech fingerprinting | ~1 min | ~2 min | ~3 min |
| Dir enumeration | ~3 min | ~15 min | ~30 min/host |
| Vuln scanning | skipped | ~15 min | ~60 min |
| **Total** | **~6 min** | **~40 min** | **~2 hrs** |

---

## 🏗️ Architecture

Reconner follows a pipeline architecture — each phase feeds into the next:

```
CLI Input
  └─► Config Validation
        └─► Runner (Orchestrator)
              ├─► Phase 1: subfinder  ──► parsers.py ──► normalized data
              ├─► Phase 2: httpx      ──► parsers.py ──► normalized data
              ├─► Phase 3: whatweb    ──► parsers.py ──► normalized data
              ├─► Phase 4: gobuster   ──► parsers.py ──► normalized data
              └─► Phase 5: nuclei     ──► parsers.py ──► normalized data
                                                              │
                                                        reporter.py
                                                              │
                                          ┌───────────────────────────────┐
                                          │  summary.json                 │
                                          │  report.md  (Jinja2)          │
                                          │  report.pdf (weasyprint/RL)   │
                                          └───────────────────────────────┘
```

### Key Design Principles

- **Each tool is isolated** — failures in one phase don't abort the entire scan
- **All output is normalized** — parsers convert raw tool output to a common schema
- **Reports are decoupled** — `reporter.py` reads from `summary.json`, not tool outputs directly
- **Config-driven** — tool paths, wordlists, and flags are injected, not hardcoded

---

## 🧪 Testing

```bash
# Run all tests
pytest

# With coverage
pytest --cov=reconner --cov-report=html

# Smoke tests only
pytest tests/test_cli_smoke.py -v
```

---

## 🤝 Contributing

Contributions are welcome! Please read [CONTRIBUTING.md](CONTRIBUTING.md) before submitting a PR.

```bash
# 1. Fork and clone
git clone https://github.com/yourusername/reconner.git

# 2. Create a feature branch
git checkout -b feat/your-feature-name

# 3. Install dev dependencies
pip install -r requirements-dev.txt

# 4. Make changes, write tests
pytest

# 5. Format and lint
black reconner/
ruff reconner/

# 6. Submit PR
```

### Areas Where Help is Needed

- [ ] Windows support
- [ ] Docker image
- [ ] Additional parser modules (amass, nmap, ffuf)
- [ ] Web dashboard UI
- [ ] CI/CD pipeline hardening
- [ ] More nuclei template filters

---

## 🔐 Security Policy

Found a vulnerability in Reconner itself? Please do **not** open a public issue. See [SECURITY.md](SECURITY.md) for responsible disclosure instructions.

---

## 📄 Changelog

See [CHANGELOG.md](CHANGELOG.md) for version history.

---

## 📜 License

MIT License — see [LICENSE](LICENSE) for details.

---

## 🙏 Acknowledgements

Reconner is built on top of exceptional open-source tools:

- [subfinder](https://github.com/projectdiscovery/subfinder) by ProjectDiscovery
- [httpx](https://github.com/projectdiscovery/httpx) by ProjectDiscovery
- [nuclei](https://github.com/projectdiscovery/nuclei) by ProjectDiscovery
- [gobuster](https://github.com/OJ/gobuster) by OJ Reeves
- [WhatWeb](https://github.com/urbanadventurer/WhatWeb) by Andrew Horton
- [SecLists](https://github.com/danielmiessler/SecLists) by Daniel Miessler

---

<div align="center">

Made with ❤️ for the security community · Use responsibly

⭐ Star this repo if it helped you!

</div>
