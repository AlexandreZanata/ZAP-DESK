#include "AppConfig.hpp"

#include "components/AppTheme.hpp"
#include "security/CredentialStore.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>

AppConfig& AppConfig::instance() {
    static AppConfig cfg;
    return cfg;
}

AppConfig::AppConfig() {
    reload();
}

void AppConfig::reload() {
    m_projectRoot = detectProjectRoot();
}

QString AppConfig::detectProjectRoot() const {
    if (qEnvironmentVariableIsSet("ZAP_DESK_HOME")) {
        return qEnvironmentVariable("ZAP_DESK_HOME");
    }

    const QString appDir = QCoreApplication::applicationDirPath();
    QDir prefixDir(appDir);
    if (prefixDir.cdUp()) {
        const QString shareRoot = prefixDir.filePath("share/zap-desk");
        if (QFileInfo::exists(QDir(shareRoot).filePath("reconner/setup.py"))) {
            return shareRoot;
        }
    }

    QDir dir(appDir);
    for (int i = 0; i < 5; ++i) {
        if (QFileInfo::exists(dir.filePath("reconner/setup.py"))) {
            return dir.absolutePath();
        }
        if (!dir.cdUp()) break;
    }

    return QDir::currentPath();
}

QString AppConfig::projectRoot() const {
    return m_projectRoot;
}

QString AppConfig::dataDir() const {
    if (qEnvironmentVariableIsSet("ZAP_DESK_DATA")) {
        return qEnvironmentVariable("ZAP_DESK_DATA");
    }
    const QString base =
        QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/zap-desk";
    QDir().mkpath(base);
    return base;
}

QString AppConfig::reconnerDir() const {
    if (qEnvironmentVariableIsSet("RECONNER_DIR")) {
        return qEnvironmentVariable("RECONNER_DIR");
    }
    QSettings settings;
    if (settings.contains("paths/reconner")) {
        return settings.value("paths/reconner").toString();
    }
    return QDir(m_projectRoot).filePath("reconner");
}

QString AppConfig::reconnerModule() const {
    return QDir(reconnerDir()).filePath("reconner");
}

QString AppConfig::resultsDir() const {
    if (qEnvironmentVariableIsSet("ZAP_DESK_RESULTS")) {
        const QString base = qEnvironmentVariable("ZAP_DESK_RESULTS");
        QDir().mkpath(base);
        return base;
    }
    QSettings settings;
    if (settings.contains("paths/results")) {
        const QString base = settings.value("paths/results").toString();
        QDir().mkpath(base);
        return base;
    }
    const QString base = dataDir() + "/results";
    QDir().mkpath(base);
    return base;
}

QString AppConfig::zapLaunchScript() const {
    if (qEnvironmentVariableIsSet("ZAP_LAUNCH_SCRIPT")) {
        return qEnvironmentVariable("ZAP_LAUNCH_SCRIPT");
    }
    const QString local = QDir(m_projectRoot).filePath("scripts/zap-launch.sh");
    if (QFileInfo::exists(local)) return local;
    return QString();
}

QString AppConfig::zapHome() const {
    if (qEnvironmentVariableIsSet("ZAP_HOME")) {
        return qEnvironmentVariable("ZAP_HOME");
    }
    QSettings settings;
    if (settings.contains("zap/home")) {
        return settings.value("zap/home").toString();
    }
    return QDir(dataDir()).filePath("zap");
}

QString AppConfig::zapConfigDir() const {
    if (qEnvironmentVariableIsSet("ZAP_CONFIG_DIR")) {
        return qEnvironmentVariable("ZAP_CONFIG_DIR");
    }
    const QString dir = QDir(dataDir()).filePath("config/home");
    QDir().mkpath(dir);
    return dir;
}

QString AppConfig::zapPidFile() const {
    if (qEnvironmentVariableIsSet("ZAP_DESK_PID_FILE")) {
        return qEnvironmentVariable("ZAP_DESK_PID_FILE");
    }
    const QString runtime = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);
    if (!runtime.isEmpty()) {
        return QDir(runtime).filePath("zap-desk.pid");
    }
    return QDir(dataDir()).filePath("zap-desk.pid");
}

QString AppConfig::zapApiUrl() const {
    return QString("http://127.0.0.1:%1").arg(zapApiPort());
}

int AppConfig::zapApiPort() const {
    bool ok = false;
    const int port = qEnvironmentVariable("ZAP_API_PORT").toInt(&ok);
    if (ok && port > 0) return port;

    QSettings settings;
    if (settings.contains("zap/port")) {
        return settings.value("zap/port").toInt();
    }
    return 8080;
}

bool AppConfig::crtOverlayEnabled() const {
    if (!components::AppTheme::supportsCrtOverlay(uiThemeMode())) return false;
    QSettings settings;
    return settings.value("ui/crtOverlay", true).toBool();
}

QString AppConfig::uiTheme() const {
    QSettings settings;
    return settings.value("ui/theme", "hacker").toString();
}

components::AppThemeMode AppConfig::uiThemeMode() const {
    return components::AppTheme::fromString(uiTheme());
}

void AppConfig::setUiTheme(const QString& theme) {
    QSettings settings;
    settings.setValue("ui/theme", theme);
    settings.sync();
}

QString AppConfig::appIconPath() const {
    const QString bundled = QDir(m_projectRoot).filePath("packaging/icons/zap-desk.svg");
    if (QFileInfo::exists(bundled)) return bundled;

    const QString appDir = QCoreApplication::applicationDirPath();
    QDir prefixDir(appDir);
    if (prefixDir.cdUp()) {
        const QString installed =
            prefixDir.filePath("share/icons/hicolor/scalable/apps/com.zapdesk.ZAPDesk.svg");
        if (QFileInfo::exists(installed)) return installed;
        const QString legacy = prefixDir.filePath("share/icons/hicolor/scalable/apps/zap-desk.svg");
        if (QFileInfo::exists(legacy)) return legacy;
    }
    return {};
}

QString AppConfig::zapApiKey() const {
    return security::CredentialStore::zapApiKey();
}

bool AppConfig::zapDevMode() const {
    QSettings settings;
    return settings.value("security/devMode", false).toBool();
}

int AppConfig::reconRateLimitSeconds() const {
    QSettings settings;
    return settings.value("security/reconRateLimit", 30).toInt();
}

QString AppConfig::auditLogPath() const {
    return QDir(dataDir()).filePath("audit.log");
}
