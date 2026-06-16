cmake_minimum_required(VERSION 3.16)
project(ZAP-DESK VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

set(SOURCES
    src/main.cpp
    src/config/AppConfig.cpp
    src/core/ZapClient.cpp
    src/core/ZapDaemon.cpp
    src/services/ReconRunner.cpp
    src/services/ReconBridge.cpp
    src/services/ZapUpdater.cpp
    src/ui/MainWindow.cpp
)

add_executable(zap-desk ${SOURCES})

target_include_directories(zap-desk PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(zap-desk PRIVATE
    Qt6::Widgets
    Qt6::Network
)

install(TARGETS zap-desk RUNTIME DESTINATION bin)
