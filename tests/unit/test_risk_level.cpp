#include <gtest/gtest.h>

#include "domain/value_objects/RiskLevel.hpp"

TEST(RiskLevelTest, ParsesKnownLevels) {
    EXPECT_EQ(domain::riskLevelFromString("High"), domain::RiskLevel::High);
    EXPECT_EQ(domain::riskLevelFromString("medium"), domain::RiskLevel::Medium);
    EXPECT_EQ(domain::riskLevelFromString("Low"), domain::RiskLevel::Low);
    EXPECT_EQ(domain::riskLevelFromString("Informational"), domain::RiskLevel::Informational);
    EXPECT_EQ(domain::riskLevelFromString("critical"), domain::RiskLevel::Critical);
}

TEST(RiskLevelTest, UnknownMapsToUnknown) {
    EXPECT_EQ(domain::riskLevelFromString("banana"), domain::RiskLevel::Unknown);
}

TEST(RiskLevelTest, RoundTripsCommonLevels) {
    EXPECT_EQ(domain::riskLevelToString(domain::RiskLevel::High), "High");
    EXPECT_EQ(domain::riskLevelToString(domain::RiskLevel::Medium), "Medium");
}
