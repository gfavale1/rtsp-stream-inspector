cmake_minimum_required(VERSION 3.20)

project(rtsp_stream_inspector
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/CompilerOptions.cmake)
include(cmake/Dependencies.cmake)

add_library(rtsi_core
    src/app/AnalyzerConfig.cpp
    src/report/JsonReportWriter.cpp
)

target_include_directories(rtsi_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(rtsi_core
    PUBLIC
        nlohmann_json::nlohmann_json
)

add_executable(rtsp-inspector
    src/main.cpp
)

target_link_libraries(rtsp-inspector
    PRIVATE
        rtsi_core
        CLI11::CLI11
)