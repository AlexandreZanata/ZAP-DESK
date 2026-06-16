#pragma once

#include <QObject>
#include <QStringList>
#include <functional>

class ReconPreflight : public QObject {
    Q_OBJECT

public:
    struct Result {
        bool ok{false};
        bool pythonOk{false};
        bool reconnerOk{false};
        QStringList missingTools;
        QString error;
    };

    explicit ReconPreflight(QObject* parent = nullptr);

    void check(std::function<void(Result)> callback);

private:
    Result parseOutput(const QString& output) const;
};
