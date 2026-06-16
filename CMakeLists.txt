cmake_minimum_required(VERSION 3.21)
project(ZAP-DESK VERSION 0.8.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

option(ZAP_DESK_ENABLE_COVERAGE "Enable GCC coverage flags for tests" OFF)
option(ZAP_DESK_ENABLE_INTEGRATION_TESTS "Build optional ZAP integration tests" OFF)
option(ZAP_DESK_BUILD_TESTS "Build GoogleTest unit tests" ON)

if(ZAP_DESK_ENABLE_COVERAGE)
    add_compile_options(--coverage -fprofile-arcs -ftest-coverage)
    add_link_options(--coverage)
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

find_package(KF6ColorScheme REQUIRED)
find_package(KF6Config REQUIRED)

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

set(COMPONENT_SOURCES
    src/components/AppTheme.cpp
    src/components/KdeTheme.cpp
    src/components/UiKit.cpp
    src/components/HackerTheme.cpp
    src/components/LogConsole.cpp
    src/components/ScanProgressBar.cpp
    src/components/ReconSummaryPanel.cpp
    src/components/FindingsTable.cpp
    src/components/ScanHistorySidebar.cpp
    src/components/StatusBanner.cpp
    src/components/CrtOverlay.cpp
    src/components/SettingsDialog.cpp
)

set(SECURITY_SOURCES
    src/security/CredentialStore.cpp
    src/security/AuditLogger.cpp
    src/security/ReconRateLimiter.cpp
    src/security/KeyringBackend.cpp
)

set(APP_SOURCES
    src/main.cpp
    resources/app.qrc
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
    ${COMPONENT_SOURCES}
    ${SECURITY_SOURCES}
    ${DOMAIN_SOURCES}
)

add_executable(zap-desk ${APP_SOURCES})

target_include_directories(zap-desk PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(zap-desk PRIVATE
    Qt6::Widgets
    Qt6::Network
    KF6::ColorScheme
    KF6::ConfigCore
)

# ---------------------------------------------------------------------------
# GoogleTest unit tests
# ---------------------------------------------------------------------------

if(ZAP_DESK_BUILD_TESTS)

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
    tests/unit/test_zap_client_auth.cpp
    tests/unit/test_recon_rate_limiter.cpp
    tests/unit/test_audit_logger.cpp
    tests/unit/test_credential_store.cpp
)

add_executable(zap-desk-unit-tests
    ${UNIT_TEST_SOURCES}
    ${DOMAIN_SOURCES}
    src/core/ZapClient.cpp
    ${SECURITY_SOURCES}
)

target_include_directories(zap-desk-unit-tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/tests
)

target_link_libraries(zap-desk-unit-tests PRIVATE
    GTest::gtest_main
    Qt6::Core
    Qt6::Network
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

endif()  # ZAP_DESK_BUILD_TESTS

install(TARGETS zap-desk RUNTIME DESTINATION bin)

install(FILES packaging/zap-desk.desktop DESTINATION share/applications)
install(FILES packaging/icons/zap-desk.svg DESTINATION share/icons/hicolor/scalable/apps)

install(DIRECTORY reconner/
    DESTINATION share/zap-desk/reconner
    USE_SOURCE_PERMISSIONS
    PATTERN "__pycache__" EXCLUDE
    PATTERN "*.pyc" EXCLUDE
    PATTERN ".pytest_cache" EXCLUDE
    PATTERN "*.egg-info" EXCLUDE
)

install(PROGRAMS
    scripts/zap-launch.sh
    scripts/update-zap.sh
    DESTINATION share/zap-desk/scripts
)

# ---------------------------------------------------------------------------
# CPack — .deb package (make package-deb)
# ---------------------------------------------------------------------------

set(CPACK_PACKAGE_NAME "zap-desk")
set(CPACK_PACKAGE_VENDOR "ZAP-DESK")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "OWASP ZAP + Reconner security terminal")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_CONTACT "https://github.com/AlexandreZanata/ZAP-DESK")
set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_DEPENDS
    "libqt6widgets6 (>= 6.2), libqt6network6 (>= 6.2), libkf6colorscheme6, libkf6configcore6, python3, python3-pip, curl"
)
set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "nmap, chromium-browser | chromium")
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
    "${CMAKE_SOURCE_DIR}/packaging/debian/postinst"
)

include(CPack)
