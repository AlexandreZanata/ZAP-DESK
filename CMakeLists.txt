cmake_minimum_required(VERSION 3.16)
project(ZAP-DESK VERSION 0.3.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

option(ZAP_DESK_ENABLE_COVERAGE "Enable GCC coverage flags for tests" OFF)
option(ZAP_DESK_ENABLE_INTEGRATION_TESTS "Build optional ZAP integration tests" OFF)

if(ZAP_DESK_ENABLE_COVERAGE)
    add_compile_options(--coverage -fprofile-arcs -ftest-coverage)
    add_link_options(--coverage)
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

set(DOMAIN_SOURCES
    src/shared/errors/AppError.cpp
    src/domain/value_objects/TargetUrl.cpp
    src/domain/value_objects/ScanId.cpp
    src/domain/value_objects/RiskLevel.cpp
    src/domain/scan/Scan.cpp
    src/domain/alert/SecurityAlert.cpp
    src/application/usecases/StartReconUseCase.cpp
    src/application/usecases/FeedZapUseCase.cpp
    src/application/usecases/RunFullPipelineUseCase.cpp
    src/application/usecases/StartZapScanUseCase.cpp
    src/infrastructure/recon/JsonReconSummaryReader.cpp
)

set(APP_SOURCES
    src/main.cpp
    src/config/AppConfig.cpp
    src/core/ZapClient.cpp
    src/core/ZapDaemon.cpp
    src/services/ReconRunner.cpp
    src/services/ReconPreflight.cpp
    src/services/ZapUpdater.cpp
    src/infrastructure/recon/QtReconGateway.cpp
    src/infrastructure/recon/QtPreflightGateway.cpp
    src/infrastructure/zap/QtZapGateway.cpp
    src/presentation/ApplicationFacade.cpp
    src/ui/MainWindow.cpp
    ${DOMAIN_SOURCES}
)

add_executable(zap-desk ${APP_SOURCES})

target_include_directories(zap-desk PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(zap-desk PRIVATE
    Qt6::Widgets
    Qt6::Network
)

# ---------------------------------------------------------------------------
# GoogleTest unit tests
# ---------------------------------------------------------------------------

include(FetchContent)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)

enable_testing()

set(UNIT_TEST_SOURCES
    tests/unit/test_target_url.cpp
    tests/unit/test_usecases.cpp
    tests/unit/test_scan_entity.cpp
    tests/unit/test_risk_level.cpp
    tests/unit/test_summary_reader.cpp
)

add_executable(zap-desk-unit-tests
    ${UNIT_TEST_SOURCES}
    ${DOMAIN_SOURCES}
)

target_include_directories(zap-desk-unit-tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/tests
)

target_link_libraries(zap-desk-unit-tests PRIVATE
    GTest::gtest_main
    Qt6::Core
)

include(GoogleTest)
gtest_discover_tests(zap-desk-unit-tests)

# ---------------------------------------------------------------------------
# Optional ZAP integration tests (skipped unless ZAP is running locally)
# ---------------------------------------------------------------------------

if(ZAP_DESK_ENABLE_INTEGRATION_TESTS)
    add_executable(zap-desk-integration-tests
        tests/integration/test_zap_api.cpp
        src/core/ZapClient.cpp
    )

    target_include_directories(zap-desk-integration-tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_link_libraries(zap-desk-integration-tests PRIVATE
        GTest::gtest_main
        Qt6::Network
        Qt6::Core
    )

    gtest_discover_tests(zap-desk-integration-tests)
endif()

install(TARGETS zap-desk RUNTIME DESTINATION bin)
