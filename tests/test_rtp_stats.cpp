#include "rtsi/rtp/RtpStats.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <cstdint>

namespace {

rtsi::RtpPacket make_packet(std::uint16_t sequence_number,
                            std::uint32_t timestamp = 0,
                            std::size_t payload_size = 4) {
  rtsi::RtpPacket packet;
  packet.version = 2;
  packet.payload_type = 96;
  packet.sequence_number = sequence_number;
  packet.timestamp = timestamp;
  packet.ssrc = 0x01020304;
  packet.header_size = 12;
  packet.payload.assign(payload_size, 0xAA);
  return packet;
}

void update(rtsi::RtpStats& stats, std::uint16_t sequence_number) {
  const auto packet = make_packet(sequence_number, sequence_number);
  stats.update(packet, packet.header_size + packet.payload.size());
}

} // namespace

TEST_CASE("RtpStats handles consecutive sequence numbers", "[rtp][stats]") {
  rtsi::RtpStats stats;

  update(stats, 100);
  update(stats, 101);
  update(stats, 102);

  const auto snapshot = stats.snapshot();

  REQUIRE(snapshot.packets_received == 3);
  REQUIRE(snapshot.packets_lost == 0);
  REQUIRE(snapshot.out_of_order_packets == 0);
  REQUIRE(snapshot.loss_rate() == Catch::Approx(0.0));
  REQUIRE(snapshot.first_sequence_number.has_value());
  REQUIRE(snapshot.first_sequence_number.value() == 100);
  REQUIRE(snapshot.last_sequence_number.has_value());
  REQUIRE(snapshot.last_sequence_number.value() == 102);
}

TEST_CASE("RtpStats estimates missing packets", "[rtp][stats]") {
  rtsi::RtpStats stats;

  update(stats, 100);
  update(stats, 101);
  update(stats, 103);

  const auto snapshot = stats.snapshot();

  REQUIRE(snapshot.packets_received == 3);
  REQUIRE(snapshot.packets_lost == 1);
  REQUIRE(snapshot.loss_rate() == Catch::Approx(1.0 / 4.0));
}

TEST_CASE("RtpStats detects out-of-order packets", "[rtp][stats]") {
  rtsi::RtpStats stats;

  update(stats, 100);
  update(stats, 102);
  update(stats, 101);

  const auto snapshot = stats.snapshot();

  REQUIRE(snapshot.packets_received == 3);
  REQUIRE(snapshot.out_of_order_packets >= 1);
}

TEST_CASE("RtpStats handles 16-bit sequence number wrap-around", "[rtp][stats]") {
  rtsi::RtpStats stats;

  update(stats, 65534);
  update(stats, 65535);
  update(stats, 0);
  update(stats, 1);

  const auto snapshot = stats.snapshot();

  REQUIRE(snapshot.packets_received == 4);
  REQUIRE(snapshot.packets_lost == 0);
  REQUIRE(snapshot.out_of_order_packets == 0);
  REQUIRE(snapshot.last_sequence_number.has_value());
  REQUIRE(snapshot.last_sequence_number.value() == 1);
}
