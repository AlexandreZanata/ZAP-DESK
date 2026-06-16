#include "KeyringBackend.hpp"

#include <QProcess>
#include <QSettings>
#include <QStandardPaths>

namespace security {

namespace {

QString secretToolPath() {
    return QStandardPaths::findExecutable("secret-tool");
}

QString runLookup(const QString& service, const QString& account) {
    const QString tool = secretToolPath();
    if (tool.isEmpty()) return {};

    QProcess process;
    process.start(tool, {"lookup", "service", service, "username", account});
    if (!process.waitForFinished(5000)) {
        process.kill();
        return {};
    }
    if (process.exitCode() != 0) return {};
    return QString::fromUtf8(process.readAllStandardOutput()).trimmed();
}

bool runStore(const QString& service, const QString& account, const QString& secret) {
    const QString tool = secretToolPath();
    if (tool.isEmpty()) return false;

    QProcess process;
    process.start(tool,
                  {"store", "--label=ZAP-DESK API Key", "service", service, "username", account});
    if (!process.waitForStarted(3000)) return false;

    process.write(secret.toUtf8());
    process.closeWriteChannel();

    if (!process.waitForFinished(8000)) {
        process.kill();
        return false;
    }
    return process.exitCode() == 0;
}

bool runClear(const QString& service, const QString& account) {
    const QString tool = secretToolPath();
    if (tool.isEmpty()) return false;

    QProcess process;
    process.start(tool, {"clear", "service", service, "username", account});
    if (!process.waitForFinished(5000)) {
        process.kill();
        return false;
    }
    return process.exitCode() == 0;
}

}  // namespace

bool KeyringBackend::isAvailable() {
    return !secretToolPath().isEmpty();
}

bool KeyringBackend::isEnabled() {
    if (!isAvailable()) return false;
    QSettings settings;
    return settings.value("security/useKeyring", true).toBool();
}

void KeyringBackend::setEnabled(bool enabled) {
    QSettings settings;
    settings.setValue("security/useKeyring", enabled);
    settings.sync();
}

QString KeyringBackend::loadSecret(const QString& service, const QString& account) {
    return runLookup(service, account);
}

bool KeyringBackend::storeSecret(const QString& service, const QString& account,
                                 const QString& secret) {
    if (secret.isEmpty()) return clearSecret(service, account);
    runClear(service, account);
    return runStore(service, account, secret);
}

bool KeyringBackend::clearSecret(const QString& service, const QString& account) {
    runClear(service, account);
    return true;
}

}  // namespace security
