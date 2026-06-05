#include "rtsi/rtsp/RtspSession.hpp"
#include "rtsi/rtsp/RtspUrl.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("RTSP session starts in initial state", "[rtsp][session]") {
    auto url = rtsi::RtspUrl::parse("rtsp://user:pass@192.168.1.50:554/stream1");
    const rtsi::RtspSession session(url);

    REQUIRE(session.state() == rtsi::RtspSessionState::Initial);
    REQUIRE(session.peek_next_cseq() == 1);
    REQUIRE_FALSE(session.has_session_id());
    REQUIRE_FALSE(session.session_id().has_value());
}

TEST_CASE("RTSP session consumes CSeq progressively", "[rtsp][session]") {
    auto url = rtsi::RtspUrl::parse("rtsp://192.168.1.50:554/stream1");
    rtsi::RtspSession session(url);

    REQUIRE(session.peek_next_cseq() == 1);
    REQUIRE(session.consume_cseq() == 1);

    REQUIRE(session.peek_next_cseq() == 2);
    REQUIRE(session.consume_cseq() == 2);

    REQUIRE(session.peek_next_cseq() == 3);
}

TEST_CASE("RTSP session builds OPTIONS and DESCRIBE requests", "[rtsp][session]") {
    auto url = rtsi::RtspUrl::parse("rtsp://192.168.1.50:554/stream1");
    rtsi::RtspSession session(url);

    const auto options = session.make_options_request();
    REQUIRE(options.method() == rtsi::RtspMethod::Options);
    REQUIRE(options.cseq() == 1);
    REQUIRE(options.uri() == "rtsp://192.168.1.50:554/stream1");

    const auto describe = session.make_describe_request();
    REQUIRE(describe.method() == rtsi::RtspMethod::Describe);
    REQUIRE(describe.cseq() == 2);
    REQUIRE(describe.header("Accept").value() == "application/sdp");

    REQUIRE(session.peek_next_cseq() == 3);
}

TEST_CASE("RTSP session builds SETUP request", "[rtsp][session]") {
    auto url = rtsi::RtspUrl::parse("rtsp://192.168.1.50:554/stream1");
    rtsi::RtspSession session(url);

    const auto setup = session.make_setup_request(
        "rtsp://192.168.1.50:554/stream1/trackID=0",
        "RTP/AVP;unicast;client_port=5000-5001"
    );

    REQUIRE(setup.method() == rtsi::RtspMethod::Setup);
    REQUIRE(setup.cseq() == 1);
    REQUIRE(setup.uri() == "rtsp://192.168.1.50:554/stream1/trackID=0");
    REQUIRE(setup.header("Transport").value() == "RTP/AVP;unicast;client_port=5000-5001");
}

TEST_CASE("RTSP session stores session id manually", "[rtsp][session]") {
    auto url = rtsi::RtspUrl::parse("rtsp://192.168.1.50:554/stream1");
    rtsi::RtspSession session(url);

    session.set_session_id("12345678");

    REQUIRE(session.has_session_id());
    REQUIRE(session.session_id().value() == "12345678");

    session.clear_session_id();

    REQUIRE_FALSE(session.has_session_id());
}

TEST_CASE("RTSP session extracts session id from response", "[rtsp][session]") {
    auto url = rtsi::RtspUrl::parse("rtsp://192.168.1.50:554/stream1");
    rtsi::RtspSession session(url);

    const std::string raw_response =
        "RTSP/1.0 200 OK\r\n"
        "CSeq: 3\r\n"
        "Session: 12345678;timeout=60\r\n"
        "\r\n";

    const auto response = rtsi::RtspResponse::parse(raw_response);

    session.apply_response(response);

    REQUIRE(session.has_session_id());
    REQUIRE(session.session_id().value() == "12345678");
}

TEST_CASE("RTSP session builds PLAY request after session id is available", "[rtsp][session]") {
    auto url = rtsi::RtspUrl::parse("rtsp://192.168.1.50:554/stream1");
    rtsi::RtspSession session(url);

    session.set_session_id("12345678");

    const auto play = session.make_play_request();

    REQUIRE(play.method() == rtsi::RtspMethod::Play);
    REQUIRE(play.cseq() == 1);
    REQUIRE(play.header("Session").value() == "12345678");
    REQUIRE(play.uri() == "rtsp://192.168.1.50:554/stream1");
}

TEST_CASE("RTSP session builds TEARDOWN request after session id is available", "[rtsp][session]") {
    auto url = rtsi::RtspUrl::parse("rtsp://192.168.1.50:554/stream1");
    rtsi::RtspSession session(url);

    session.set_session_id("12345678");

    const auto teardown = session.make_teardown_request();

    REQUIRE(teardown.method() == rtsi::RtspMethod::Teardown);
    REQUIRE(teardown.cseq() == 1);
    REQUIRE(teardown.header("Session").value() == "12345678");
}

TEST_CASE("RTSP session rejects empty session id", "[rtsp][session]") {
    auto url = rtsi::RtspUrl::parse("rtsp://192.168.1.50:554/stream1");
    rtsi::RtspSession session(url);

    REQUIRE_THROWS_AS(
        session.set_session_id(""),
        std::invalid_argument
    );
}

TEST_CASE("RTSP session rejects PLAY without session id", "[rtsp][session]") {
    auto url = rtsi::RtspUrl::parse("rtsp://192.168.1.50:554/stream1");
    rtsi::RtspSession session(url);

    REQUIRE_THROWS_AS(
        session.make_play_request(),
        std::logic_error
    );
}

TEST_CASE("RTSP session rejects TEARDOWN without session id", "[rtsp][session]") {
    auto url = rtsi::RtspUrl::parse("rtsp://192.168.1.50:554/stream1");
    rtsi::RtspSession session(url);

    REQUIRE_THROWS_AS(
        session.make_teardown_request(),
        std::logic_error
    );
}

TEST_CASE("RTSP session state transitions", "[rtsp][session]") {
    auto url = rtsi::RtspUrl::parse("rtsp://192.168.1.50:554/stream1");
    rtsi::RtspSession session(url);

    REQUIRE(session.state() == rtsi::RtspSessionState::Initial);
    REQUIRE(rtsi::to_string(session.state()) == "initial");

    session.mark_described();
    REQUIRE(session.state() == rtsi::RtspSessionState::Described);
    REQUIRE(rtsi::to_string(session.state()) == "described");

    session.mark_setup();
    REQUIRE(session.state() == rtsi::RtspSessionState::Setup);
    REQUIRE(rtsi::to_string(session.state()) == "setup");

    session.mark_playing();
    REQUIRE(session.state() == rtsi::RtspSessionState::Playing);
    REQUIRE(rtsi::to_string(session.state()) == "playing");

    session.mark_closed();
    REQUIRE(session.state() == rtsi::RtspSessionState::Closed);
    REQUIRE(rtsi::to_string(session.state()) == "closed");
}