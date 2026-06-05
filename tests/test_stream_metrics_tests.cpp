#include "rtsi/metrics/StreamMetrics.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("StreamMetrics computes rates and averages from RTP stats", "[metrics][stream]") {
  rtsi::RtpStatsSnapshot rtp;
  rtp.packets_received = 10;
  rtp.total_rtp_bytes = 1000;
  rtp.total_payload_bytes = 800;

  const auto metrics = rtsi::StreamMetrics::from_rtp_stats(rtp, 2.0);

  REQUIRE(metrics.capture_seconds == Catch::Approx(2.0));
  REQUIRE(metrics.rtp_bitrate_mbps == Catch::Approx(0.004));
  REQUIRE(metrics.h264_payload_bitrate_mbps == Catch::Approx(0.0032));
  REQUIRE(metrics.rtp_packets_per_second == Catch::Approx(5.0));
  REQUIRE(metrics.average_rtp_packet_size == Catch::Approx(100.0));
  REQUIRE(metrics.average_h264_payload_size == Catch::Approx(80.0));
}

TEST_CASE("StreamMetrics handles zero duration and zero packets", "[metrics][stream]") {
  rtsi::RtpStatsSnapshot rtp;

  const auto metrics = rtsi::StreamMetrics::from_rtp_stats(rtp, 0.0);

  REQUIRE(metrics.capture_seconds == Catch::Approx(0.0));
  REQUIRE(metrics.rtp_bitrate_mbps == Catch::Approx(0.0));
  REQUIRE(metrics.h264_payload_bitrate_mbps == Catch::Approx(0.0));
  REQUIRE(metrics.rtp_packets_per_second == Catch::Approx(0.0));
  REQUIRE(metrics.average_rtp_packet_size == Catch::Approx(0.0));
  REQUIRE(metrics.average_h264_payload_size == Catch::Approx(0.0));
}
