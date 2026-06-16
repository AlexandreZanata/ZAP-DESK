# Phase 3 — Domain & Architecture

Clean architecture layer introduced in v0.3.0.

## Layers

```
presentation/     ApplicationFacade — thin controller for UI
application/      Use cases (validation, business rules)
domain/           Entities, value objects, port interfaces
infrastructure/   Qt adapters (gateways, JSON readers)
shared/           Result<T>, AppError, DomainError
```

## Use cases

| Use case | Responsibility |
|----------|----------------|
| `StartReconUseCase` | Authorization + target validation → `Scan` + `ReconOptions` |
| `FeedZapUseCase` | Validates summary path + ZAP online |
| `RunFullPipelineUseCase` | Reuses recon validation for pipeline |
| `StartZapScanUseCase` | Validates scan target URL |

## Ports (domain interfaces)

- `IReconGateway` — start/stop recon process
- `IPreflightGateway` — tool dependency checks
- `IReconSummaryReader` — parse `summary.json`
- `IZapGateway` — ZAP daemon + REST API

## Infrastructure adapters

- `QtReconGateway` → `ReconRunner`
- `QtPreflightGateway` → `ReconPreflight`
- `QtZapGateway` → `ZapClient` + `ZapDaemon` + `ZapUpdater`
- `JsonReconSummaryReader` → JSON file parsing

## UI rule

`MainWindow` only:

- Renders widgets and styles
- Calls `ApplicationFacade` methods
- Maps facade signals to UI updates

No business validation in `MainWindow` slots.

## Tests

```bash
make build
make test-domain
```

Domain tests (`tests/unit/test_domain.cpp`) run without GUI:

- `TargetUrl` validation
- `StartReconUseCase` authorization rules
- `FeedZapUseCase` ZAP offline guard
- `JsonReconSummaryReader` fixture parsing

## Validate Phase 3

```bash
make build
make test-domain
make test          # includes reconner pytest
```
