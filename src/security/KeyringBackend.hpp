#pragma once

#include <QString>

namespace security {

class KeyringBackend {
public:
    static bool isAvailable();
    static bool isEnabled();
    static void setEnabled(bool enabled);

    static QString loadSecret(const QString& service, const QString& account);
    static bool storeSecret(const QString& service, const QString& account, const QString& secret);
    static bool clearSecret(const QString& service, const QString& account);
};

}  // namespace security
