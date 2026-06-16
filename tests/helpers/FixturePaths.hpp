#pragma once

#include <filesystem>
#include <string>

namespace testutil {

inline std::filesystem::path fixturesDir() {
    return std::filesystem::path(__FILE__).parent_path().parent_path() / "fixtures";
}

inline std::string summaryFixturePath() {
    return (fixturesDir() / "summary-sample.json").string();
}

}  // namespace testutil
