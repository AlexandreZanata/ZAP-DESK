#include "CredentialStore.hpp"

#include <QProcessEnvironment>
#include <QSettings>

namespace security {

QString CredentialStore::zapApiKey() {
    if (qEnvironmentVariableIsSet("ZAP_API_KEY")) {
        return qEnvironmentVariable("ZAP_API_KEY");
    }
    QSettings settings;
    return settings.value("security/apiKey").toString();
}

void CredentialStore::setZapApiKey(const QString& key) {
    QSettings settings;
    if (key.isEmpty()) {
        settings.remove("security/apiKey");
    } else {
        settings.setValue("security/apiKey", key);
    }
    settings.sync();
}

void CredentialStore::clearZapApiKey() {
    setZapApiKey({});
}

}  // namespace security
