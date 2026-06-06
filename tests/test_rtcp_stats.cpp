#include "rtsi/rtcp/RtcpStats.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("RtcpStats aggregates packet counters and latest report fields", "[rtcp][stats]") {
    rtsi::RtcpStats stats;
    stats.observe_frame();

    rtsi::RtcpPacket sr;
    sr.packet_type = rtsi::RtcpPacketType::SenderReport;
    sr.sender_info = rtsi::RtcpSenderInfo{
        0x01020304,
        0x1111111122222222ULL,
        0x33333333,
        100,
        200,
    };
    stats.observe_packet(sr);

    rtsi::RtcpPacket rr;
    rr.packet_type = rtsi::RtcpPacketType::ReceiverReport;
    rr.report_blocks.push_back(rtsi::RtcpReportBlock{
        0x0A0B0C0D,
        5,
        9,
        1000,
        1234,
        5678,
        90,
    });
    stats.observe_packet(rr);

    rtsi::RtcpPacket sdes;
    sdes.packet_type = rtsi::RtcpPacketType::SourceDescription;
    stats.observe_packet(sdes);

    rtsi::RtcpPacket bye;
    bye.packet_type = rtsi::RtcpPacketType::Bye;
    stats.observe_packet(bye);

    stats.observe_malformed_packet();

    const auto snapshot = stats.snapshot();
    REQUIRE(snapshot.frames_received == 1);
    REQUIRE(snapshot.packets_parsed == 4);
    REQUIRE(snapshot.malformed_packets == 1);
    REQUIRE(snapshot.sender_reports == 1);
    REQUIRE(snapshot.receiver_reports == 1);
    REQUIRE(snapshot.source_description_packets == 1);
    REQUIRE(snapshot.bye_packets == 1);
    REQUIRE(snapshot.last_sender_ssrc.value() == 0x01020304);
    REQUIRE(snapshot.last_rtp_timestamp_from_sr.value() == 0x33333333);
    REQUIRE(snapshot.last_sender_packet_count.value() == 100);
    REQUIRE(snapshot.last_sender_octet_count.value() == 200);
    REQUIRE(snapshot.last_report_block_jitter.value() == 1234);
    REQUIRE(snapshot.last_fraction_lost.value() == 5);
    REQUIRE(snapshot.last_cumulative_lost.value() == 9);
}
