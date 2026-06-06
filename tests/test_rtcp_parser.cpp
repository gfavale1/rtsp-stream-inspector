#include "rtsi/rtcp/RtcpParser.hpp"

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <vector>

namespace {

void append_be16(std::vector<std::uint8_t>& bytes, std::uint16_t value) {
    bytes.push_back(static_cast<std::uint8_t>((value >> 8U) & 0xFFU));
    bytes.push_back(static_cast<std::uint8_t>(value & 0xFFU));
}

void append_be24(std::vector<std::uint8_t>& bytes, std::uint32_t value) {
    bytes.push_back(static_cast<std::uint8_t>((value >> 16U) & 0xFFU));
    bytes.push_back(static_cast<std::uint8_t>((value >> 8U) & 0xFFU));
    bytes.push_back(static_cast<std::uint8_t>(value & 0xFFU));
}

void append_be32(std::vector<std::uint8_t>& bytes, std::uint32_t value) {
    bytes.push_back(static_cast<std::uint8_t>((value >> 24U) & 0xFFU));
    bytes.push_back(static_cast<std::uint8_t>((value >> 16U) & 0xFFU));
    bytes.push_back(static_cast<std::uint8_t>((value >> 8U) & 0xFFU));
    bytes.push_back(static_cast<std::uint8_t>(value & 0xFFU));
}

std::vector<std::uint8_t> minimal_sender_report() {
    std::vector<std::uint8_t> bytes;
    bytes.push_back(0x80); // V=2, P=0, RC=0
    bytes.push_back(200);
    append_be16(bytes, 6); // 28 bytes / 4 - 1
    append_be32(bytes, 0x01020304);
    append_be32(bytes, 0x11111111);
    append_be32(bytes, 0x22222222);
    append_be32(bytes, 0x33333333);
    append_be32(bytes, 100);
    append_be32(bytes, 200);
    return bytes;
}

} // namespace

TEST_CASE("RtcpParser parses a minimal Sender Report", "[rtcp][parser]") {
    const auto packet = rtsi::RtcpParser::parse_single_packet(minimal_sender_report());

    REQUIRE(packet.version == 2);
    REQUIRE(packet.packet_type == rtsi::RtcpPacketType::SenderReport);
    REQUIRE(packet.report_count == 0);
    REQUIRE(packet.packet_size_bytes == 28);
    REQUIRE(packet.sender_info.has_value());
    REQUIRE(packet.sender_info->sender_ssrc == 0x01020304);
    REQUIRE(packet.sender_info->ntp_timestamp == 0x1111111122222222ULL);
    REQUIRE(packet.sender_info->rtp_timestamp == 0x33333333);
    REQUIRE(packet.sender_info->sender_packet_count == 100);
    REQUIRE(packet.sender_info->sender_octet_count == 200);
}

TEST_CASE("RtcpParser parses a Receiver Report with one report block", "[rtcp][parser]") {
    std::vector<std::uint8_t> bytes;
    bytes.push_back(0x81); // V=2, RC=1
    bytes.push_back(201);
    append_be16(bytes, 7); // 32 bytes / 4 - 1
    append_be32(bytes, 0x01020304); // receiver SSRC
    append_be32(bytes, 0x0A0B0C0D); // report block SSRC
    bytes.push_back(7); // fraction lost
    append_be24(bytes, 0x000102); // cumulative lost
    append_be32(bytes, 0x11121314);
    append_be32(bytes, 0x21222324);
    append_be32(bytes, 0x31323334);
    append_be32(bytes, 0x41424344);

    const auto packet = rtsi::RtcpParser::parse_single_packet(bytes);

    REQUIRE(packet.packet_type == rtsi::RtcpPacketType::ReceiverReport);
    REQUIRE(packet.receiver_ssrc.has_value());
    REQUIRE(packet.receiver_ssrc.value() == 0x01020304);
    REQUIRE(packet.report_blocks.size() == 1);
    REQUIRE(packet.report_blocks[0].ssrc == 0x0A0B0C0D);
    REQUIRE(packet.report_blocks[0].fraction_lost == 7);
    REQUIRE(packet.report_blocks[0].cumulative_lost == 0x000102);
    REQUIRE(packet.report_blocks[0].interarrival_jitter == 0x21222324);
}

TEST_CASE("RtcpParser parses a compound Sender Report and SDES packet", "[rtcp][parser]") {
    auto bytes = minimal_sender_report();

    bytes.push_back(0x81); // V=2, SC=1
    bytes.push_back(202);  // SDES
    append_be16(bytes, 2); // 12 bytes / 4 - 1
    append_be32(bytes, 0x01020304);
    bytes.push_back(0); // END
    bytes.push_back(0); // padding
    bytes.push_back(0); // padding
    bytes.push_back(0); // padding

    const auto packets = rtsi::RtcpParser::parse_compound_packet(bytes);

    REQUIRE(packets.size() == 2);
    REQUIRE(packets[0].is_sender_report());
    REQUIRE(packets[1].is_sdes());
    REQUIRE(packets[1].sdes_ssrcs.size() == 1);
    REQUIRE(packets[1].sdes_ssrcs[0] == 0x01020304);
}

TEST_CASE("RtcpParser parses BYE packets", "[rtcp][parser]") {
    std::vector<std::uint8_t> bytes;
    bytes.push_back(0x81); // V=2, SC=1
    bytes.push_back(203);
    append_be16(bytes, 1); // 8 bytes / 4 - 1
    append_be32(bytes, 0x01020304);

    const auto packet = rtsi::RtcpParser::parse_single_packet(bytes);

    REQUIRE(packet.is_bye());
    REQUIRE(packet.bye_ssrcs.size() == 1);
    REQUIRE(packet.bye_ssrcs[0] == 0x01020304);
}

TEST_CASE("RtcpParser rejects malformed packets deterministically", "[rtcp][parser]") {
    REQUIRE_THROWS(rtsi::RtcpParser::parse_single_packet({0x80, 200, 0}));
    REQUIRE_THROWS(rtsi::RtcpParser::parse_single_packet({0x40, 200, 0, 0}));
    REQUIRE_THROWS(rtsi::RtcpParser::parse_single_packet({0x80, 200, 0, 6}));
}
