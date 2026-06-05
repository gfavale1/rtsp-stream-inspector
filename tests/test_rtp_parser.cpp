#include "rtsi/rtp/RtpParser.hpp"

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <stdexcept>
#include <vector>

TEST_CASE("RtpParser parses a minimal RTP packet", "[rtp][parser]") {
  const std::vector<std::uint8_t> packet = {
      0x80,       // V=2, P=0, X=0, CC=0
      0xE0,       // M=1, PT=96
      0x12, 0x34, // sequence number
      0x01, 0x02, 0x03, 0x04, // timestamp
      0x0A, 0x0B, 0x0C, 0x0D, // SSRC
      0x65, 0x88, 0x99        // payload
  };

  const auto parsed = rtsi::RtpParser::parse(packet);

  REQUIRE(parsed.version == 2);
  REQUIRE_FALSE(parsed.padding);
  REQUIRE_FALSE(parsed.extension);
  REQUIRE(parsed.csrc_count == 0);
  REQUIRE(parsed.marker);
  REQUIRE(parsed.payload_type == 96);
  REQUIRE(parsed.sequence_number == 0x1234);
  REQUIRE(parsed.timestamp == 0x01020304);
  REQUIRE(parsed.ssrc == 0x0A0B0C0D);
  REQUIRE(parsed.header_size == 12);
  REQUIRE(parsed.payload.size() == 3);
  REQUIRE(parsed.payload[0] == 0x65);
}

TEST_CASE("RtpParser rejects packets shorter than the RTP header", "[rtp][parser]") {
  const std::vector<std::uint8_t> too_short = {0x80, 0x60, 0x00};

  REQUIRE_THROWS_AS(rtsi::RtpParser::parse(too_short), std::invalid_argument);
}
