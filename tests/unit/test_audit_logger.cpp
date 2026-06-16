#include <gtest/gtest.h>

#include <QDir>
#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>

#include "security/AuditLogger.hpp"

TEST(AuditLoggerTest, AppendsTimestampedEntry) {
    QTemporaryDir temp;
    ASSERT_TRUE(temp.isValid());

    const QString logPath = temp.filePath("audit.log");
    security::AuditLogger logger(logPath);
    logger.log("BOOT_ZAP");
    logger.log("START_RECON", "https://example.com");

    QFile file(logPath);
    ASSERT_TRUE(file.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString contents = QTextStream(&file).readAll();

    EXPECT_TRUE(contents.contains("BOOT_ZAP"));
    EXPECT_TRUE(contents.contains("START_RECON"));
    EXPECT_TRUE(contents.contains("https://example.com"));
}
