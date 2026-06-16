#include <gtest/gtest.h>

#include <algorithm>
#include <vector>

#include "infrastructure/recon/JsonReconSummaryReader.hpp"
#include "tests/helpers/FixturePaths.hpp"

TEST(JsonReconSummaryReaderTest, LoadsStatisticsFromFixture) {
    infrastructure::JsonReconSummaryReader reader;
    const auto stats = reader.load(testutil::summaryFixturePath());

    ASSERT_TRUE(stats.valid);
    EXPECT_EQ(stats.subdomains, 3);
    EXPECT_EQ(stats.liveHosts, 2);
    EXPECT_EQ(stats.openPorts, 1);
    EXPECT_EQ(stats.paths, 1);
    EXPECT_EQ(stats.vulnerabilities, 1);
}

TEST(JsonReconSummaryReaderTest, LoadsUrlsFromFixture) {
    infrastructure::JsonReconSummaryReader reader;
    const auto urls = reader.loadUrls(testutil::summaryFixturePath());

    ASSERT_FALSE(urls.empty());
    EXPECT_NE(urls.end(), std::find(urls.begin(), urls.end(), "https://example.com"));
}

TEST(JsonReconSummaryReaderTest, ReturnsEmptyForMissingFile) {
    infrastructure::JsonReconSummaryReader reader;
    const auto stats = reader.load("/nonexistent/summary.json");
    EXPECT_FALSE(stats.valid);
    EXPECT_TRUE(reader.loadUrls("/nonexistent/summary.json").empty());
}
