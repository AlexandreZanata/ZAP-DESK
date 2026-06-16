# ZAP-DESK — Arquitetura Enterprise (C++/Linux)

> Monorepo unificado: **ZAP-DESK** (Qt6/C++20) + **Reconner** (Python 3.10+)
> Domínio: plataforma desktop de segurança ofensiva autorizada para Linux.

---

## CONTEXTO

ZAP-DESK é um terminal de segurança desktop com tema hacker anos 90 que orquestra:

1. **OWASP ZAP** — proxy, spider, active scan, alertas
2. **Reconner** — pipeline de recon (subfinder → httpx → nmap → whatweb → gobuster → nuclei)

O sistema deve parecer produzido por um time sênior — não um wrapper trivial.

---

## STACK OBRIGATÓRIA

| Camada | Tecnologia |
|--------|------------|
| **UI Desktop** | Qt6 Widgets (C++20) |
| **HTTP Client** | QNetworkAccessManager (ZAP REST API) |
| **Processos** | QProcess (ZAP daemon, Reconner, scripts shell) |
| **Recon Engine** | Python 3.10+ / Reconner 2.0.0 (vendored em `reconner/`) |
| **Build** | CMake 3.16+ |
| **Testes C++** | GoogleTest + Qt Test |
| **Testes Python** | pytest (reconner/tests/) |
| **Lint C++** | clang-tidy + clang-format |
| **Lint Python** | ruff + black + mypy |
| **Logging** | QTextEdit UI + journal estruturado (futuro: spdlog) |
| **Config** | AppConfig + variáveis de ambiente |
| **CI** | GitHub Actions |
| **Containerização** | Docker opcional para CI (reconner) |
| **Atualizações ZAP** | GitHub API + scripts/update-zap.sh |

---

## ESTRUTURA DE PASTAS

```
ZAP-DESK/
├── .cursor/rules/              # Regras Cursor Agent
├── docs/                       # Documentação arquitetural
├── reconner/                   # Engine Python (vendored)
│   ├── reconner/               # Pacote Python
│   ├── tests/
│   └── setup.py
├── scripts/
│   ├── build.sh
│   ├── install-deps.sh
│   ├── install-reconner.sh
│   ├── zap-launch.sh
│   └── update-zap.sh
├── src/
│   ├── config/
│   │   └── AppConfig.*         # Paths, env, XDG
│   ├── core/
│   │   ├── ZapClient.*         # Cliente REST ZAP
│   │   └── ZapDaemon.*         # Lifecycle ZAP
│   ├── domain/                 # Entidades puras (futuro)
│   ├── application/            # Use cases (futuro)
│   ├── infrastructure/         # Adaptadores (futuro)
│   ├── services/
│   │   ├── ReconRunner.*       # Orquestra reconner via QProcess
│   │   ├── ReconBridge.*       # summary.json → ZAP
│   │   └── ZapUpdater.*        # Auto-update ZAP
│   ├── ui/
│   │   └── MainWindow.*        # Interface hacker 90s
│   ├── components/             # Widgets reutilizáveis (futuro)
│   └── main.cpp
├── tests/                      # Testes C++ (futuro)
├── CMakeLists.txt
├── Makefile
└── README.md
```

---

## REGRAS DE DOMÍNIO

### Entidades (evolução planejada)

- Toda entidade de scan: `id`, `targetId`, `createdAt`, `updatedAt`, `status`, `version`
- Entidades **nunca** importam Qt Network ou QProcess diretamente
- Validação de invariantes no construtor — falha com `DomainError`

### Value Objects

- `TargetUrl` — URL normalizada com validação
- `ScanId` — ULID/UUID
- `RiskLevel` — enum tipado para alertas ZAP

### Domain Events

- `ScanStarted`, `ScanCompleted`, `ZapFed`, `AlertReceived`
- Publicados após operações bem-sucedidas via event bus interno

