# Phase 7 — Packaging & Release

One-command install and distributable packages for Ubuntu/Pop!_OS.

## One-command install (from source)

```bash
git clone git@github.com:AlexandreZanata/ZAP-DESK.git
cd ZAP-DESK
make install-all
make dev
```

`install-all` runs: system deps → reconner → OWASP ZAP → `make build`.

## .deb package

```bash
make package-deb
sudo dpkg -i build/zap-desk_*.deb
```

Installs:

| Path | Content |
|------|---------|
| `/usr/bin/zap-desk` | Application binary |
| `/usr/share/zap-desk/reconner/` | Python recon engine |
| `/usr/share/zap-desk/scripts/` | `zap-launch.sh`, `update-zap.sh` |
| `/usr/share/applications/zap-desk.desktop` | Desktop entry |
| `/usr/share/icons/hicolor/scalable/apps/zap-desk.svg` | Icon |

Post-install (`postinst`) runs `pip install` for reconner and updates desktop/icon caches.

## AppDir / AppImage

```bash
make package-appimage
# Output: build/ZAP-DESK.AppDir (or .AppImage if appimagetool is installed)
```

Portable layout; target system still needs Qt6 runtime libraries.

## GitHub releases

Push a version tag to trigger the release workflow:

```bash
git tag v0.7.0
git push origin v0.7.0
```

Artifacts: `.deb` + `ZAP-DESK-AppDir.tar.gz`.

## Installed layout detection

`AppConfig` resolves paths automatically:

1. `ZAP_DESK_HOME` environment variable
2. FHS install: `<prefix>/share/zap-desk` (when binary is in `<prefix>/bin`)
3. Development tree: walk up from binary to find `reconner/setup.py`

## Exit criteria

- [x] `.deb` package via CPack
- [x] Desktop entry (`zap-desk.desktop`)
- [x] Bundled install script (`install-all.sh`)
- [x] Release workflow (tag → artifacts)
- [x] Subdomain discovery guide (`docs/SUBDOMAIN-DISCOVERY.md`)

**App version:** v0.7.0
