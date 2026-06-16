# Phase 4 — Testing & CI

Automated test strategy for ZAP-DESK v0.3.x.

## Test suites

| Suite | Command | What it covers |
|-------|---------|----------------|
| C++ unit (GoogleTest) | `make test-unit` | Domain, use cases, JSON reader |
| C++ integration (optional) | `make test-integration` | Live ZAP API (skipped if offline) |
| Reconner (pytest) | `make test-reconner` | Python engine + ZAP_DESK_MODE hook |
| All | `make test` | Unit + reconner |
| Phase 4 gate | `make validate-phase4` | Full validation script |

## Run locally

```bash
make install-deps
make install-reconner
make build
make test
```

### Unit tests only

```bash
cd build && ctest --output-on-failure
# or
make test-unit
```

### With GCC coverage (optional)

```bash
cd build
cmake .. -DZAP_DESK_ENABLE_COVERAGE=ON
cmake --build . -j$(nproc)
./zap-desk-unit-tests
```

### ZAP integration tests (optional)

Requires ZAP running on `127.0.0.1:8080`:

```bash
make test-integration
```

Tests auto-skip with `GTEST_SKIP` when ZAP is offline.

## C++ unit test files

```
tests/unit/
  test_target_url.cpp      # TargetUrl validation
  test_usecases.cpp        # StartRecon, FeedZap, pipeline, Zap scan
  test_scan_entity.cpp     # Scan lifecycle + ScanId
  test_risk_level.cpp      # RiskLevel parsing
  test_summary_reader.cpp  # summary.json fixture parsing
```

## Reconner coverage

Configured in `reconner/pytest.ini`:

- `fail_under = 70` (path to 80% in Phase 4+)
- `pytest --cov=reconner` in CI and `make test-reconner`

New test: `reconner/tests/test_zap_desk_runner.py` — validates `@@ZAP-DESK@@PHASE` output.

## CI (GitHub Actions)

Workflow `.github/workflows/ci.yml` jobs:

1. **test-cpp** — build + `ctest` (GoogleTest)
2. **test-reconner** — pytest with coverage gate
3. **validate-phase2** — integration layer smoke checks

## Fixtures

- `tests/fixtures/summary-sample.json` — canonical `summary.json` for reader tests

## Exit criteria (Phase 4)

- [x] GoogleTest wired via CMake FetchContent
- [x] Expanded unit tests for domain + application layers
- [x] `make test` runs C++ and Python suites
- [x] CI runs unit tests on every PR
- [x] Reconner coverage gate in pytest
- [ ] 80% coverage threshold (target; currently 70%)

## Related

- [Phase 3 architecture](PHASE3-ARCHITECTURE.md)
- [Phase 2 validation](PHASE2-VALIDATION.md)
- [Roadmap](ROADMAP.md)
