#include <gtest/gtest.h>

#include "security/ReconRateLimiter.hpp"

TEST(ReconRateLimiterTest, AllowsFirstLaunch) {
    security::ReconRateLimiter limiter(30);
    EXPECT_TRUE(limiter.canStart());
}

TEST(ReconRateLimiterTest, BlocksImmediateSecondLaunch) {
    security::ReconRateLimiter limiter(30);
    limiter.recordStart();

    QString reason;
    EXPECT_FALSE(limiter.canStart(&reason));
    EXPECT_FALSE(reason.isEmpty());
}

TEST(ReconRateLimiterTest, DisabledWhenIntervalZero) {
    security::ReconRateLimiter limiter(0);
    limiter.recordStart();
    EXPECT_TRUE(limiter.canStart());
}
