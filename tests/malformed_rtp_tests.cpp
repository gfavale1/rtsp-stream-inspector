#include "rtsi/rtp/RtpParser.hpp"
#include "rtsi/rtp/RtpStats.hpp"

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <stdexcept>
#include <vector>

namespace {

std::vector<std::uint8_t> make_rtp_packet(std::uint16_t sequence) {
    return {
        0x80, 0x60,
        static_cast<std::uint8_t>((sequence >> 8U) & 0xFFU),
        static_cast<std::uint8_t>(sequence & 0xFFU),
        0x01, 0x02, 0x03, 0x04,
        0x0A, 0x0B, 0x0C, 0x0D,
        0x65,
    };
}

void observe_sequence(rtsi::RtpStats& stats, std::uint16_t sequence) {
    const auto bytes = make_rtp_packet(sequence);
    const auto packet = rtsi::RtpParser::parse(bytes);
    stats.update(packet, bytes.size());
}

} // namespace

TEST_CASE("RtpParser rejects empty and undersized RTP packets", "[rtp][malformed]") {
    REQUIRE_THROWS_AS(rtsi::RtpParser::parse({}), std::invalid_argument);
    REQUIRE_THROWS_AS(rtsi::RtpParser::parse({0x80}), std::invalid_argument);
    REQUIRE_THROWS_AS(rtsi::RtpParser::parse({0x80, 0x60, 0x00, 0x01}), std::invalid_argument);
    REQUIRE_THROWS_AS(
        rtsi::RtpParser::parse({0x80, 0x60, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0xAA, 0xBB, 0xCC}),
        std::invalid_argument);
}

TEST_CASE("RtpParser rejects unsupported RTP versions", "[rtp][malformed]") {
    auto packet = make_rtp_packet(1);

    packet[0] = 0x00;
    REQUIRE_THROWS_AS(rtsi::RtpParser::parse(packet), std::invalid_argument);

    packet[0] = 0x40;
    REQUIRE_THROWS_AS(rtsi::RtpParser::parse(packet), std::invalid_argument);

    packet[0] = 0xC0;
    REQUIRE_THROWS_AS(rtsi::RtpParser::parse(packet), std::invalid_argument);
}

TEST_CASE("RtpParser rejects packets shorter than declared CSRC list", "[rtp][malformed]") {
    auto packet = make_rtp_packet(1);
    packet[0] = 0x83; // V=2, CC=3, requires 12 extra CSRC bytes.
    REQUIRE_THROWS_AS(rtsi::RtpParser::parse(packet), std::invalid_argument);
}

TEST_CASE("RtpParser rejects missing RTP extension header and payload", "[rtp][malformed]") {
    auto packet = make_rtp_packet(1);
    packet[0] = 0x90; // V=2, X=1, no extension header present.
    REQUIRE_THROWS_AS(rtsi::RtpParser::parse(packet), std::invalid_argument);

    auto extension_too_short = make_rtp_packet(2);
    extension_too_short[0] = 0x90;
    extension_too_short.resize(14);
    REQUIRE_THROWS_AS(rtsi::RtpParser::parse(extension_too_short), std::invalid_argument);

    auto extension_length_exceeds_buffer = make_rtp_packet(3);
    extension_length_exceeds_buffer[0] = 0x90;
    extension_length_exceeds_buffer.resize(16);
    extension_length_exceeds_buffer[14] = 0x00;
    extension_length_exceeds_buffer[15] = 0x02; // 8 extension payload bytes, not present.
    REQUIRE_THROWS_AS(rtsi::RtpParser::parse(extension_length_exceeds_buffer), std::invalid_argument);
}

TEST_CASE("RtpParser accepts valid RTP header with empty payload", "[rtp][malformed]") {
    auto packet = make_rtp_packet(7);
    packet.resize(12);

    const auto parsed = rtsi::RtpParser::parse(packet);
    REQUIRE(parsed.sequence_number == 7);
    REQUIRE(parsed.header_size == 12);
    REQUIRE(parsed.payload.empty());
    REQUIRE(parsed.payload_size() == 0);
}

TEST_CASE("RtpStats handles sequence wrap-around without false loss", "[rtp][stats][malformed]") {
    rtsi::RtpStats stats;
    observe_sequence(stats, 65534);
    observe_sequence(stats, 65535);
    observe_sequence(stats, 0);
    observe_sequence(stats, 1);

    const auto snapshot = stats.snapshot();
    REQUIRE(snapshot.packets_received == 4);
    REQUIRE(snapshot.packets_lost == 0);
    REQUIRE(snapshot.out_of_order_packets == 0);
}

TEST_CASE("RtpStats detects controlled out-of-order packet", "[rtp][stats][malformed]") {
    rtsi::RtpStats stats;
    observe_sequence(stats, 100);
    observe_sequence(stats, 101);
    observe_sequence(stats, 103);
    observe_sequence(stats, 102);

    const auto snapshot = stats.snapshot();
    REQUIRE(snapshot.packets_received == 4);
    REQUIRE(snapshot.packets_lost == 1);
    REQUIRE(snapshot.out_of_order_packets == 1);
}

TEST_CASE("RtpStats estimates controlled sequence gap", "[rtp][stats][malformed]") {
    rtsi::RtpStats stats;
    observe_sequence(stats, 100);
    observe_sequence(stats, 101);
    observe_sequence(stats, 105);

    const auto snapshot = stats.snapshot();
    REQUIRE(snapshot.packets_received == 3);
    REQUIRE(snapshot.packets_lost == 3);
    REQUIRE(snapshot.out_of_order_packets == 0);
}
