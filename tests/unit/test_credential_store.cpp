#include <gtest/gtest.h>

#include "security/CredentialStore.hpp"
#include "security/KeyringBackend.hpp"

TEST(CredentialStoreTest, KeyringDisabledWhenToolMissing) {
    if (!security::KeyringBackend::isAvailable()) {
        security::CredentialStore::setUseKeyring(true);
        EXPECT_FALSE(security::CredentialStore::useKeyring());
    }
}

TEST(KeyringBackendTest, EnabledFollowsSettingsWhenAvailable) {
    if (security::KeyringBackend::isAvailable()) {
        security::KeyringBackend::setEnabled(true);
        EXPECT_TRUE(security::KeyringBackend::isEnabled());
        security::KeyringBackend::setEnabled(false);
        EXPECT_FALSE(security::KeyringBackend::isEnabled());
    }
}
