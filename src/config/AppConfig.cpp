#include "AppConfig.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
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
    return QDir(m_projectRoot).filePath("reconner");
}

QString AppConfig::reconnerModule() const {
    return QDir(reconnerDir()).filePath("reconner");
}

QString AppConfig::resultsDir() const {
    const QString base = qEnvironmentVariableIsSet("ZAP_DESK_RESULTS")
                             ? qEnvironmentVariable("ZAP_DESK_RESULTS")
                             : dataDir() + "/results";
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
    return ok && port > 0 ? port : 8080;
}
