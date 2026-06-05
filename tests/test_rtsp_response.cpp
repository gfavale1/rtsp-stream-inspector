#include "rtsi/rtsp/RtspResponse.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Parse successful RTSP response", "[rtsp][response]") {
    const std::string raw =
        "RTSP/1.0 200 OK\r\n"
        "CSeq: 1\r\n"
        "Public: OPTIONS, DESCRIBE, SETUP, PLAY, TEARDOWN\r\n"
        "\r\n";

    const auto response = rtsi::RtspResponse::parse(raw);

    REQUIRE(response.version() == "RTSP/1.0");
    REQUIRE(response.status_code() == 200);
    REQUIRE(response.reason_phrase() == "OK");
    REQUIRE(response.is_success());
    REQUIRE(response.cseq().value() == 1);
    REQUIRE(response.header("Public").value() == "OPTIONS, DESCRIBE, SETUP, PLAY, TEARDOWN");
    REQUIRE(response.body().empty());
}

TEST_CASE("Parse RTSP response with SDP body", "[rtsp][response]") {
    const std::string body =
        "v=0\r\n"
        "m=video 0 RTP/AVP 96\r\n"
        "a=rtpmap:96 H264/90000\r\n";

    const std::string raw =
        "RTSP/1.0 200 OK\r\n"
        "CSeq: 2\r\n"
        "Content-Type: application/sdp\r\n"
        "Content-Length: " + std::to_string(body.size()) + "\r\n"
        "\r\n" +
        body;

    const auto response = rtsi::RtspResponse::parse(raw);

    REQUIRE(response.status_code() == 200);
    REQUIRE(response.cseq().value() == 2);
    REQUIRE(response.header("Content-Type").value() == "application/sdp");
    REQUIRE(response.content_length().value() == body.size());
    REQUIRE(response.body() == body);
}

TEST_CASE("Header lookup is case insensitive", "[rtsp][response]") {
    const std::string raw =
        "RTSP/1.0 200 OK\r\n"
        "CSeq: 3\r\n"
        "Content-Type: application/sdp\r\n"
        "\r\n";

    const auto response = rtsi::RtspResponse::parse(raw);

    REQUIRE(response.header("content-type").value() == "application/sdp");
    REQUIRE(response.header("CONTENT-TYPE").value() == "application/sdp");
    REQUIRE(response.header("CSeq").value() == "3");
    REQUIRE(response.header("cseq").value() == "3");
}

TEST_CASE("Parse session id without parameters", "[rtsp][response]") {
    const std::string raw =
        "RTSP/1.0 200 OK\r\n"
        "CSeq: 4\r\n"
        "Session: 12345678\r\n"
        "\r\n";

    const auto response = rtsi::RtspResponse::parse(raw);

    REQUIRE(response.session_id().value() == "12345678");
}

TEST_CASE("Parse session id with timeout parameter", "[rtsp][response]") {
    const std::string raw =
        "RTSP/1.0 200 OK\r\n"
        "CSeq: 5\r\n"
        "Session: 12345678;timeout=60\r\n"
        "\r\n";

    const auto response = rtsi::RtspResponse::parse(raw);

    REQUIRE(response.session_id().value() == "12345678");
}

TEST_CASE("Parse non-success RTSP response", "[rtsp][response]") {
    const std::string raw =
        "RTSP/1.0 401 Unauthorized\r\n"
        "CSeq: 6\r\n"
        "WWW-Authenticate: Basic realm=\"camera\"\r\n"
        "\r\n";

    const auto response = rtsi::RtspResponse::parse(raw);

    REQUIRE(response.status_code() == 401);
    REQUIRE(response.reason_phrase() == "Unauthorized");
    REQUIRE_FALSE(response.is_success());
    REQUIRE(response.header("WWW-Authenticate").value() == "Basic realm=\"camera\"");
}

TEST_CASE("Trim body to Content-Length when extra bytes are present", "[rtsp][response]") {
    const std::string raw =
        "RTSP/1.0 200 OK\r\n"
        "CSeq: 7\r\n"
        "Content-Length: 5\r\n"
        "\r\n"
        "helloEXTRA_BYTES";

    const auto response = rtsi::RtspResponse::parse(raw);

    REQUIRE(response.body() == "hello");
}

TEST_CASE("Reject empty response", "[rtsp][response]") {
    REQUIRE_THROWS_AS(
        rtsi::RtspResponse::parse(""),
        std::invalid_argument
    );
}

TEST_CASE("Reject response without separator", "[rtsp][response]") {
    REQUIRE_THROWS_AS(
        rtsi::RtspResponse::parse("RTSP/1.0 200 OK\r\nCSeq: 1\r\n"),
        std::invalid_argument
    );
}

TEST_CASE("Reject unsupported version", "[rtsp][response]") {
    const std::string raw =
        "HTTP/1.1 200 OK\r\n"
        "CSeq: 1\r\n"
        "\r\n";

    REQUIRE_THROWS_AS(
        rtsi::RtspResponse::parse(raw),
        std::invalid_argument
    );
}

TEST_CASE("Reject invalid header without colon", "[rtsp][response]") {
    const std::string raw =
        "RTSP/1.0 200 OK\r\n"
        "InvalidHeader\r\n"
        "\r\n";

    REQUIRE_THROWS_AS(
        rtsi::RtspResponse::parse(raw),
        std::invalid_argument
    );
}

TEST_CASE("Reject body shorter than Content-Length", "[rtsp][response]") {
    const std::string raw =
        "RTSP/1.0 200 OK\r\n"
        "CSeq: 1\r\n"
        "Content-Length: 20\r\n"
        "\r\n"
        "short";

    REQUIRE_THROWS_AS(
        rtsi::RtspResponse::parse(raw),
        std::invalid_argument
    );
}