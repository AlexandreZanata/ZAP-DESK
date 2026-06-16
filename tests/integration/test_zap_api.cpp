#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QEventLoop>
#include <QTimer>

#include "core/ZapClient.hpp"

namespace {

bool zapReachable() {
    QEventLoop loop;
    bool ok = false;
    ZapClient client;
    client.checkConnection([&](bool connected, const QString&) {
        ok = connected;
        loop.quit();
    });
    QTimer::singleShot(3000, &loop, &QEventLoop::quit);
    loop.exec();
    return ok;
}

}  // namespace

TEST(ZapApiIntegration, VersionEndpoint) {
    if (!zapReachable()) {
        GTEST_SKIP() << "ZAP not reachable at http://127.0.0.1:8080";
    }

    QEventLoop loop;
    QString version;
    ZapClient client;
    client.checkConnection([&](bool ok, const QString& v) {
        ASSERT_TRUE(ok);
        version = v;
        loop.quit();
    });
    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    loop.exec();

    EXPECT_FALSE(version.isEmpty());
}

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
