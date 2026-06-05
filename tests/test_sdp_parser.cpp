#include "rtsi/rtsp/SdpParser.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Parse Tapo C200 SDP with video and audio tracks", "[rtsp][sdp]") {
    const std::string sdp =
        "v=0\r\n"
        "o=- 14665860 31787219 1 IN IP4 192.168.0.242\r\n"
        "s=Session streamed by \"TP-LINK RTSP Server\"\r\n"
        "t=0 0\r\n"
        "m=video 0 RTP/AVP 96\r\n"
        "c=IN IP4 0.0.0.0\r\n"
        "b=AS:4096\r\n"
        "a=range:npt=0-\r\n"
        "a=control:track1\r\n"
        "a=rtpmap:96 H264/90000\r\n"
        "a=fmtp:96 packetization-mode=1; profile-level-id=64001F; "
        "sprop-parameter-sets=Z2QAH6w7UCgC3QgAAAMACAAAAwD0IA==,aO484QBCQgCEhARMUhuTxXyfk/k/J8nm5MkkLCJCkJyeT6/J/X5PrycmpMA=\r\n"
        "m=audio 0 RTP/AVP 8\r\n"
        "a=rtpmap:8 PCMA/8000\r\n"
        "a=control:track2\r\n";

    const auto parsed = rtsi::SdpParser::parse(sdp);

    REQUIRE(parsed.session_name == "Session streamed by \"TP-LINK RTSP Server\"");
    REQUIRE(parsed.tracks.size() == 2);

    const auto video = parsed.first_video_track();

    REQUIRE(video.has_value());
    REQUIRE(video->media_type == "video");
    REQUIRE(video->protocol == "RTP/AVP");
    REQUIRE(video->payload_type == 96);
    REQUIRE(video->codec == "H264");
    REQUIRE(video->clock_rate == 90000);
    REQUIRE(video->control == "track1");
    REQUIRE(video->fmtp.find("packetization-mode=1") != std::string::npos);
    REQUIRE(video->fmtp.find("profile-level-id=64001F") != std::string::npos);

    const auto audio = parsed.first_audio_track();

    REQUIRE(audio.has_value());
    REQUIRE(audio->media_type == "audio");
    REQUIRE(audio->payload_type == 8);
    REQUIRE(audio->codec == "PCMA");
    REQUIRE(audio->clock_rate == 8000);
    REQUIRE(audio->control == "track2");
}

TEST_CASE("Build absolute control URI from relative SDP control", "[rtsp][sdp]") {
    const auto uri = rtsi::build_control_uri(
        "rtsp://192.168.0.242:554/stream2",
        "track1"
    );

    REQUIRE(uri == "rtsp://192.168.0.242:554/stream2/track1");
}

TEST_CASE("Control URI remains unchanged when already absolute", "[rtsp][sdp]") {
    const auto uri = rtsi::build_control_uri(
        "rtsp://192.168.0.242:554/stream2",
        "rtsp://192.168.0.242:554/custom-track"
    );

    REQUIRE(uri == "rtsp://192.168.0.242:554/custom-track");
}

TEST_CASE("Reject empty SDP", "[rtsp][sdp]") {
    REQUIRE_THROWS_AS(
        rtsi::SdpParser::parse(""),
        std::invalid_argument
    );
}

TEST_CASE("Reject SDP without media tracks", "[rtsp][sdp]") {
    const std::string sdp =
        "v=0\r\n"
        "s=No media\r\n";

    REQUIRE_THROWS_AS(
        rtsi::SdpParser::parse(sdp),
        std::invalid_argument
    );
}