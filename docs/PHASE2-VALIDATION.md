# Phase 2 Validation Checklist

Use this checklist to validate the **Integration Layer** before moving to Phase 3.

Automated checks:

```bash
chmod +x scripts/validate-phase2.sh
./scripts/validate-phase2.sh
```

---

## 1. Environment setup

| # | Check | Command / action | Expected |
|---|-------|------------------|----------|
| 1.1 | Dependencies | `make install-deps` | cmake, qt6, python3 installed |
| 1.2 | Reconner | `make install-reconner` | `python3 -m reconner --help` works |
| 1.3 | Recon tools | `cd reconner && ./install-tools.sh` | subfinder, httpx, nmap, etc. on PATH |
| 1.4 | OWASP ZAP | `make update-zap` | `$HOME/.local/share/zap-desk/zap/zap.sh` exists |
| 1.5 | Build app | `make build` | `build/zap-desk` executable |

---

## 2. ZAP integration (manual UI)

| # | Check | Steps | Expected |
|---|-------|-------|----------|
| 2.1 | Boot ZAP | Open app → **[ ZAP ]** → **BOOT ZAP** | Log: `ZAP started in daemon mode` |
| 2.2 | API online | Wait ~15s | Status: `ONLINE — ZAP x.x.x` |
| 2.3 | Version check | **CHECK ZAP UPDATE** | Shows local vs GitHub version |
| 2.4 | AJAX spider | Enter authorized URL → **AJAX SPIDER** | Status shows running |
| 2.5 | Active scan | **ACTIVE SCAN** | Progress % in status bar |
| 2.6 | Alerts | **REFRESH ALERTS** | Table populates (may be empty) |
| 2.7 | Kill ZAP | **KILL ZAP** | Status: `OFFLINE` |

---

## 3. Recon integration (manual UI)

| # | Check | Steps | Expected |
|---|-------|-------|----------|
| 3.1 | Authorization | Run without checkbox | Warning dialog blocks scan |
| 3.2 | Preflight | Check auth → **RUN RECON** | Log: `Preflight OK` or clear missing-tool error |
| 3.3 | Progress | During scan | `RECON [1/6] subfinder — ...` updates through phases |
| 3.4 | Summary | After scan | `RECON SUMMARY: N subdomains \| ...` |
| 3.5 | Feed ZAP | **FEED ZAP** (ZAP online) | Log: `N URL(s) sent to ZAP` |
| 3.6 | Proxy mode | Enable proxy checkbox, ZAP online | Traffic routes through 127.0.0.1:8080 |

---

## 4. Full pipeline (authorized target only)

| # | Check | Steps | Expected |
|---|-------|-------|----------|
| 4.1 | Pipeline | **[ RECON ]** → auth → **FULL PIPELINE** | ZAP boots → recon runs → feed → active scan |
| 4.2 | Artifacts | Check results dir | `summary.json`, `report.md` under `~/.local/share/zap-desk/results/` |
| 4.3 | ZAP tree | After feed | ZAP has seeded URLs (check via REFRESH ALERTS after scan) |

---

## 5. Error handling

| # | Check | Steps | Expected |
|---|-------|-------|----------|
| 5.1 | ZAP missing | Remove/rename ZAP_HOME → BOOT ZAP | Clear error: run `make update-zap` |
| 5.2 | Tool missing | Rename one tool temporarily → RUN RECON | Preflight dialog lists missing tool |
| 5.3 | ZAP offline feed | FEED ZAP without ZAP | Error: `ZAP offline` |

---

## 6. Paths (XDG defaults)

| Variable | Default | Verify |
|----------|---------|--------|
| `ZAP_HOME` | `~/.local/share/zap-desk/zap` | `ls $ZAP_HOME/zap.sh` |
| `ZAP_CONFIG_DIR` | `~/.local/share/zap-desk/config/home` | exists after first ZAP boot |
| Results | `~/.local/share/zap-desk/results` | recon output appears here |

---

## Sign-off

Phase 2 is complete when:

- [ ] `./scripts/validate-phase2.sh` exits 0 (or only WARN for optional ZAP/tools on CI)
- [ ] Manual checks 2.1–2.7 pass
- [ ] Manual checks 3.1–3.5 pass
- [ ] Full pipeline (4.1) tested on **authorized** target

Then update [ROADMAP.md](ROADMAP.md) — Phase 2 ✅, Phase 3 🔄.
