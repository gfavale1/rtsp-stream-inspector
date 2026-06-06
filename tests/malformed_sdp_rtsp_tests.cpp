#include "rtsi/rtsp/RtspResponse.hpp"
#include "rtsi/rtsp/SdpParser.hpp"

#include <catch2/catch_test_macros.hpp>

#include <stdexcept>
#include <string>

TEST_CASE("SdpParser rejects empty SDP and SDP without media tracks", "[sdp][malformed]") {
    REQUIRE_THROWS_AS(rtsi::SdpParser::parse(""), std::invalid_argument);
    REQUIRE_THROWS_AS(rtsi::SdpParser::parse("v=0\r\ns=No media\r\n"), std::invalid_argument);
}

TEST_CASE("SdpParser accepts audio-only SDP without video track", "[sdp][malformed]") {
    const std::string sdp =
        "v=0\r\n"
        "s=Audio only\r\n"
        "m=audio 0 RTP/AVP 8\r\n"
        "a=rtpmap:8 PCMA/8000\r\n"
        "a=control:track2\r\n";

    const auto parsed = rtsi::SdpParser::parse(sdp);
    REQUIRE(parsed.tracks.size() == 1);
    REQUIRE_FALSE(parsed.first_video_track().has_value());
    REQUIRE(parsed.first_audio_track().has_value());
}

TEST_CASE("SdpParser handles video media line without rtpmap", "[sdp][malformed]") {
    const std::string sdp =
        "v=0\r\n"
        "s=Video without rtpmap\r\n"
        "m=video 0 RTP/AVP 96\r\n"
        "a=control:track1\r\n";

    const auto parsed = rtsi::SdpParser::parse(sdp);
    const auto video = parsed.first_video_track();
    REQUIRE(video.has_value());
    REQUIRE(video->payload_type == 96);
    REQUIRE(video->codec.empty());
    REQUIRE(video->clock_rate == -1);
    REQUIRE(video->control == "track1");
}

TEST_CASE("SdpParser rejects malformed rtpmap numeric fields", "[sdp][malformed]") {
    REQUIRE_THROWS_AS(
        rtsi::SdpParser::parse(
            "v=0\r\n"
            "s=Bad payload\r\n"
            "m=video 0 RTP/AVP 96\r\n"
            "a=rtpmap:abc H264/90000\r\n"),
        std::invalid_argument);

    REQUIRE_THROWS_AS(
        rtsi::SdpParser::parse(
            "v=0\r\n"
            "s=Bad clock\r\n"
            "m=video 0 RTP/AVP 96\r\n"
            "a=rtpmap:96 H264/not_numeric\r\n"),
        std::invalid_argument);
}

TEST_CASE("SdpParser ignores incomplete rtpmap instead of crashing", "[sdp][malformed]") {
    const auto parsed = rtsi::SdpParser::parse(
        "v=0\r\n"
        "s=Incomplete rtpmap\r\n"
        "m=video 0 RTP/AVP 96\r\n"
        "a=rtpmap:96\r\n"
        "a=control:track1\r\n");

    const auto video = parsed.first_video_track();
    REQUIRE(video.has_value());
    REQUIRE(video->payload_type == 96);
    REQUIRE(video->codec.empty());
}

TEST_CASE("SdpParser selects video track from audio plus video SDP", "[sdp][malformed]") {
    const std::string sdp =
        "v=0\r\n"
        "s=Multiple tracks\r\n"
        "m=audio 0 RTP/AVP 8\r\n"
        "a=rtpmap:8 PCMA/8000\r\n"
        "a=control:track2\r\n"
        "m=video 0 RTP/AVP 96\r\n"
        "a=rtpmap:96 H264/90000\r\n"
        "a=control:track1\r\n";

    const auto parsed = rtsi::SdpParser::parse(sdp);
    REQUIRE(parsed.tracks.size() == 2);
    REQUIRE(parsed.first_video_track().has_value());
    REQUIRE(parsed.first_video_track()->control == "track1");
}

TEST_CASE("RtspResponse rejects malformed response envelopes", "[rtsp][response][malformed]") {
    REQUIRE_THROWS_AS(rtsi::RtspResponse::parse(""), std::invalid_argument);
    REQUIRE_THROWS_AS(rtsi::RtspResponse::parse("HELLO\r\n\r\n"), std::invalid_argument);
    REQUIRE_THROWS_AS(rtsi::RtspResponse::parse("RTSP/1.0\r\n\r\n"), std::invalid_argument);
    REQUIRE_THROWS_AS(rtsi::RtspResponse::parse("RTSP/1.0 ABC Broken\r\n\r\n"), std::invalid_argument);
    REQUIRE_THROWS_AS(rtsi::RtspResponse::parse("RTSP/1.0 200 OK\r\nCSeq: 1"), std::invalid_argument);
}

TEST_CASE("RtspResponse rejects invalid Content-Length values", "[rtsp][response][malformed]") {
    REQUIRE_THROWS_AS(
        rtsi::RtspResponse::parse("RTSP/1.0 200 OK\r\nContent-Length: abc\r\n\r\n"),
        std::invalid_argument);

    REQUIRE_THROWS_AS(
        rtsi::RtspResponse::parse("RTSP/1.0 200 OK\r\nContent-Length: 10\r\n\r\nshort"),
        std::invalid_argument);
}

TEST_CASE("RtspResponse handles long headers and 401 without challenge", "[rtsp][response][malformed]") {
    const std::string long_value(4096, 'x');
    const auto response = rtsi::RtspResponse::parse(
        "RTSP/1.0 200 OK\r\nX-Long-Header: " + long_value + "\r\n\r\n");
    REQUIRE(response.is_success());
    REQUIRE(response.header("X-Long-Header").value() == long_value);

    const auto unauthorized = rtsi::RtspResponse::parse("RTSP/1.0 401 Unauthorized\r\nCSeq: 2\r\n\r\n");
    REQUIRE(unauthorized.status_code() == 401);
    REQUIRE_FALSE(unauthorized.header("WWW-Authenticate").has_value());
}
