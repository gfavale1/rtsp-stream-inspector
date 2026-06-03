#include "rtsi/rtsp/RtspRequest.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Build OPTIONS request", "[rtsp][request]") {
    const auto request = rtsi::RtspRequest::options(
        "rtsp://192.168.1.50:554/stream1",
        1
    );

    REQUIRE(request.method() == rtsi::RtspMethod::Options);
    REQUIRE(request.uri() == "rtsp://192.168.1.50:554/stream1");
    REQUIRE(request.cseq() == 1);

    const auto serialized = request.serialize();

    REQUIRE(serialized.find("OPTIONS rtsp://192.168.1.50:554/stream1 RTSP/1.0\r\n") == 0);
    REQUIRE(serialized.find("CSeq: 1\r\n") != std::string::npos);
    REQUIRE(serialized.ends_with("\r\n\r\n"));
}

TEST_CASE("Build DESCRIBE request with SDP accept header", "[rtsp][request]") {
    const auto request = rtsi::RtspRequest::describe(
        "rtsp://192.168.1.50:554/stream1",
        2
    );

    REQUIRE(request.method() == rtsi::RtspMethod::Describe);
    REQUIRE(request.header("CSeq").value() == "2");
    REQUIRE(request.header("Accept").value() == "application/sdp");

    const auto serialized = request.serialize();

    REQUIRE(serialized.find("DESCRIBE rtsp://192.168.1.50:554/stream1 RTSP/1.0\r\n") == 0);
    REQUIRE(serialized.find("CSeq: 2\r\n") != std::string::npos);
    REQUIRE(serialized.find("Accept: application/sdp\r\n") != std::string::npos);
}

TEST_CASE("Build SETUP request with transport header", "[rtsp][request]") {
    const auto request = rtsi::RtspRequest::setup(
        "rtsp://192.168.1.50:554/stream1/trackID=0",
        3,
        "RTP/AVP;unicast;client_port=5000-5001"
    );

    REQUIRE(request.method() == rtsi::RtspMethod::Setup);
    REQUIRE(request.header("CSeq").value() == "3");
    REQUIRE(request.header("Transport").value() == "RTP/AVP;unicast;client_port=5000-5001");

    const auto serialized = request.serialize();

    REQUIRE(serialized.find("SETUP rtsp://192.168.1.50:554/stream1/trackID=0 RTSP/1.0\r\n") == 0);
    REQUIRE(serialized.find("Transport: RTP/AVP;unicast;client_port=5000-5001\r\n") != std::string::npos);
}

TEST_CASE("Build PLAY request with session header", "[rtsp][request]") {
    const auto request = rtsi::RtspRequest::play(
        "rtsp://192.168.1.50:554/stream1",
        4,
        "12345678"
    );

    REQUIRE(request.method() == rtsi::RtspMethod::Play);
    REQUIRE(request.header("CSeq").value() == "4");
    REQUIRE(request.header("Session").value() == "12345678");

    const auto serialized = request.serialize();

    REQUIRE(serialized.find("PLAY rtsp://192.168.1.50:554/stream1 RTSP/1.0\r\n") == 0);
    REQUIRE(serialized.find("Session: 12345678\r\n") != std::string::npos);
}

TEST_CASE("Build TEARDOWN request with session header", "[rtsp][request]") {
    const auto request = rtsi::RtspRequest::teardown(
        "rtsp://192.168.1.50:554/stream1",
        5,
        "12345678"
    );

    REQUIRE(request.method() == rtsi::RtspMethod::Teardown);
    REQUIRE(request.header("CSeq").value() == "5");
    REQUIRE(request.header("Session").value() == "12345678");

    const auto serialized = request.serialize();

    REQUIRE(serialized.find("TEARDOWN rtsp://192.168.1.50:554/stream1 RTSP/1.0\r\n") == 0);
    REQUIRE(serialized.find("Session: 12345678\r\n") != std::string::npos);
}

TEST_CASE("Reject empty URI", "[rtsp][request]") {
    REQUIRE_THROWS_AS(
        rtsi::RtspRequest::options("", 1),
        std::invalid_argument
    );
}

TEST_CASE("Reject zero CSeq", "[rtsp][request]") {
    REQUIRE_THROWS_AS(
        rtsi::RtspRequest::options("rtsp://camera.local/stream1", 0),
        std::invalid_argument
    );
}

TEST_CASE("Reject SETUP without transport", "[rtsp][request]") {
    REQUIRE_THROWS_AS(
        rtsi::RtspRequest::setup(
            "rtsp://camera.local/stream1",
            1,
            ""
        ),
        std::invalid_argument
    );
}

TEST_CASE("Reject PLAY without session id", "[rtsp][request]") {
    REQUIRE_THROWS_AS(
        rtsi::RtspRequest::play(
            "rtsp://camera.local/stream1",
            1,
            ""
        ),
        std::invalid_argument
    );
}