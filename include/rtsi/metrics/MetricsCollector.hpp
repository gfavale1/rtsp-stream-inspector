#pragma once

#include "rtsi/h264/H264Analyzer.hpp"
#include "rtsi/h264/NalUnit.hpp"
#include "rtsi/metrics/StreamMetrics.hpp"
#include "rtsi/rtp/RtpPacket.hpp"
#include "rtsi/rtp/RtpStats.hpp"

#include <chrono>
#include <cstddef>

namespace rtsi {

struct MetricsCollectorSnapshot {
  RtpStatsSnapshot rtp;
  H264AnalysisSnapshot h264;
  StreamMetricsSnapshot stream;
};

class MetricsCollector {
public:
  void start_capture() noexcept;
  void stop_capture() noexcept;

  void update_rtp_packet(const RtpPacket& packet,
                         std::size_t rtp_packet_size);

  void update_h264_nal(const H264NalUnitInfo& nal_info);

  [[nodiscard]] MetricsCollectorSnapshot snapshot() const noexcept;

  [[nodiscard]] double capture_seconds() const noexcept;

private:
  using Clock = std::chrono::steady_clock;

  RtpStats rtp_stats_;
  H264Analyzer h264_analyzer_;

  Clock::time_point capture_start_{};
  Clock::time_point capture_end_{};

  bool capture_started_ = false;
  bool capture_stopped_ = false;
};

} // namespace rtsi
