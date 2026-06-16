# Installation Guide

## Prerequisites

| Requirement | Version | Notes |
|-------------|---------|-------|
| Python      | 3.10+   | Core runtime |
| Go          | 1.20+   | Required to install security tools |
| Git         | Any     | For cloning |

---

## Quick Install

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

---

## Manual Tool Installation

### subfinder — Subdomain discovery

```bash
go install -v github.com/projectdiscovery/subfinder/v2/cmd/subfinder@latest
sudo mv ~/go/bin/subfinder /usr/local/bin/
```

### httpx — HTTP probing

```bash
go install -v github.com/projectdiscovery/httpx/cmd/httpx@latest
sudo mv ~/go/bin/httpx /usr/local/bin/
```

### gobuster — Directory enumeration

```bash
go install github.com/OJ/gobuster/v3@latest
sudo mv ~/go/bin/gobuster /usr/local/bin/
```

### nuclei — Vulnerability scanner

```bash
go install -v github.com/projectdiscovery/nuclei/v3/cmd/nuclei@latest
sudo mv ~/go/bin/nuclei /usr/local/bin/
```

### whatweb — Technology fingerprinting

```bash
# Debian/Ubuntu
sudo apt install whatweb
```

### SecLists — Wordlists

```bash
# Debian/Ubuntu
sudo apt install seclists

# Or clone from GitHub (~1 GB)
sudo git clone --depth 1 https://github.com/danielmiessler/SecLists /usr/share/seclists
```

---

## Development Install

```bash
pip install -r requirements-dev.txt
pre-commit install
```

---

## Verify All Tools

```bash
for tool in subfinder httpx whatweb gobuster nuclei; do
  which $tool && echo "$tool OK" || echo "$tool NOT FOUND"
done
```
