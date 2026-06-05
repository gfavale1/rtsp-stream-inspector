#include "rtsi/metrics/StreamMetrics.hpp"

namespace rtsi {
namespace {

double bitrate_mbps(std::size_t byte_count, double seconds) noexcept {
  if (seconds <= 0.0) {
    return 0.0;
  }

  return (static_cast<double>(byte_count) * 8.0) /
         (seconds * 1000000.0);
}

double packets_per_second(std::size_t packet_count, double seconds) noexcept {
  if (seconds <= 0.0) {
    return 0.0;
  }

  return static_cast<double>(packet_count) / seconds;
}

double average_size(std::size_t byte_count, std::size_t packet_count) noexcept {
  if (packet_count == 0) {
    return 0.0;
  }

  return static_cast<double>(byte_count) /
         static_cast<double>(packet_count);
}

} // namespace

StreamMetricsSnapshot StreamMetrics::from_rtp_stats(
    const RtpStatsSnapshot& rtp_stats,
    double capture_seconds) noexcept {
  StreamMetricsSnapshot snapshot;

  snapshot.capture_seconds = capture_seconds > 0.0 ? capture_seconds : 0.0;

  snapshot.rtp_bitrate_mbps =
      bitrate_mbps(rtp_stats.total_rtp_bytes, capture_seconds);

  snapshot.h264_payload_bitrate_mbps =
      bitrate_mbps(rtp_stats.total_payload_bytes, capture_seconds);

  snapshot.rtp_packets_per_second =
      packets_per_second(rtp_stats.packets_received, capture_seconds);

  snapshot.average_rtp_packet_size =
      average_size(rtp_stats.total_rtp_bytes, rtp_stats.packets_received);

  snapshot.average_h264_payload_size =
      average_size(rtp_stats.total_payload_bytes, rtp_stats.packets_received);

  return snapshot;
}

} // namespace rtsi
