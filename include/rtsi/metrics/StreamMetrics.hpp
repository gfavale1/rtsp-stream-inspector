#pragma once

#include "rtsi/rtp/RtpStats.hpp"

namespace rtsi {

struct StreamMetricsSnapshot {
  double capture_seconds = 0.0;
  double rtp_bitrate_mbps = 0.0;
  double h264_payload_bitrate_mbps = 0.0;
  double rtp_packets_per_second = 0.0;
  double average_rtp_packet_size = 0.0;
  double average_h264_payload_size = 0.0;
};

class StreamMetrics {
public:
  [[nodiscard]] static StreamMetricsSnapshot
  from_rtp_stats(const RtpStatsSnapshot& rtp_stats,
                 double capture_seconds) noexcept;
};

} // namespace rtsi
