#include <gtest/gtest.h>

#include "domain/scan/Scan.hpp"
#include "domain/value_objects/ScanId.hpp"
#include "domain/value_objects/TargetUrl.hpp"

TEST(ScanEntityTest, CreateWithValidInputs) {
    auto target = domain::TargetUrl::create("https://example.com");
    ASSERT_TRUE(target.isOk());

    auto scan = domain::Scan::create(domain::ScanId::generate(), target.value());
    ASSERT_TRUE(scan.isOk());
    EXPECT_EQ(scan.value().status(), domain::ScanStatus::Pending);
    EXPECT_GT(scan.value().createdAt(), 0);
}

TEST(ScanEntityTest, TransitionsThroughLifecycle) {
    auto target = domain::TargetUrl::create("https://example.com");
    auto scan = domain::Scan::create(domain::ScanId::generate(), target.value());
    ASSERT_TRUE(scan.isOk());

    auto entity = scan.value();
    const int version = entity.version();

    entity.markRunning();
    EXPECT_EQ(entity.status(), domain::ScanStatus::Running);
    EXPECT_EQ(entity.version(), version + 1);

    entity.markCompleted();
    EXPECT_EQ(entity.status(), domain::ScanStatus::Completed);

    entity.markFailed();
    EXPECT_EQ(entity.status(), domain::ScanStatus::Failed);

    entity.markCancelled();
    EXPECT_EQ(entity.status(), domain::ScanStatus::Cancelled);
}

TEST(ScanIdTest, GenerateIsNonEmpty) {
    const auto id = domain::ScanId::generate();
    EXPECT_FALSE(id.value().empty());
}

TEST(ScanIdTest, CreateRejectsEmpty) {
    EXPECT_FALSE(domain::ScanId::create("").isOk());
}
