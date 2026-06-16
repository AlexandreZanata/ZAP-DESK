# ZAP-DESK

Terminal de segurança desktop para Linux — tema hacker anos 90.

Unifica **OWASP ZAP** (spider, active scan, alertas) com **Reconner** (pipeline de recon automatizado) em uma única interface Qt6.

```
╔══════════════════════════════════════════════════════════╗
║  ZAP-DESK // RECONNER — SECURITY TERMINAL v0.2.0         ║
║  [ OWASP ZAP + RECON PIPELINE ]  :: LINUX EDITION ::    ║
╚══════════════════════════════════════════════════════════╝
```

## Funcionalidades

- Boot/stop do OWASP ZAP em modo daemon
- AJAX Spider e Active Scan via REST API
- Pipeline Reconner: subfinder → httpx → nmap → whatweb → gobuster → nuclei
- Feed automático de URLs do `summary.json` para o ZAP
- Full Pipeline: ZAP → Recon → Feed → Active Scan
- Verificação e instalação automática de atualizações do ZAP
- Tema CRT verde fosforescente (anos 90)

## Requisitos

- Linux (Ubuntu/Debian recomendado)
- CMake 3.16+, GCC com C++20
- Qt6 (Widgets + Network)
- Python 3.10+
- OWASP ZAP instalado
- Ferramentas de recon (subfinder, httpx, nmap, etc.) — ver `reconner/install-tools.sh`

## Instalação rápida

```bash
git clone <seu-repo> ZAP-DESK
cd ZAP-DESK

make install-deps      # Qt6, cmake, build-essential
make install-reconner  # pip install reconner
make build
make dev               # executa zap-desk
```

## Variáveis de ambiente

| Variável | Default | Descrição |
|----------|---------|-----------|
| `ZAP_DESK_HOME` | auto-detect | Raiz do projeto |
| `ZAP_HOME` | `/data/dev/tools/zap` | Instalação ZAP |
| `ZAP_LAUNCH_SCRIPT` | `scripts/zap-launch.sh` | Launcher ZAP |
| `ZAP_API_PORT` | `8080` | Porta API ZAP |
| `RECONNER_DIR` | `<root>/reconner` | Engine recon |
| `ZAP_DESK_RESULTS` | XDG AppData | Resultados |

## Estrutura

```
ZAP-DESK/
├── reconner/          # Engine Python (Reconner v2.0.0)
├── src/               # App Qt6/C++20
├── scripts/           # Build, ZAP launcher, update
├── docs/              # Arquitetura e integração
└── .cursor/rules/     # Regras Cursor Agent
```

## Documentação

- [Arquitetura](docs/ARCHITECTURE.md)
- [Stack](docs/STACK.md)
- [Integração ZAP + Reconner](docs/INTEGRATION.md)

## Atualizar OWASP ZAP

```bash
make update-zap
# ou via UI: CHECK ZAP UPDATE
```

## Aviso legal

Este software é para **testes de segurança autorizados** apenas. O operador deve possuir autorização escrita antes de escanear qualquer alvo.

## Licença

ZAP-DESK: MIT (a definir)
Reconner: MIT — ver `reconner/LICENSE`
