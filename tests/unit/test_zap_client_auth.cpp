#include <gtest/gtest.h>

#include "core/ZapClient.hpp"

TEST(ZapClientAuthTest, AppendsApiKeyQueryParam) {
    ZapClient client;
    client.setApiKey("secret-key");

    const QString path = client.authenticatedPath("/JSON/core/view/version/");
    EXPECT_TRUE(path.contains("apikey=secret-key"));
}

TEST(ZapClientAuthTest, AppendsApiKeyWithAmpersandWhenQueryExists) {
    ZapClient client;
    client.setApiKey("abc123");

    const QString path = client.authenticatedPath("/JSON/alert/view/alerts/?start=0&count=10");
    EXPECT_TRUE(path.contains("&apikey="));
    EXPECT_FALSE(path.contains("?start=0&apikey="));
}

TEST(ZapClientAuthTest, LeavesPathUnchangedWhenKeyEmpty) {
    ZapClient client;
    const QString path = "/JSON/core/view/version/";
    EXPECT_EQ(client.authenticatedPath(path), path);
}