### Repositórios

- Interface em `domain/` — retorna entidades, não JSON bruto
- Implementação em `infrastructure/` — mapeia summary.json / ZAP API

---

## INTEGRAÇÃO ZAP + RECONNER

### Fluxo FULL PIPELINE

```
[BOOT ZAP] → [RUN RECON --proxy 127.0.0.1:8080] → [summary.json]
    → [FEED ZAP URLs] → [ACTIVE SCAN] → [ALERTS]
```

### Contrato de integração

O arquivo `summary.json` é o handshake entre Reconner e ZAP-DESK:

```json
{
  "live_hosts": { "list": ["https://..."] },
  "subdomains": { "list": ["..."] },
  "gobuster_results": [{ "url": "https://..." }]
}
```

### Atualizações automáticas ZAP

1. `ZapUpdater` consulta `api.github.com/repos/zaproxy/zap/releases/latest`
2. Compara versão local (ZAP API) vs release
3. Executa `scripts/update-zap.sh` sob confirmação do usuário

---

## SEGURANÇA

- Recon **somente** com checkbox de autorização escrita na UI
- ZAP local com `api.disablekey=true` apenas em dev — produção deve usar API key
- Nunca logar credenciais ou tokens
- Paths configuráveis via env — sem hardcode de paths de máquina

### Variáveis de ambiente

| Variável | Descrição |
|----------|-----------|
| `ZAP_DESK_HOME` | Raiz do projeto |
| `ZAP_HOME` | Instalação OWASP ZAP |
| `ZAP_LAUNCH_SCRIPT` | Script de boot do ZAP |
| `ZAP_API_PORT` | Porta API (default 8080) |
| `RECONNER_DIR` | Diretório do reconner |
| `ZAP_DESK_RESULTS` | Diretório de resultados |

---

## OBSERVABILIDADE

### Logging UI

- Timestamps em todo log da interface
- Prefixos `>>` estilo terminal CRT
- Cores por severidade via stylesheet

### Health Checks (futuro)

- ZAP API `/JSON/core/view/version/`
- Reconner `python3 -m reconner --version`
- Ferramentas externas via `check_tool_exists`

---

## TESTES — ESTRATÉGIA TDD

```
tests/           # C++ GoogleTest
reconner/tests/  # Python pytest
```

### Cobertura mínima alvo

- statements: 80%
- branches: 75%
- functions: 80%

### Para cada serviço C++:

- Teste unitário com mocks
- Teste do caminho feliz
- Teste de cada erro possível

---

## PADRÕES DE CÓDIGO

### Result Type (C++ — evolução)

```cpp
template<typename T, typename E = AppError>
class Result {
    bool ok_;
    T value_;
    E error_;
};
```

Use Cases retornam `Result<T>` — controllers interpretam para UI.

### Repository Pattern

```cpp
class ScanRepository {
public:
    virtual std::optional<Scan> findById(const ScanId& id) = 0;
    virtual void save(const Scan& scan) = 0;
};
```

---

## MAKEFILE TARGETS

```makefile
make dev          # build + run
make test         # pytest + ctest
make lint         # clang-format + ruff
make build        # cmake release
make install-deps # apt + reconner
make update-zap   # atualiza OWASP ZAP
```

---

## O QUE NÃO FAZER

- ❌ Lógica de negócio em slots de UI além de orquestração
- ❌ Paths hardcoded (usar AppConfig)
- ❌ `console.log` equivalente sem timestamp
- ❌ Executar recon sem confirmação de autorização
- ❌ Queries ZAP fora do ZapClient
- ❌ Deletar resultados de scan sem confirmação

---

## ADICIONAR NOVOS MÓDULOS

1. Criar interface em `domain/`
2. Implementar use case em `application/`
3. Adaptador em `infrastructure/` ou `services/`
4. Conectar na UI via sinais Qt
5. Escrever testes antes da implementação (TDD)
