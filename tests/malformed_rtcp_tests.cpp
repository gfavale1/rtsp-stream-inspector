#include "rtsi/rtcp/RtcpParser.hpp"
#include "rtsi/rtcp/RtcpStats.hpp"

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <stdexcept>
#include <vector>

namespace {

std::vector<std::uint8_t> minimal_sender_report() {
    return {
        0x80, 0xC8, 0x00, 0x06,
        0x01, 0x02, 0x03, 0x04,
        0x11, 0x11, 0x11, 0x11,
        0x22, 0x22, 0x22, 0x22,
        0x33, 0x33, 0x33, 0x33,
        0x00, 0x00, 0x00, 0x64,
        0x00, 0x00, 0x00, 0xC8,
    };
}

} // namespace

TEST_CASE("RtcpParser rejects empty and undersized RTCP packets", "[rtcp][malformed]") {
    REQUIRE_THROWS(rtsi::RtcpParser::parse_single_packet({}));
    REQUIRE_THROWS(rtsi::RtcpParser::parse_single_packet({0x80}));
    REQUIRE_THROWS(rtsi::RtcpParser::parse_single_packet({0x80, 0xC8}));
    REQUIRE_THROWS(rtsi::RtcpParser::parse_single_packet({0x80, 0xC8, 0x00}));
}

TEST_CASE("RtcpParser rejects unsupported RTCP version", "[rtcp][malformed]") {
    REQUIRE_THROWS(rtsi::RtcpParser::parse_single_packet({0x00, 0xC8, 0x00, 0x00}));
    REQUIRE_THROWS(rtsi::RtcpParser::parse_single_packet({0x40, 0xC8, 0x00, 0x00}));
    REQUIRE_THROWS(rtsi::RtcpParser::parse_single_packet({0xC0, 0xC8, 0x00, 0x00}));
}

TEST_CASE("RtcpParser rejects RTCP length larger than available buffer", "[rtcp][malformed]") {
    REQUIRE_THROWS(rtsi::RtcpParser::parse_single_packet({0x80, 0xC8, 0x00, 0x06}));
}

TEST_CASE("RtcpParser rejects Sender Report shorter than minimum body", "[rtcp][malformed]") {
    const std::vector<std::uint8_t> too_short_sr = {
        0x80, 0xC8, 0x00, 0x01,
        0x01, 0x02, 0x03, 0x04,
    };
    REQUIRE_THROWS(rtsi::RtcpParser::parse_single_packet(too_short_sr));
}

TEST_CASE("RtcpParser rejects Receiver Report shorter than minimum body", "[rtcp][malformed]") {
    const std::vector<std::uint8_t> too_short_rr = {0x80, 0xC9, 0x00, 0x00};
    REQUIRE_THROWS(rtsi::RtcpParser::parse_single_packet(too_short_rr));
}

TEST_CASE("RtcpParser rejects Receiver Report with incomplete report block", "[rtcp][malformed]") {
    const std::vector<std::uint8_t> incomplete_rr = {
        0x81, 0xC9, 0x00, 0x02,
        0x01, 0x02, 0x03, 0x04,
        0xAA, 0xBB, 0xCC, 0xDD,
    };
    REQUIRE_THROWS(rtsi::RtcpParser::parse_single_packet(incomplete_rr));
}

TEST_CASE("RtcpParser rejects truncated compound RTCP packet", "[rtcp][malformed]") {
    auto compound = minimal_sender_report();
    compound.push_back(0x80);
    compound.push_back(0xCA);

    REQUIRE_THROWS(rtsi::RtcpParser::parse_compound_packet(compound));
}

TEST_CASE("RtcpParser accepts coherent unknown RTCP packet type", "[rtcp][malformed]") {
    const std::vector<std::uint8_t> unknown = {0x80, 0xFA, 0x00, 0x00};

    const auto packet = rtsi::RtcpParser::parse_single_packet(unknown);
    REQUIRE(packet.packet_type == rtsi::RtcpPacketType::Unknown);
    REQUIRE(packet.packet_size_bytes == 4);
}

TEST_CASE("RtcpStats increments malformed counter deterministically", "[rtcp][stats][malformed]") {
    rtsi::RtcpStats stats;
    stats.observe_frame();
    stats.observe_malformed_packet();
    stats.observe_malformed_packet();

    const auto snapshot = stats.snapshot();
    REQUIRE(snapshot.frames_received == 1);
    REQUIRE(snapshot.malformed_packets == 2);
    REQUIRE(snapshot.packets_parsed == 0);
}
