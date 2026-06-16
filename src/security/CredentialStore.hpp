#pragma once

#include <QString>

namespace security {

class CredentialStore {
public:
    static QString zapApiKey();
    static void setZapApiKey(const QString& key);
    static void clearZapApiKey();
};

}  // namespace security
