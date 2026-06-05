#include "rtsi/app/StreamAnalyzer.hpp"

#include "rtsi/h264/NalUnit.hpp"
#include "rtsi/rtp/RtpParser.hpp"

#include <algorithm>
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

  for (int i = 0; i < frame_count; ++i) {
    try {
      const auto frame = reader.read_frame();
      ++result.interleaved_frames_received;

      const auto frame_type = classify_frame(frame);
      const bool should_log_packet = i < packet_log_limit;

      result.total_payload_bytes += frame.payload.size();

      if (frame_type == InterleavedFrameType::Rtp) {
        ++result.rtp_frames_received;

        const auto rtp_packet = RtpParser::parse(frame.payload);
        metrics_collector.update_rtp_packet(rtp_packet, frame.payload.size());

        const auto nal_info =
            NalUnitParser::parse_rtp_payload(rtp_packet.payload);
        metrics_collector.update_h264_nal(nal_info);
      } else if (frame_type == InterleavedFrameType::Rtcp) {
        ++result.rtcp_frames_received;
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
  result.report.h264 = result.metrics.h264;
  result.report.stream = result.metrics.stream;

  return result;
}

} // namespace rtsi
