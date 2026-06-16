#include "application/usecases/FeedZapUseCase.hpp"
#include "application/usecases/StartReconUseCase.hpp"
#include "application/usecases/StartZapScanUseCase.hpp"
#include "domain/scan/Scan.hpp"
#include "domain/value_objects/TargetUrl.hpp"
#include "infrastructure/recon/JsonReconSummaryReader.hpp"

#include <cassert>
#include <filesystem>
#include <iostream>
#include <string>

static int failures = 0;

#define EXPECT_TRUE(expr)                                   \
    do {                                                    \
        if (!(expr)) {                                      \
            std::cerr << "FAIL: " #expr << '\n';            \
            ++failures;                                     \
        }                                                   \
    } while (0)

static void testTargetUrl() {
    auto bad = domain::TargetUrl::create("");
    EXPECT_TRUE(!bad.isOk());

    auto badHost = domain::TargetUrl::create("https://");
    EXPECT_TRUE(!badHost.isOk());

    auto ok = domain::TargetUrl::create("https://example.com");
    EXPECT_TRUE(ok.isOk());
    EXPECT_TRUE(ok.value().value() == "https://example.com");
}

static void testStartReconUseCase() {
    application::StartReconRequest req{"https://example.com", false, false, false, true};
    auto denied = application::StartReconUseCase::validate(req);
    EXPECT_TRUE(!denied.isOk());

    req.authorized = true;
    auto ok = application::StartReconUseCase::validate(req);
    EXPECT_TRUE(ok.isOk());
    EXPECT_TRUE(ok.value().scan.status() == domain::ScanStatus::Pending);
}

static void testFeedZapUseCase() {
    application::FeedZapRequest req{"", false};
    EXPECT_TRUE(!application::FeedZapUseCase::validate(req).isOk());

    req.summaryPath = "/tmp/summary.json";
    req.zapRunning = false;
    EXPECT_TRUE(!application::FeedZapUseCase::validate(req).isOk());

    req.zapRunning = true;
    EXPECT_TRUE(application::FeedZapUseCase::validate(req).isOk());
}

static void testSummaryReader() {
    const std::filesystem::path fixture =
        std::filesystem::path(__FILE__).parent_path().parent_path() / "fixtures" / "summary-sample.json";

    infrastructure::JsonReconSummaryReader reader;
    const auto stats = reader.load(fixture.string());
    EXPECT_TRUE(stats.valid);
    EXPECT_TRUE(stats.subdomains == 3);
    EXPECT_TRUE(stats.liveHosts == 2);

    const auto urls = reader.loadUrls(fixture.string());
    EXPECT_TRUE(!urls.empty());
}

int main() {
    testTargetUrl();
    testStartReconUseCase();
    testFeedZapUseCase();
    testSummaryReader();

    if (failures > 0) {
        std::cerr << failures << " domain test(s) failed\n";
        return 1;
    }

    std::cout << "All domain tests passed\n";
    return 0;
}
