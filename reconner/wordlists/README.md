# Wordlists Directory

This directory is a placeholder for custom wordlists used by Reconner
(specifically by the gobuster directory enumeration phase).

By default, Reconner looks for wordlists in `/usr/share/seclists`.
Override this with the `--wordlists-dir` option.

---

## ZAP-DESK + OWASP ZAP (Linux)

This project bundles **Reconner** with **ZAP-DESK**, a Qt desktop app that
controls OWASP ZAP. For the Reconner proxy integration and full pipeline
(Recon → ZAP → Active Scan), you must install and configure ZAP first.

**Full Linux installation guide (ZAP + ZAP-DESK + Reconner proxy):**

→ **[docs/ZAP-INSTALL-LINUX.md](../../docs/ZAP-INSTALL-LINUX.md)**

That guide covers:

- Java, Chromium, ChromeDriver
- Installing ZAP via `scripts/update-zap.sh` or manual download
- Environment variables (`ZAP_HOME`, `ZAP_CONFIG_DIR`, `ZAP_LAUNCH_SCRIPT`)
- AJAX Spider setup
- Proxy mode: `reconner --proxy http://127.0.0.1:8080`
- Troubleshooting and updates

Quick start after ZAP is running:

```bash
# From ZAP-DESK UI: [ RECON ] → enable "Rotear via proxy ZAP" → RUN RECON
# Or CLI:
cd ../../reconner
PYTHONPATH=. python3 -m reconner -t example.com \
  --proxy http://127.0.0.1:8080 --quiet
```

---

## Recommended Layout

```
wordlists/
├── Discovery/
│   ├── Web-Content/
│   │   ├── common.txt                      # Small, fast — used in --fast mode
│   │   ├── directory-list-2.3-medium.txt   # Default for normal scans
│   │   ├── directory-list-2.3-big.txt      # Comprehensive (slow)
│   │   ├── raft-small-files.txt            # Sensitive file names
│   │   └── raft-small-directories.txt      # Common directory names
│   └── DNS/
│       ├── subdomains-top1million-5000.txt
│       └── subdomains-top1million-110000.txt
```

---

## Installing SecLists (Recommended)

```bash
# Debian / Ubuntu
sudo apt install seclists

# Or clone from GitHub (~1 GB)
sudo git clone --depth 1 https://github.com/danielmiessler/SecLists.git /usr/share/seclists
```

---

## Using a Custom Wordlist Directory

```bash
reconner --target example.com --wordlists-dir ./wordlists
```

Place your wordlists under `wordlists/Discovery/Web-Content/` to match
the default path resolution logic, or pass the full path directly.

---

## Related documentation

| Document | Description |
|----------|-------------|
| [ZAP install (Linux)](../../docs/ZAP-INSTALL-LINUX.md) | OWASP ZAP setup for this project |
| [Integration](../../docs/INTEGRATION.md) | Reconner ↔ ZAP pipeline |
| [Reconner installation](../docs/installation.md) | subfinder, httpx, nmap, nuclei, etc. |
