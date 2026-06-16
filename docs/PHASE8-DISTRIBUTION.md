# Phase 8 — Distribution & Secrets

Post-release improvements: secure credential storage and Flatpak distribution.

## System keyring (libsecret)

When `libsecret-tools` is installed (`secret-tool` on PATH), ZAP-DESK can store the ZAP API key in the **GNOME Keyring** / **KWallet** (via libsecret) instead of plaintext `QSettings`.

### Enable

1. Install: `sudo apt install libsecret-tools`
2. Open **File → Settings → Security**
3. Check **Store API key in system keyring**
4. Save and enter your API key

### Priority order

1. `ZAP_API_KEY` environment variable (always wins)
2. System keyring (when enabled)
3. Legacy `QSettings` plaintext (auto-migrated to keyring on first load)

### Implementation

| Module | Role |
|--------|------|
| `KeyringBackend` | `secret-tool` lookup/store/clear |
| `CredentialStore` | Unified API with migration |

## Flatpak

Portable sandboxed install via Flathub-compatible manifest.

### Prerequisites

```bash
sudo apt install flatpak flatpak-builder
flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo
flatpak install flathub org.kde.Platform//6.7 org.kde.Sdk//6.7
```

### Build

```bash
make package-flatpak
flatpak install --user build/flatpak/ZAP-DESK.flatpak
flatpak run com.zapdesk.ZAPDesk
```

Manifest: `packaging/flatpak/com.zapdesk.ZAPDesk.yml`

Permissions: network, home filesystem (results), D-Bus secret service.

## Validate

```bash
make validate-phase8
```

## Exit criteria

- [x] Keyring integration for API key (libsecret via `secret-tool`)
- [x] Settings UI toggle for keyring storage
- [x] Flatpak manifest + build script
- [x] Auto-migrate plaintext keys to keyring

**App version:** v0.8.0
