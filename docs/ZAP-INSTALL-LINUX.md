# Complete guide: install and configure OWASP ZAP on Linux (ZAP-DESK)

This guide covers installing **OWASP ZAP** on Linux, configuring it for use with **ZAP-DESK**, and integrating with **Reconner** (proxy + pipeline).

> **Legal requirement:** use ZAP and Reconner only on targets you own or have written authorization to test.

---

## Table of contents

1. [Overview](#overview)
2. [System requirements](#system-requirements)
3. [Installing ZAP](#installing-zap)
4. [Recommended directory layout](#recommended-directory-layout)
5. [Configuration for ZAP-DESK](#configuration-for-zap-desk)
6. [Chromium and ChromeDriver (AJAX Spider)](#chromium-and-chromedriver-ajax-spider)
7. [Environment variables](#environment-variables)
8. [Testing that ZAP works](#testing-that-zap-works)
9. [Reconner integration](#reconner-integration)
10. [Updating ZAP](#updating-zap)
11. [Troubleshooting](#troubleshooting)
12. [Quick reference](#quick-reference)

---

## Overview

ZAP-DESK controls OWASP ZAP in two ways:

| Layer | File / component | Role |
|-------|------------------|------|
| Process | `scripts/zap-launch.sh` | Start/stop ZAP in daemon mode |
| REST API | `src/core/ZapClient.cpp` | Spider, active scan, alerts |
| UI | `src/ui/MainWindow.cpp` | BOOT ZAP, scans, updates buttons |
| Recon | `src/services/ReconRunner.cpp` | Routes Reconner traffic through ZAP proxy |

Typical flow:

```
BOOT ZAP (port 8080) → RUN RECON (--proxy) → FEED ZAP → ACTIVE SCAN
```

See also: [ZAP + Reconner integration](INTEGRATION.md) and [Development roadmap](ROADMAP.md).

---

## System requirements

### Required

| Package | Version | Reason |
|---------|---------|--------|
| **Java (JRE/JDK)** | 17+ | OWASP ZAP is written in Java |
| **curl** | any | Download and API tests |
| **tar**, **unzip** | any | Extract ZAP package |

```bash
# Debian / Ubuntu / Pop!_OS
sudo apt update
sudo apt install -y openjdk-17-jre curl tar unzip
java -version
```

### Recommended (AJAX Spider and Reconner integration)

| Package | Reason |
|---------|--------|
| **Chromium** or **Google Chrome** | AJAX Spider (JavaScript-heavy sites) |
| **ChromeDriver** | Compatible with installed Chromium version |
| **Reconner tools** | subfinder, httpx, nmap, etc. — see [reconner/docs/installation.md](../reconner/docs/installation.md) |

```bash
# Chromium via snap (common on Ubuntu/Pop!_OS)
sudo snap install chromium

# Or Google Chrome
wget https://dl.google.com/linux/direct/google-chrome-stable_current_amd64.deb
sudo apt install -y ./google-chrome-stable_current_amd64.deb
```

### Suggested minimum hardware

- **RAM:** 4 GB (8 GB+ recommended for active scan + Reconner)
- **Disk:** ~500 MB for ZAP + ~1 GB for SecLists/wordlists

---

## Installing ZAP

### Method 1 — Project script (recommended)

ZAP-DESK includes a script that downloads the latest official GitHub release:

```bash
cd /path/to/ZAP-DESK

# Set where ZAP will be installed (choose a permanent path)
export ZAP_HOME="$HOME/.local/share/zap-desk/zap"
mkdir -p "$(dirname "$ZAP_HOME")"

# Install
chmod +x scripts/update-zap.sh
./scripts/update-zap.sh
```

Verify:

```bash
ls "$ZAP_HOME/zap.sh"
"$ZAP_HOME/zap.sh" -version
```

### Method 2 — Manual download

1. Visit: https://github.com/zaproxy/zap/releases/latest
2. Download the **Linux** file (`.tar.gz`, e.g. `ZAP_2.16.1_Linux.tar.gz`)
3. Extract and move to the installation directory:

```bash
export ZAP_HOME="$HOME/.local/share/zap-desk/zap"
mkdir -p /tmp/zap-extract

tar -xzf ZAP_*.tar.gz -C /tmp/zap-extract
ZAP_DIR=$(find /tmp/zap-extract -name "zap.sh" -printf '%h\n' | head -1)

rm -rf "$ZAP_HOME"
cp -a "$ZAP_DIR" "$ZAP_HOME"
chmod +x "$ZAP_HOME/zap.sh"
```

### Method 3 — Via ZAP-DESK UI

After building the app:

1. Open `zap-desk`
2. **[ ZAP ]** tab → **CHECK ZAP UPDATE**
3. Confirm execution of `scripts/update-zap.sh`

> The script uses `$ZAP_HOME` — set the variable first if you are not using the default.

### What we do NOT use by default

- **ZAP Snap/Flatpak:** may work, but the project launcher (`scripts/zap-launch.sh`) expects the official tarball with `zap.sh` in `$ZAP_HOME`.
- **Docker:** useful for CI, but the ZAP-DESK desktop app expects ZAP locally on the machine.

---

## Recommended directory layout

```
~/.local/share/zap-desk/
├── zap/                          # ZAP_HOME — OWASP ZAP binaries
│   ├── zap.sh
│   ├── zap.jar
│   └── ...
└── config/
    └── home/                     # ZAP_CONFIG_DIR — sessions, certs, webdriver
        └── webdriver/linux/64/
            └── chromedriver

~/WebstormProjects/ZAP-DESK/      # ZAP_DESK_HOME — repository
├── scripts/zap-launch.sh
├── scripts/update-zap.sh
├── reconner/
└── build/zap-desk
```

Separate **binaries** (`ZAP_HOME`) from **data/config** (`ZAP_CONFIG_DIR`) to simplify updates without losing configuration.

---

## Configuration for ZAP-DESK

### 1. Execute permission on scripts

```bash
cd /path/to/ZAP-DESK
chmod +x scripts/zap-launch.sh scripts/update-zap.sh
```

### 2. Persistent environment variables

Add to `~/.bashrc` or `~/.profile`:

```bash
# ZAP-DESK — OWASP ZAP
export ZAP_DESK_HOME="$HOME/WebstormProjects/ZAP-DESK"
export ZAP_HOME="$HOME/.local/share/zap-desk/zap"
export ZAP_CONFIG_DIR="$HOME/.local/share/zap-desk/config/home"
export ZAP_LAUNCH_SCRIPT="$ZAP_DESK_HOME/scripts/zap-launch.sh"
export ZAP_API_PORT=8080
```

Reload:

```bash
source ~/.bashrc
```

### 3. How the launcher works

The project `scripts/zap-launch.sh`:

- Runs `$ZAP_HOME/zap.sh`
- Uses `-dir "$ZAP_CONFIG_DIR"` for persistent data
- Configures Chromium + ChromeDriver for the **AJAX Spider**
- Forwards extra arguments (`-daemon`, `-port`, etc.)

`ZapDaemon` (C++) invokes `scripts/zap-launch.sh` with security-aware flags:

| Profile | Daemon flags |
|---------|----------------|
| **Dev mode** (Settings, no API key) | `-config api.disablekey=true` |
| **Production** (API key set) | `-config api.key=<your-key>` |
| **Production** (no key) | No disablekey — configure key in ZAP-DESK Settings |

| Flag | Meaning |
|------|---------|
| `-daemon` | No ZAP GUI (headless) |
| `-port 8080` | REST API on port 8080 |
| `api.disablekey=true` | Local API without key (**dev/localhost only**) |
| `api.key=...` | ZAP API key for authenticated requests |

> **Production:** disable dev mode in **File → Settings**, set a ZAP API key, and restart ZAP. `ZapClient` sends `apikey=` on every REST call. Override via `ZAP_API_KEY` environment variable.

### 4. Build and run ZAP-DESK

```bash
cd "$ZAP_DESK_HOME"
make install-deps
make install-reconner
make build
make dev
```

In the UI: **[ ZAP ]** tab → **BOOT ZAP** → wait for status `ONLINE`.

---

## Chromium and ChromeDriver (AJAX Spider)

The **AJAX Spider** needs a real browser. Without it, the AJAX spider fails (HTTP active scan still works).

### Install Chromium

```bash
# Snap (Ubuntu / Pop!_OS)
sudo snap install chromium
```

`zap-launch.sh` auto-detects:

1. `/snap/chromium/current/usr/lib/chromium-browser/chrome`
2. `/snap/bin/chromium`
3. `/usr/bin/google-chrome` (fallback)

### Install ChromeDriver

ChromeDriver version must be compatible with the installed Chromium.

**Option A — Let ZAP download (recommended on first run)**

The first time the AJAX Spider runs, ZAP may download the driver to:

```
$ZAP_CONFIG_DIR/webdriver/linux/64/chromedriver
```

**Option B — Manual**

```bash
# Check Chromium version
chromium --version
# or: google-chrome --version

# Download matching chromedriver from:
# https://googlechromelabs.github.io/chrome-for-testing/

mkdir -p "$ZAP_CONFIG_DIR/webdriver/linux/64"
curl -fsSL -o "$ZAP_CONFIG_DIR/webdriver/linux/64/chromedriver" \
  "CHROMEDRIVER_LINUX64_URL"
chmod +x "$ZAP_CONFIG_DIR/webdriver/linux/64/chromedriver"
```

**Option C — Environment variable**

```bash
export CHROMEDRIVER="$ZAP_CONFIG_DIR/webdriver/linux/64/chromedriver"
export CHROME_BIN="/snap/bin/chromium"
```

### Test AJAX Spider via API

```bash
# With ZAP running in daemon mode
curl "http://127.0.0.1:8080/JSON/core/view/version/"
curl "http://127.0.0.1:8080/JSON/core/action/accessUrl/?url=https://example.com"
curl "http://127.0.0.1:8080/JSON/ajaxSpider/action/scan/?url=https://example.com"
curl "http://127.0.0.1:8080/JSON/ajaxSpider/view/status/"
```

---

## Environment variables

| Variable | Project default | Description |
|----------|-----------------|-------------|
| `ZAP_DESK_HOME` | auto-detect | ZAP-DESK repository root |
| `ZAP_HOME` | `/data/dev/tools/zap` * | Directory containing `zap.sh` |
| `ZAP_CONFIG_DIR` | `/data/dev/tools/config/zap/home` * | ZAP data and webdriver |
| `ZAP_LAUNCH_SCRIPT` | `scripts/zap-launch.sh` | Boot script |
| `ZAP_API_PORT` | `8080` | REST API port |
| `ZAP_DESK_PID_FILE` | XDG runtime / `.zap-desk.pid` | Daemon process PID |
| `ZAP_DESK_RESULTS` | XDG AppData | Reconner results |
| `CHROME_BIN` | auto-detect | Chromium/Chrome binary |
| `CHROMEDRIVER` | `$ZAP_CONFIG_DIR/webdriver/...` | ChromeDriver |

\* Code defaults point to `/data/dev/tools/...`. **Set `ZAP_HOME` and `ZAP_CONFIG_DIR` in your `~/.bashrc`** for paths under `$HOME` (see section above).

`AppConfig` (C++) reads these variables at runtime — no rebuild needed after changing env.

---

## Testing that ZAP works

### 1. Start manually (terminal)

```bash
# Dev / localhost (no API key):
"$ZAP_LAUNCH_SCRIPT" -daemon -port 8080 -config api.disablekey=true

# Production (with API key):
"$ZAP_LAUNCH_SCRIPT" -daemon -port 8080 -config api.key=YOUR_KEY_HERE
```
```

### 2. Verify API

```bash
curl -s "http://127.0.0.1:8080/JSON/core/view/version/" | python3 -m json.tool
```

Expected response:

```json
{
    "version": "2.16.1"
}
```

### 3. Stop ZAP

```bash
# If started by ZAP-DESK, use KILL ZAP in the UI
# Or find the PID:
pgrep -f "zap.jar"
kill <PID>
```

### 4. ZAP-DESK checklist

| Step | Expected result |
|------|-----------------|
| BOOT ZAP | Log: `ZAP started in daemon mode` |
| Status | `>> STATUS: ONLINE — ZAP x.x.x` |
| REFRESH ALERTS | Empty table or alerts listed |
| AJAX SPIDER | Status changes to `AJAX SPIDER RUNNING` |
| CHECK ZAP UPDATE | Shows local version vs GitHub |

---

## Reconner integration

Reconner lives in the same repository under `reconner/`. With ZAP running, recon HTTP traffic passes through the local proxy.

### Proxy during recon

In the ZAP-DESK UI, **[ RECON ]** tab:

- Check **Route via ZAP proxy (127.0.0.1:8080)**
- Click **RUN RECON**

CLI equivalent:

```bash
cd "$ZAP_DESK_HOME/reconner"
PYTHONPATH=. python3 -m reconner -t example.com \
  --proxy "http://127.0.0.1:8080" \
  --quiet -o "$HOME/.local/share/ZAP-DESK/results"
```

Tools that honor `--proxy`: subfinder, httpx, whatweb, gobuster, nuclei.

### Full Pipeline

1. **BOOT ZAP**
2. **FULL PIPELINE** (RECON tab) — runs recon → feeds ZAP from `summary.json` → active scan

### Wordlists (gobuster)

Reconner uses wordlists for directory enumeration. Install SecLists:

```bash
sudo apt install seclists
# or
sudo git clone --depth 1 https://github.com/danielmiessler/SecLists.git /usr/share/seclists
```

Details: [reconner/wordlists/README.md](../reconner/wordlists/README.md)

---

## Updating ZAP

OWASP ZAP publishes regular releases at https://github.com/zaproxy/zap/releases

### Via terminal

```bash
export ZAP_HOME="$HOME/.local/share/zap-desk/zap"
cd "$ZAP_DESK_HOME"
make update-zap
```

The script:

1. Queries the latest GitHub release
2. Backs up `$ZAP_HOME` to `$ZAP_HOME.bak.TIMESTAMP`
3. Downloads and extracts the new package
4. Preserves `$ZAP_CONFIG_DIR` (separate config)

### Via UI

**[ ZAP ]** → **CHECK ZAP UPDATE** → confirm download.

### After updating

```bash
"$ZAP_HOME/zap.sh" -version
# Restart ZAP in the UI (KILL ZAP → BOOT ZAP)
```

---

## Troubleshooting

### `Error: could not start ZAP`

```bash
# Verify paths
echo "ZAP_HOME=$ZAP_HOME"
test -x "$ZAP_HOME/zap.sh" && echo "zap.sh OK" || echo "zap.sh MISSING"
test -x "$ZAP_LAUNCH_SCRIPT" && echo "launcher OK" || echo "launcher MISSING"

# Test Java
java -version

# Run manually to see the error
"$ZAP_LAUNCH_SCRIPT" -daemon -port 8080
```

### Port 8080 in use

```bash
sudo ss -tlnp | grep 8080
# Change port:
export ZAP_API_PORT=8090
# Restart ZAP and reconfigure Reconner proxy to :8090
```

### API returns error / ZAP "OFFLINE"

- Wait 10–30 s after BOOT ZAP (JVM startup is slow)
- Enable **dev mode** in Settings if you have no API key, or set `ZAP_API_KEY` / configure key in Settings
- Test: `curl http://127.0.0.1:8080/JSON/core/view/version/` (add `?apikey=...` in production)

### AJAX Spider does not start

- Install Chromium: `sudo snap install chromium`
- Check ChromeDriver at `$ZAP_CONFIG_DIR/webdriver/linux/64/`
- See logs in the ZAP-DESK **SYSTEM LOG** panel

### Reconner does not use the proxy

- ZAP must be **ONLINE** before recon
- **Route via ZAP proxy** checkbox must be checked
- Proxy port = `ZAP_API_PORT` (default 8080)

### Permission denied on `/data/dev/tools/`

Use paths under `$HOME` ([Environment variables](#environment-variables) section) — the `/data/dev/...` defaults are from the original development environment.

---

## Quick reference

```bash
# Install ZAP
export ZAP_HOME="$HOME/.local/share/zap-desk/zap"
./scripts/update-zap.sh

# Configure env (add to ~/.bashrc)
export ZAP_DESK_HOME="$HOME/WebstormProjects/ZAP-DESK"
export ZAP_CONFIG_DIR="$HOME/.local/share/zap-desk/config/home"
export ZAP_LAUNCH_SCRIPT="$ZAP_DESK_HOME/scripts/zap-launch.sh"

# Start ZAP manually (dev mode)
"$ZAP_LAUNCH_SCRIPT" -daemon -port 8080 -config api.disablekey=true
# Or with API key: -config api.key=YOUR_KEY_HERE

# Test API
curl -s "http://127.0.0.1:8080/JSON/core/view/version/"

# Build and open ZAP-DESK
cd "$ZAP_DESK_HOME" && make build && make dev
```

### Related links

- [Main README](../README.md)
- [ZAP + Reconner integration](INTEGRATION.md)
- [Technical stack](STACK.md)
- [Development roadmap](ROADMAP.md)
- [Reconner installation](../reconner/docs/installation.md)
- [Reconner wordlists](../reconner/wordlists/README.md)
- [OWASP ZAP — official documentation](https://www.zaproxy.org/docs/)
- [ZAP releases on GitHub](https://github.com/zaproxy/zap/releases)
