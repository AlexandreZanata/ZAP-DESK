cmake_minimum_required(VERSION 3.16)
project(ZAP-DESK VERSION 0.3.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

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

add_executable(zap-desk-domain-tests
    tests/unit/test_domain.cpp
    ${DOMAIN_SOURCES}
)

target_include_directories(zap-desk-domain-tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(zap-desk-domain-tests PRIVATE
    Qt6::Core
)

install(TARGETS zap-desk RUNTIME DESTINATION bin)
