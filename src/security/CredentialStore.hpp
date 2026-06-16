#pragma once

#include <QString>

namespace security {

class CredentialStore {
public:
    static QString zapApiKey();
    static void setZapApiKey(const QString& key);
    static void clearZapApiKey();

    static bool keyringAvailable();
    static bool useKeyring();
    static void setUseKeyring(bool enabled);
};

}  // namespace security
