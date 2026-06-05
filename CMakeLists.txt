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

    src/net/TcpSocket.cpp
    src/net/UdpSocket.cpp

    src/rtsp/RtspUrl.cpp
    src/rtsp/RtspRequest.cpp
    src/rtsp/RtspResponse.cpp
    src/rtsp/RtspSession.cpp
    src/rtsp/RtspAuth.cpp
    src/rtsp/SdpParser.cpp

    src/rtp/RtpPacket.cpp
    src/rtp/RtpParser.cpp
    src/rtp/RtpStats.cpp

    src/h264/NalUnit.cpp
    src/h264/H264Analyzer.cpp
    src/metrics/StreamMetrics.cpp
    src/metrics/MetricsCollector.cpp
    src/rtsp/RtspClient.cpp
    src/rtsp/InterleavedFrameReader.cpp
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

enable_testing()
add_subdirectory(tests)