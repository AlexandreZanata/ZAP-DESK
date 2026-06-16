#include "CredentialStore.hpp"

#include "KeyringBackend.hpp"

#include <QProcessEnvironment>
#include <QSettings>

namespace security {

namespace {
constexpr auto kService = "ZAP-DESK";
constexpr auto kAccount = "zap-api-key";

QString loadFromKeyring() {
    if (!KeyringBackend::isEnabled()) return {};
    return KeyringBackend::loadSecret(kService, kAccount);
}

QString loadFromSettings() {
    QSettings settings;
    return settings.value("security/apiKey").toString();
}

void clearSettingsKey() {
    QSettings settings;
    settings.remove("security/apiKey");
    settings.sync();
}

void migrateSettingsToKeyring() {
    const QString legacy = loadFromSettings();
    if (legacy.isEmpty() || !KeyringBackend::isEnabled()) return;

    if (KeyringBackend::storeSecret(kService, kAccount, legacy)) {
        clearSettingsKey();
    }
}

}  // namespace

QString CredentialStore::zapApiKey() {
    if (qEnvironmentVariableIsSet("ZAP_API_KEY")) {
        return qEnvironmentVariable("ZAP_API_KEY");
    }

    migrateSettingsToKeyring();

    const QString fromKeyring = loadFromKeyring();
    if (!fromKeyring.isEmpty()) return fromKeyring;

    return loadFromSettings();
}

void CredentialStore::setZapApiKey(const QString& key) {
    if (KeyringBackend::isEnabled()) {
        KeyringBackend::storeSecret(kService, kAccount, key);
        clearSettingsKey();
        return;
    }

    QSettings settings;
    if (key.isEmpty()) {
        settings.remove("security/apiKey");
    } else {
        settings.setValue("security/apiKey", key);
    }
    settings.sync();
}

void CredentialStore::clearZapApiKey() {
    KeyringBackend::clearSecret(kService, kAccount);
    clearSettingsKey();
}

bool CredentialStore::keyringAvailable() {
    return KeyringBackend::isAvailable();
}

bool CredentialStore::useKeyring() {
    return KeyringBackend::isEnabled();
}

void CredentialStore::setUseKeyring(bool enabled) {
    if (enabled && !KeyringBackend::isAvailable()) return;

    const QString current = zapApiKey();
    KeyringBackend::setEnabled(enabled);

    if (enabled) {
        if (!current.isEmpty()) {
            KeyringBackend::storeSecret(kService, kAccount, current);
            clearSettingsKey();
        }
    } else if (!current.isEmpty()) {
        QSettings settings;
        settings.setValue("security/apiKey", current);
        settings.sync();
        KeyringBackend::clearSecret(kService, kAccount);
    }
}

}  // namespace security
