# ZAP-DESK — Enterprise Architecture (C++/Linux)

> Unified monorepo: **ZAP-DESK** (Qt6/C++20) + **Reconner** (Python 3.10+)
> Domain: authorized offensive security desktop platform for Linux.

---

## CONTEXT

ZAP-DESK is a 90s hacker-themed desktop security terminal that orchestrates:

1. **OWASP ZAP** — proxy, spider, active scan, alerts
2. **Reconner** — recon pipeline (subfinder → httpx → nmap → whatweb → gobuster → nuclei)

The system should feel like it was built by a senior team — not a trivial wrapper.

---

## REQUIRED STACK

| Layer | Technology |
|-------|------------|
| **Desktop UI** | Qt6 Widgets (C++20) |
| **HTTP Client** | QNetworkAccessManager (ZAP REST API) |
| **Processes** | QProcess (ZAP daemon, Reconner, shell scripts) |
| **Recon Engine** | Python 3.10+ / Reconner 2.0.0 (vendored in `reconner/`) |
| **Build** | CMake 3.16+ |
| **C++ Tests** | GoogleTest + Qt Test |
| **Python Tests** | pytest (reconner/tests/) |
| **C++ Lint** | clang-tidy + clang-format |
| **Python Lint** | ruff + black + mypy |
| **Logging** | QTextEdit UI + structured journal (future: spdlog) |
| **Config** | AppConfig + environment variables |
| **CI** | GitHub Actions |
| **Containerization** | Optional Docker for CI (reconner) |
| **ZAP Updates** | GitHub API + scripts/update-zap.sh |

---

## FOLDER STRUCTURE

```
ZAP-DESK/
├── .cursor/rules/              # Cursor Agent rules
├── docs/                       # Architecture documentation
├── reconner/                   # Python engine (vendored)
│   ├── reconner/               # Python package
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
│   │   ├── ZapClient.*         # ZAP REST client
│   │   └── ZapDaemon.*         # ZAP lifecycle
│   ├── domain/                 # Entities, value objects, ports
│   ├── application/            # Use cases
│   ├── infrastructure/         # Qt gateways, JSON readers
│   ├── presentation/           # ApplicationFacade
│   ├── services/
│   │   ├── ReconRunner.*       # Orchestrates reconner via QProcess
│   │   ├── ReconBridge.*       # Legacy (superseded by facade)
│   │   └── ZapUpdater.*        # ZAP auto-update
│   ├── ui/
│   │   └── MainWindow.*        # Thin controller
│   ├── components/             # LogConsole, FindingsTable, SettingsDialog, etc.
│   └── main.cpp
├── tests/                      # GoogleTest unit + optional integration
├── CMakeLists.txt
├── Makefile
└── README.md
```

---

## DOMAIN RULES

### Entities (planned evolution)

- Every scan entity: `id`, `targetId`, `createdAt`, `updatedAt`, `status`, `version`
- Entities **never** import Qt Network or QProcess directly
- Invariant validation in the constructor — fails with `DomainError`

### Value Objects

- `TargetUrl` — normalized URL with validation
- `ScanId` — ULID/UUID
- `RiskLevel` — typed enum for ZAP alerts

### Domain Events

- `ScanStarted`, `ScanCompleted`, `ZapFed`, `AlertReceived`
- Published after successful operations via internal event bus

### Repositories

- Interface in `domain/` — returns entities, not raw JSON
- Implementation in `infrastructure/` — maps summary.json / ZAP API

---

## ZAP + RECONNER INTEGRATION

### FULL PIPELINE flow

```
[BOOT ZAP] → [RUN RECON --proxy 127.0.0.1:8080] → [summary.json]
    → [FEED ZAP URLs] → [ACTIVE SCAN] → [ALERTS]
```

### Integration contract

The `summary.json` file is the handshake between Reconner and ZAP-DESK:

```json
{
  "live_hosts": { "list": ["https://..."] },
  "subdomains": { "list": ["..."] },
  "gobuster_results": [{ "url": "https://..." }]
}
```

### Automatic ZAP updates

1. `ZapUpdater` queries `api.github.com/repos/zaproxy/zap/releases/latest`
2. Compares local version (ZAP API) vs release
3. Runs `scripts/update-zap.sh` after user confirmation

---

## SECURITY

- Recon **only** with written authorization checkbox in the UI
- Local ZAP with `api.disablekey=true` in **dev mode only** — production uses API key (see [Phase 6 Security](PHASE6-SECURITY.md))
- Never log credentials or tokens
- Configurable paths via env — no hardcoded machine paths

### Environment variables

| Variable | Description |
|----------|-------------|
| `ZAP_DESK_HOME` | Project root |
| `ZAP_HOME` | OWASP ZAP installation |
| `ZAP_LAUNCH_SCRIPT` | ZAP boot script |
| `ZAP_API_PORT` | API port (default 8080) |
| `RECONNER_DIR` | Reconner directory |
| `ZAP_DESK_RESULTS` | Results directory |

---

## OBSERVABILITY

### UI logging

- Timestamps on every interface log line
- `>>` prefixes in CRT terminal style
- Severity colors via stylesheet

### Health checks (future)

- ZAP API `/JSON/core/view/version/`
- Reconner `python3 -m reconner --version`
- External tools via `check_tool_exists`

---

## TESTING — TDD STRATEGY

```
tests/           # C++ GoogleTest
reconner/tests/  # Python pytest
```

### Minimum coverage targets

- statements: 80%
- branches: 75%
- functions: 80%

### For each C++ service:

- Unit test with mocks
- Happy-path test
- Test for each possible error

---

## CODE PATTERNS

### Result type (C++ — evolution)

```cpp
template<typename T, typename E = AppError>
class Result {
    bool ok_;
    T value_;
    E error_;
};
```

Use cases return `Result<T>` — controllers interpret for the UI.

### Repository pattern

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
make update-zap   # update OWASP ZAP
```

---

## WHAT NOT TO DO

- ❌ Business logic in UI slots beyond orchestration
- ❌ Hardcoded paths (use AppConfig)
- ❌ `console.log` equivalent without a timestamp
- ❌ Run recon without authorization confirmation
- ❌ ZAP queries outside ZapClient
- ❌ Delete scan results without confirmation

---

## ADDING NEW MODULES

1. Create interface in `domain/`
2. Implement use case in `application/`
3. Adapter in `infrastructure/` or `services/`
4. Wire into UI via Qt signals
5. Write tests before implementation (TDD)

---

## Related docs

- [ZAP + Reconner integration](INTEGRATION.md)
- [Technical stack](STACK.md)
- [Development roadmap](ROADMAP.md)
