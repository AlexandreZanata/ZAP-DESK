# Subdomain Discovery Guide

How to discover subdomains with ZAP-DESK and Reconner on authorized targets.

> **Legal notice:** Only scan systems you own or have explicit written permission to test.

---

## Overview

Reconner's pipeline starts with **subfinder** to enumerate subdomains, then **httpx** probes live hosts. Results appear in `summary.json` and the ZAP-DESK UI after a recon run.

```
subfinder → httpx → nmap → whatweb → gobuster → nuclei
```

---

## Quick start (GUI)

1. Launch ZAP-DESK (`make dev` or from the application menu after `.deb` install).
2. Enter the **root domain** or URL (e.g. `https://example.com`).
3. Check **I confirm written authorization to test this target**.
4. Open the **[ RECON ]** tab.
5. Click **RUN RECON** (or **FULL PIPELINE** for ZAP + recon + active scan).
6. Review stats in **RECON SUMMARY** and subdomains in `summary.json`.

Default results directory: `~/.local/share/zap-desk/results/<target>/summary.json`

---

## Command line (Reconner only)

```bash
cd ZAP-DESK/reconner
python3 -m reconner -t example.com --fast
```

With ZAP proxy (traffic through ZAP):

```bash
export ZAP_DESK_MODE=1
python3 -m reconner -t example.com --proxy http://127.0.0.1:8080
```

---

## subfinder configuration

### API keys (optional, more results)

Create `~/.config/subfinder/provider-config.yaml` with API keys for passive sources (Shodan, Censys, VirusTotal, etc.). See [subfinder documentation](https://github.com/projectdiscovery/subfinder).

### Wordlist / resolvers

Reconner uses subfinder defaults. For custom resolvers:

```bash
# ~/.config/subfinder/config.yaml
resolvers:
  - 1.1.1.1
  - 8.8.8.8
```

---

## Reading subdomain results

### summary.json

```json
{
  "subdomains": {
    "total": 42,
    "list": ["example.com", "www.example.com", "api.example.com"]
  },
  "live_hosts": {
    "total": 12,
    "list": ["https://www.example.com", "https://api.example.com"]
  }
}
```

### ZAP-DESK UI

- **RECON SUMMARY** — subdomain and live-host counts
- **FINDINGS** — nuclei + ZAP alerts after refresh
- **SCAN HISTORY** — recent targets

---

## Tips for better coverage

| Technique | How |
|-----------|-----|
| Wildcard scope | Use root domain `example.com`, not only `www` |
| Fast vs full | `--fast` skips slower steps; disable for deeper scans |
| Skip nuclei | Use **Skip nuclei** for enumeration-only runs |
| Feed ZAP | **FEED ZAP** seeds discovered URLs into ZAP's site tree |
| Full pipeline | **FULL PIPELINE** = boot ZAP → recon → feed → active scan |

---

## Troubleshooting

| Issue | Fix |
|-------|-----|
| Zero subdomains | Check target spelling; verify subfinder: `subfinder -d example.com` |
| subfinder not found | Run `reconner/install-tools.sh` or `make install-reconner` |
| Preflight failed | Install missing tools listed in the error dialog |
| Rate limited | Wait for recon cooldown (Settings → recon rate limit) |

---

## Related docs

- [ZAP Install (Linux)](ZAP-INSTALL-LINUX.md)
- [Integration](INTEGRATION.md)
- [Reconner wordlists](../reconner/wordlists/README.md)
