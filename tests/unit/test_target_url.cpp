#include <gtest/gtest.h>

#include "domain/value_objects/TargetUrl.hpp"

TEST(TargetUrlTest, RejectsEmpty) {
    const auto result = domain::TargetUrl::create("");
    EXPECT_FALSE(result.isOk());
}

TEST(TargetUrlTest, RejectsBareScheme) {
    EXPECT_FALSE(domain::TargetUrl::create("https://").isOk());
    EXPECT_FALSE(domain::TargetUrl::create("http://").isOk());
}

TEST(TargetUrlTest, AcceptsValidHttpsUrl) {
    const auto result = domain::TargetUrl::create("https://example.com");
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.value().value(), "https://example.com");
}

TEST(TargetUrlTest, AcceptsValidHttpUrl) {
    const auto result = domain::TargetUrl::create("http://localhost:8080/path");
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.value().value(), "http://localhost:8080/path");
}
