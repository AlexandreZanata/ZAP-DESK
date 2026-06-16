# Technical Stack — ZAP-DESK

## Desktop (C++)

| Component | Version | Purpose |
|-----------|---------|---------|
| C++ | 20 | Primary language |
| Qt6 | 6.x | Widgets, Network, Process |
| CMake | 3.16+ | Build system |

## Recon Engine (Python)

| Component | Version | Purpose |
|-----------|---------|---------|
| Python | 3.10+ | Reconner runtime |
| Click | 8+ | CLI |
| Rich | 10+ | Progress UI (CLI) |
| Jinja2 | 3+ | Reports |

## External Tools (Reconner)

| Tool | Role |
|------|------|
| subfinder | Subdomains |
| httpx | Live hosts |
| nmap | Ports/services |
| whatweb | Fingerprinting |
| gobuster | Directories |
| nuclei | Vulnerabilities |

## OWASP ZAP

- JSON REST API at `http://127.0.0.1:8080`
- Updates via GitHub Releases (`zaproxy/zap`)
- Launcher: `scripts/zap-launch.sh`
- **Complete Linux guide:** [ZAP-INSTALL-LINUX.md](ZAP-INSTALL-LINUX.md)

## System Dependencies

```bash
# C++ / Qt
sudo apt install cmake qt6-base-dev qt6-tools-dev-tools build-essential

# Reconner tools
cd reconner && ./install-tools.sh
```

## Build

```bash
./scripts/install-deps.sh
./scripts/install-reconner.sh
./scripts/build.sh
./build/zap-desk
```

## Related docs

- [Architecture](ARCHITECTURE.md)
- [ZAP + Reconner integration](INTEGRATION.md)
- [Development roadmap](ROADMAP.md)
