#include "rtsi/rtsp/RtspAuth.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Base64 encode basic values", "[rtsp][auth]") {
    REQUIRE(rtsi::base64_encode("user:password") == "dXNlcjpwYXNzd29yZA==");
    REQUIRE(rtsi::base64_encode("admin:admin") == "YWRtaW46YWRtaW4=");
    REQUIRE(rtsi::base64_encode("") == "");
}

TEST_CASE("Build Basic Authorization header value", "[rtsp][auth]") {
    const auto value = rtsi::make_basic_authorization_value("user", "password");

    REQUIRE(value == "Basic dXNlcjpwYXNzd29yZA==");
}

TEST_CASE("Reject empty Basic username", "[rtsp][auth]") {
    REQUIRE_THROWS_AS(
        rtsi::make_basic_authorization_value("", "password"),
        std::invalid_argument
    );
}