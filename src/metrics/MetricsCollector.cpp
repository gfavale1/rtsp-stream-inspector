#include "rtsi/metrics/MetricsCollector.hpp"

namespace rtsi {

void MetricsCollector::start_capture() noexcept {
  capture_start_ = Clock::now();
  capture_end_ = {};
  capture_started_ = true;
  capture_stopped_ = false;
}

void MetricsCollector::stop_capture() noexcept {
  if (!capture_started_) {
    return;
  }

  capture_end_ = Clock::now();
  capture_stopped_ = true;
}

void MetricsCollector::update_rtp_packet(const RtpPacket& packet,
                                         std::size_t rtp_packet_size) {
  rtp_stats_.update(packet, rtp_packet_size);
}

void MetricsCollector::update_h264_nal(const H264NalUnitInfo& nal_info) {
  h264_analyzer_.update(nal_info);
}

MetricsCollectorSnapshot MetricsCollector::snapshot() const noexcept {
  MetricsCollectorSnapshot result;

  result.rtp = rtp_stats_.snapshot();
  result.h264 = h264_analyzer_.snapshot();
  result.stream =
      StreamMetrics::from_rtp_stats(result.rtp, capture_seconds());

  return result;
}

double MetricsCollector::capture_seconds() const noexcept {
  if (!capture_started_) {
    return 0.0;
  }

  const auto end = capture_stopped_ ? capture_end_ : Clock::now();
  return std::chrono::duration<double>(end - capture_start_).count();
}

} // namespace rtsi
