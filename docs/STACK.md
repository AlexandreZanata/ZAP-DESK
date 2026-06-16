# Stack Técnica — ZAP-DESK

## Desktop (C++)

| Componente | Versão | Uso |
|------------|--------|-----|
| C++ | 20 | Linguagem principal |
| Qt6 | 6.x | Widgets, Network, Process |
| CMake | 3.16+ | Build system |

## Recon Engine (Python)

| Componente | Versão | Uso |
|------------|--------|-----|
| Python | 3.10+ | Runtime reconner |
| Click | 8+ | CLI |
| Rich | 10+ | Progress UI (CLI) |
| Jinja2 | 3+ | Relatórios |

## Ferramentas Externas (Reconner)

| Ferramenta | Função |
|------------|--------|
| subfinder | Subdomínios |
| httpx | Hosts vivos |
| nmap | Portas/serviços |
| whatweb | Fingerprint |
| gobuster | Diretórios |
| nuclei | Vulnerabilidades |

## OWASP ZAP

- REST API JSON em `http://127.0.0.1:8080`
- Atualização via GitHub Releases (`zaproxy/zap`)
- Launcher: `scripts/zap-launch.sh`

## Dependências de Sistema

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
