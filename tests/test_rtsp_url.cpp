#include "rtsi/rtsp/RtspUrl.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Parse full RTSP URL with credentials, port and path", "[rtsp][url]") {
    const auto url = rtsi::RtspUrl::parse(
        "rtsp://user:password@192.168.1.50:554/stream1"
    );

    REQUIRE(url.raw == "rtsp://user:password@192.168.1.50:554/stream1");
    REQUIRE(url.scheme == "rtsp");
    REQUIRE(url.username == "user");
    REQUIRE(url.password == "password");
    REQUIRE(url.host == "192.168.1.50");
    REQUIRE(url.port == 554);
    REQUIRE(url.path == "/stream1");
    REQUIRE(url.has_explicit_port);
    REQUIRE(url.has_credentials());
}

TEST_CASE("Parse RTSP URL without credentials", "[rtsp][url]") {
    const auto url = rtsi::RtspUrl::parse(
        "rtsp://192.168.1.50:8554/live"
    );

    REQUIRE(url.scheme == "rtsp");
    REQUIRE(url.username.empty());
    REQUIRE(url.password.empty());
    REQUIRE(url.host == "192.168.1.50");
    REQUIRE(url.port == 8554);
    REQUIRE(url.path == "/live");
    REQUIRE(url.has_explicit_port);
    REQUIRE_FALSE(url.has_credentials());
}

TEST_CASE("Parse RTSP URL with default port", "[rtsp][url]") {
    const auto url = rtsi::RtspUrl::parse(
        "rtsp://camera.local/stream1"
    );

    REQUIRE(url.scheme == "rtsp");
    REQUIRE(url.host == "camera.local");
    REQUIRE(url.port == 554);
    REQUIRE(url.path == "/stream1");
    REQUIRE_FALSE(url.has_explicit_port);
}

TEST_CASE("Parse RTSP URL with username only", "[rtsp][url]") {
    const auto url = rtsi::RtspUrl::parse(
        "rtsp://admin@camera.local/stream1"
    );

    REQUIRE(url.username == "admin");
    REQUIRE(url.password.empty());
    REQUIRE(url.host == "camera.local");
    REQUIRE(url.port == 554);
    REQUIRE(url.path == "/stream1");
    REQUIRE(url.has_credentials());
}

TEST_CASE("Parse RTSP URL without explicit path", "[rtsp][url]") {
    const auto url = rtsi::RtspUrl::parse(
        "rtsp://camera.local"
    );

    REQUIRE(url.host == "camera.local");
    REQUIRE(url.port == 554);
    REQUIRE(url.path == "/");
}

TEST_CASE("Reject unsupported schemes", "[rtsp][url]") {
    REQUIRE_THROWS_AS(
        rtsi::RtspUrl::parse("http://camera.local/stream1"),
        std::invalid_argument
    );
}

TEST_CASE("Reject invalid ports", "[rtsp][url]") {
    REQUIRE_THROWS_AS(
        rtsi::RtspUrl::parse("rtsp://camera.local:abc/stream1"),
        std::invalid_argument
    );

    REQUIRE_THROWS_AS(
        rtsi::RtspUrl::parse("rtsp://camera.local:70000/stream1"),
        std::invalid_argument
    );
}

TEST_CASE("Reject empty URL", "[rtsp][url]") {
    REQUIRE_THROWS_AS(
        rtsi::RtspUrl::parse(""),
        std::invalid_argument
    );
}

TEST_CASE("Build RTSP authority string", "[rtsp][url]") {
    const auto url = rtsi::RtspUrl::parse(
        "rtsp://user:password@192.168.1.50:554/stream1"
    );

    REQUIRE(url.authority() == "user:password@192.168.1.50:554");
}