# Phase 6 — Security Hardening

Production-safe defaults for ZAP API access, audit trail, and recon rate limiting.

## ZAP API key

`ZapClient` appends `apikey=<key>` to every REST request when a key is configured.

**Priority order:**

1. `ZAP_API_KEY` environment variable
2. `security/apiKey` in QSettings (`~/.config/ZAP-DESK/zap-desk.conf`)
3. Empty — requires dev mode or manual ZAP key setup

Configure in **File → Settings → Security**.

## ZAP boot profiles

`ZapDaemon` selects launch flags based on settings:

| Condition | Behavior |
|-----------|----------|
| Dev mode ON + no API key | `-config api.disablekey=true` (localhost only) |
| API key set | `-config api.key=<key>` |
| Dev mode OFF + no key | Starts without disablekey; operator must set a key |

**Default:** dev mode **off** (production-safe). Enable dev mode for local testing without a key.

## Audit log

Scan actions are appended to `~/.local/share/zap-desk/audit.log`:

```
2026-06-16T14:30:00Z | BOOT_ZAP
2026-06-16T14:31:12Z | START_RECON | https://authorized-target.com
2026-06-16T14:45:00Z | FULL_PIPELINE | https://authorized-target.com
```

Logged actions: `BOOT_ZAP`, `SHUTDOWN_ZAP`, `AJAX_SCAN`, `ACTIVE_SCAN`, `STOP_SCANS`, `START_RECON`, `STOP_RECON`, `FEED_ZAP`, `FULL_PIPELINE`, `CHECK_ZAP_UPDATE`, `RUN_ZAP_UPDATE`.

## Recon rate limiting

Default minimum interval between recon launches: **30 seconds** (configurable in Settings, `0` disables).

Enforced in `ApplicationFacade::startRecon()` before preflight.

## Components (`src/security/`)

| Module | Role |
|--------|------|
| `CredentialStore` | API key load/save (env + QSettings) |
| `AuditLogger` | Append-only action log |
| `ReconRateLimiter` | Cooldown between recon runs |

## Exit criteria

- [x] ZAP API key in `ZapClient` and Settings UI
- [x] `api.disablekey=true` only in dev mode
- [x] Credential storage via QSettings + env override
- [x] Audit log for scan actions
- [x] Rate limiting on recon launches

**App version:** v0.6.0
