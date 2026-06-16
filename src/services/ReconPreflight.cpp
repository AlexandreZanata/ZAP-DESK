#include "ReconPreflight.hpp"

#include "config/AppConfig.hpp"

#include <QProcess>
#include <QRegularExpression>

ReconPreflight::ReconPreflight(QObject* parent) : QObject(parent) {}

void ReconPreflight::check(std::function<void(Result)> callback) {
    const auto& cfg = AppConfig::instance();

    const QString script = R"(
import sys
sys.path.insert(0, sys.argv[1])
missing = []
try:
    import click  # noqa: F401
    import jinja2  # noqa: F401
    import rich  # noqa: F401
except ImportError as exc:
    missing.append(f"python:{exc.name}")

from reconner.utils import check_tool_exists
for tool in ["subfinder", "httpx", "nmap", "whatweb", "gobuster", "nuclei"]:
    ok, _ = check_tool_exists(tool)
    if not ok:
        missing.append(tool)

if missing:
    print("MISSING:" + ",".join(missing))
else:
    print("OK")
)";

    auto* process = new QProcess(this);
    process->setProgram("python3");
    process->setArguments({"-c", script, cfg.reconnerDir()});

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            [process, callback, this](int exitCode, QProcess::ExitStatus status) {
                Result result;
                if (status != QProcess::NormalExit || exitCode != 0) {
                    result.error = QString::fromUtf8(process->readAllStandardError()).trimmed();
                    if (result.error.isEmpty()) {
                        result.error = "Preflight check failed to run (is python3 installed?)";
                    }
                    callback(result);
                    process->deleteLater();
                    return;
                }

                callback(parseOutput(QString::fromUtf8(process->readAllStandardOutput()).trimmed()));
                process->deleteLater();
            });

    process->start();
}

ReconPreflight::Result ReconPreflight::parseOutput(const QString& output) const {
    Result result;
    result.pythonOk = true;
    result.reconnerOk = true;

    if (output == "OK") {
        result.ok = true;
        return result;
    }

    static const QRegularExpression re(R"(^MISSING:(.+)$)");
    const auto match = re.match(output.trimmed());
    if (!match.hasMatch()) {
        result.error = output.isEmpty() ? "Unknown preflight response" : output;
        return result;
    }

    for (const QString& item : match.captured(1).split(',', Qt::SkipEmptyParts)) {
        if (item.startsWith("python:")) {
            result.pythonOk = false;
            result.missingTools.append(item);
        } else {
            result.missingTools.append(item);
        }
    }

    result.ok = result.missingTools.isEmpty();
    return result;
}
