#include <gtest/gtest.h>

#include "application/usecases/FeedZapUseCase.hpp"
#include "application/usecases/RunFullPipelineUseCase.hpp"
#include "application/usecases/StartReconUseCase.hpp"
#include "application/usecases/StartZapScanUseCase.hpp"
#include "domain/scan/ScanStatus.hpp"
#include "shared/errors/AppError.hpp"

TEST(StartReconUseCaseTest, DeniesWithoutAuthorization) {
    application::StartReconRequest req{"https://example.com", false, false, false, true};
    const auto result = application::StartReconUseCase::validate(req);
    EXPECT_FALSE(result.isOk());
    EXPECT_EQ(result.error().code(), shared::ErrorCode::Domain);
}

TEST(StartReconUseCaseTest, RejectsInvalidTarget) {
    application::StartReconRequest req{"https://", true, false, false, true};
    EXPECT_FALSE(application::StartReconUseCase::validate(req).isOk());
}

TEST(StartReconUseCaseTest, CreatesPendingScanWhenValid) {
    application::StartReconRequest req{"https://example.com", true, true, true, false};
    const auto result = application::StartReconUseCase::validate(req);
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.value().scan.status(), domain::ScanStatus::Pending);
    EXPECT_TRUE(result.value().options.fastMode);
    EXPECT_TRUE(result.value().options.skipNuclei);
    EXPECT_FALSE(result.value().options.useZapProxy);
}

TEST(FeedZapUseCaseTest, RejectsEmptySummaryPath) {
    application::FeedZapRequest req{"", true};
    const auto result = application::FeedZapUseCase::validate(req);
    EXPECT_FALSE(result.isOk());
    EXPECT_EQ(result.error().code(), shared::ErrorCode::NotFound);
}

TEST(FeedZapUseCaseTest, RejectsWhenZapOffline) {
    application::FeedZapRequest req{"/tmp/summary.json", false};
    const auto result = application::FeedZapUseCase::validate(req);
    EXPECT_FALSE(result.isOk());
    EXPECT_EQ(result.error().code(), shared::ErrorCode::ZapOffline);
}

TEST(FeedZapUseCaseTest, AcceptsWhenZapOnline) {
    application::FeedZapRequest req{"/tmp/summary.json", true};
    const auto result = application::FeedZapUseCase::validate(req);
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.value(), "/tmp/summary.json");
}

TEST(RunFullPipelineUseCaseTest, PropagatesAuthorizationError) {
    application::FullPipelineRequest req{"https://example.com", false, false, false, true};
    EXPECT_FALSE(application::RunFullPipelineUseCase::validate(req).isOk());
}

TEST(RunFullPipelineUseCaseTest, AcceptsValidRequest) {
    application::FullPipelineRequest req{"https://scan.example.com", true, false, false, true};
    const auto result = application::RunFullPipelineUseCase::validate(req);
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.value().targetUrl, "https://scan.example.com");
}

TEST(StartZapScanUseCaseTest, RejectsInvalidUrl) {
    EXPECT_FALSE(application::StartZapScanUseCase::validateTarget("not-a-url").isOk());
}

TEST(StartZapScanUseCaseTest, ReturnsNormalizedTarget) {
    const auto result = application::StartZapScanUseCase::validateTarget("https://zap.test/");
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.value(), "https://zap.test/");
}
