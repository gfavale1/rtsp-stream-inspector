#include "rtsi/app/StreamAnalyzer.hpp"

#include "rtsi/h264/NalUnit.hpp"
#include "rtsi/rtp/JitterEstimator.hpp"
#include "rtsi/rtp/RtpParser.hpp"
#include "rtsi/rtcp/RtcpParser.hpp"
#include "rtsi/rtcp/RtcpStats.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <exception>

namespace rtsi {
namespace {

InterleavedFrameType classify_frame(const InterleavedFrame& frame) noexcept {
  if (frame.is_rtp()) {
    return InterleavedFrameType::Rtp;
  }

  if (frame.is_rtcp()) {
    return InterleavedFrameType::Rtcp;
  }

  return InterleavedFrameType::Unknown;
}

std::uint32_t sanitize_clock_rate(int clock_rate) noexcept {
  if (clock_rate <= 0) {
    return 90000;
  }

  return static_cast<std::uint32_t>(clock_rate);
}

RtpQualityReport make_rtp_quality_report(
    const JitterSnapshot& snapshot) noexcept {
  RtpQualityReport report;
  report.packets_observed = snapshot.packets_observed;
  report.jitter_timestamp_units = snapshot.jitter_timestamp_units;
  report.jitter_seconds = snapshot.jitter_seconds;
  report.jitter_ms = snapshot.jitter_ms;
  report.average_interarrival_gap_ms =
      snapshot.average_interarrival_gap_ms;
  report.max_interarrival_gap_ms = snapshot.max_interarrival_gap_ms;
  return report;
}

} // namespace

StreamAnalyzerResult StreamAnalyzer::analyze(
    InterleavedFrameReader& reader,
    const StreamAnalyzerConfig& config,
    PacketLogCallback on_packet_log) const {
  StreamAnalyzerResult result;

  const int frame_count = std::max(0, config.frame_count);
  const int packet_log_limit = std::max(0, config.packet_log_limit);

  MetricsCollector metrics_collector;
  metrics_collector.start_capture();

  JitterEstimator jitter_estimator(sanitize_clock_rate(config.rtp_clock_rate));
    RtcpStats rtcp_stats;

  for (int i = 0; i < frame_count; ++i) {
    try {
      const auto frame = reader.read_frame();
      const auto arrival_time = std::chrono::steady_clock::now();
      ++result.interleaved_frames_received;

      const auto frame_type = classify_frame(frame);
      const bool should_log_packet = i < packet_log_limit;

      result.total_payload_bytes += frame.payload.size();

      if (frame_type == InterleavedFrameType::Rtp) {
        ++result.rtp_frames_received;

        const auto rtp_packet = RtpParser::parse(frame.payload);
        metrics_collector.update_rtp_packet(rtp_packet, frame.payload.size());
        jitter_estimator.update(rtp_packet.timestamp, arrival_time);

        const auto nal_info =
            NalUnitParser::parse_rtp_payload(rtp_packet.payload);
        metrics_collector.update_h264_nal(nal_info);
      } else if (frame_type == InterleavedFrameType::Rtcp) {
                ++result.rtcp_frames_received;
                rtcp_stats.observe_frame();
                try {
                    const auto packets = RtcpParser::parse_compound_packet(frame.payload);
                    for (const auto& packet : packets) {
                        rtcp_stats.observe_packet(packet);
                    }
                } catch (const std::exception&) {
                    rtcp_stats.observe_malformed_packet();
                }
            }

      if (should_log_packet && on_packet_log) {
        StreamAnalyzerPacketLogEntry entry;
        entry.frame_index = static_cast<std::size_t>(i) + 1;
        entry.channel = frame.channel;
        entry.payload_size = frame.payload.size();
        entry.type = frame_type;

        on_packet_log(entry);
      }

    } catch (const std::exception& ex) {
      result.stop_reason = ex.what();
      break;
    }
  }

  metrics_collector.stop_capture();
  result.metrics = metrics_collector.snapshot();

  result.report.interleaved.rtp_frames_received = result.rtp_frames_received;
  result.report.interleaved.rtcp_frames_received = result.rtcp_frames_received;
  result.report.interleaved.total_interleaved_payload_bytes =
      result.total_payload_bytes;
  result.report.rtp = result.metrics.rtp;
    result.report.rtcp = rtcp_stats.snapshot();
  result.report.h264 = result.metrics.h264;
  result.report.stream = result.metrics.stream;
  result.report.rtp_quality =
      make_rtp_quality_report(jitter_estimator.snapshot());

  return result;
}

} // namespace rtsi
