# Usage Guide

## Basic Scan

```bash
reconner --target example.com
```

---

## Common Patterns

```bash
# Fast recon (small wordlists, skip nuclei)
reconner -t example.com --fast

# Full scan with proxy
reconner -t example.com --proxy http://127.0.0.1:8080

# Bulk targets from file
reconner --input-file targets.txt --output-dir ./results

# Stealth mode (slow, low-noise)
reconner -t example.com --stealth

# Skip vulnerability scanning
reconner -t example.com --skip-nuclei

# Regenerate reports from cached data
reconner --export-only --output-dir ./results/example.com_2026-05-08_19-30
```

---

## All Options

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

## Output Structure

```
results/
└── example.com - 08_05_2026 - 7:30pm/
    ├── summary.json        # Structured scan data
    ├── report.md           # Full Markdown report
    ├── report.pdf          # Professional PDF report
    ├── highlights.txt      # Executive summary
    ├── discoveries.txt     # Key findings
    ├── reconner.log        # Full execution log
    └── raw/
        ├── subfinder.json
        ├── httpx.json
        ├── whatweb.json
        ├── gobuster.txt
        └── nuclei.json
```

---

## Estimated Scan Times

| Phase              | Fast Mode | Normal   | Full      |
|--------------------|-----------|----------|-----------|
| Subdomain discovery | ~1 min   | ~3 min   | ~5 min    |
| Live host probing   | ~1 min   | ~5 min   | ~10 min   |
| Tech fingerprinting | ~1 min   | ~2 min   | ~3 min    |
| Dir enumeration     | ~3 min   | ~15 min  | ~30 min/host |
| Vuln scanning       | skipped  | ~15 min  | ~60 min   |
| **Total**           | **~6 min** | **~40 min** | **~2 hrs** |
