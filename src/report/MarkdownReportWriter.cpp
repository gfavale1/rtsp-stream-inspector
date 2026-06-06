#include "rtsi/report/MarkdownReportWriter.hpp"

#include <cstdint>
#include <fstream>
#include <iomanip>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>

namespace rtsi {
namespace {

template <typename T>
std::string optional_to_string(const std::optional<T>& value) {
  if (!value.has_value()) {
    return "N/A";
  }

  std::ostringstream stream;
  stream << value.value();
  return stream.str();
}

std::string optional_payload_type_to_string(
    const std::optional<std::uint8_t>& value) {
  if (!value.has_value()) {
    return "N/A";
  }

  return std::to_string(static_cast<int>(value.value()));
}

std::string finding_label(const std::string& severity) {
  if (severity == "ok") {
    return "[OK]";
  }

  if (severity == "warning") {
    return "[WARN]";
  }

  if (severity == "critical") {
    return "[CRITICAL]";
  }

  return "[INFO]";
}

std::string yes_no(bool value) { return value ? "yes" : "no"; }

} // namespace

void MarkdownReportWriter::write_report(
    const AnalysisReport& report,
    const std::string& output_path) const {
  std::ofstream file(output_path);

  if (!file) {
    throw std::runtime_error("Unable to open output file: " + output_path);
  }

  file << std::fixed << std::setprecision(3);

  file << "# RTSP Stream Inspection Report\n\n";

  file << "## Metadata\n\n";
  file << "- Tool: " << report.metadata.tool << '\n';
  file << "- Version: " << report.metadata.version << '\n';
  file << "- Schema version: " << report.metadata.schema_version << '\n';
  file << "- Command: " << report.metadata.command << '\n';
  file << "- Generated at UTC: " << report.metadata.generated_at_utc << "\n\n";

  file << "## Configuration\n\n";
  file << "- Frames requested: " << report.configuration.frames_requested << '\n';
  file << "- Packet log limit: " << report.configuration.packet_log_limit << '\n';
  file << "- Timeout: " << report.configuration.timeout_ms << " ms\n";
  file << "- Transport: " << report.configuration.transport << "\n\n";

  file << "## Source\n\n";
  file << "- Host: " << report.source.host << '\n';
  file << "- Port: " << report.source.port << '\n';
  file << "- Path: " << report.source.path << '\n';
  file << "- Transport: " << report.source.transport << "\n\n";

  file << "## Video Track\n\n";
  file << "- Codec: " << report.video.codec << '\n';
  file << "- Payload type: " << report.video.payload_type << '\n';
  file << "- Clock rate: " << report.video.clock_rate << '\n';
  file << "- Control: " << report.video.control << "\n\n";

  file << "## Interleaved Capture\n\n";
  file << "- RTP frames received: "
       << report.interleaved.rtp_frames_received << '\n';
  file << "- RTCP frames received: "
       << report.interleaved.rtcp_frames_received << '\n';
  file << "- Total interleaved payload bytes: "
       << report.interleaved.total_interleaved_payload_bytes << "\n\n";

  file << "## RTP Statistics\n\n";
  file << "- Packets received: " << report.rtp.packets_received << '\n';
  file << "- Packets lost estimated: " << report.rtp.packets_lost << '\n';
  file << "- Out-of-order packets: "
       << report.rtp.out_of_order_packets << '\n';
  file << "- Loss rate: " << report.rtp.loss_rate() << '\n';
  file << "- Total RTP bytes: " << report.rtp.total_rtp_bytes << '\n';
  file << "- Total RTP payload bytes: "
       << report.rtp.total_payload_bytes << '\n';
  file << "- First sequence number: "
       << optional_to_string(report.rtp.first_sequence_number) << '\n';
  file << "- Last sequence number: "
       << optional_to_string(report.rtp.last_sequence_number) << '\n';
  file << "- Last RTP timestamp: "
       << optional_to_string(report.rtp.last_timestamp) << '\n';
  file << "- Payload type: "
       << optional_payload_type_to_string(report.rtp.payload_type) << '\n';
  file << "- SSRC: " << optional_to_string(report.rtp.ssrc) << "\n\n";

  
    file << "## RTCP Statistics\n\n";
    file << "- RTCP frames received: " << report.rtcp.frames_received << '\n';
    file << "- RTCP packets parsed: " << report.rtcp.packets_parsed << '\n';
    file << "- Malformed RTCP packets: " << report.rtcp.malformed_packets << '\n';
    file << "- Sender Reports: " << report.rtcp.sender_reports << '\n';
    file << "- Receiver Reports: " << report.rtcp.receiver_reports << '\n';
    file << "- SDES packets: " << report.rtcp.source_description_packets << '\n';
    file << "- BYE packets: " << report.rtcp.bye_packets << '\n';
    file << "- APP packets: " << report.rtcp.app_packets << '\n';
    file << "- Unknown RTCP packets: " << report.rtcp.unknown_packets << '\n';
    file << "- Last SR RTP timestamp: " << optional_to_string(report.rtcp.last_rtp_timestamp_from_sr) << '\n';
    file << "- Last sender packet count: " << optional_to_string(report.rtcp.last_sender_packet_count) << '\n';
    file << "- Last sender octet count: " << optional_to_string(report.rtcp.last_sender_octet_count) << "\n\n";

    file << "## H.264 NAL Statistics\n\n";
  file << "- NAL units seen: " << report.h264.nal_units_seen << '\n';
  file << "- SPS: " << report.h264.sps_count << '\n';
  file << "- PPS: " << report.h264.pps_count << '\n';
  file << "- SEI: " << report.h264.sei_count << '\n';
  file << "- IDR slices: " << report.h264.idr_count << '\n';
  file << "- Non-IDR slices: " << report.h264.non_idr_count << '\n';
  file << "- STAP-A packets: " << report.h264.stap_a_count << '\n';
  file << "- FU-A packets: " << report.h264.fu_a_count << '\n';
  file << "- FU-A starts: " << report.h264.fu_a_start_count << '\n';
  file << "- FU-A ends: " << report.h264.fu_a_end_count << '\n';
  file << "- Unknown NAL units: " << report.h264.unknown_count << "\n\n";

  file << "## Stream Metrics\n\n";
  file << "- Capture duration: " << report.stream.capture_seconds << " s\n";
  file << "- RTP bitrate: " << report.stream.rtp_bitrate_mbps << " Mbps\n";
  file << "- H264 payload bitrate: "
       << report.stream.h264_payload_bitrate_mbps << " Mbps\n";
  file << "- RTP packets/sec: "
       << report.stream.rtp_packets_per_second << " pps\n";
  file << "- Average RTP packet size: "
       << report.stream.average_rtp_packet_size << " bytes\n";
  file << "- Average H264 payload size: "
       << report.stream.average_h264_payload_size << " bytes\n\n";

  file << "## RTP Quality Metrics\n\n";
  file << "- Packets observed for jitter: "
       << report.rtp_quality.packets_observed << '\n';
  file << "- RTP jitter: " << report.rtp_quality.jitter_ms << " ms\n";
  file << "- Average inter-arrival gap: "
       << report.rtp_quality.average_interarrival_gap_ms << " ms\n";
  file << "- Max inter-arrival gap: "
       << report.rtp_quality.max_interarrival_gap_ms << " ms\n\n";

  file << "## Findings\n\n";

  if (report.findings.empty()) {
    file << "- [INFO] No findings were generated.\n\n";
  } else {
    for (const auto& finding : report.findings) {
      file << "- " << finding_label(finding.severity) << ' '
           << finding.message << '\n';
    }

    file << '\n';
  }

  file << "## RTSP Session Cleanup\n\n";
  file << "- TEARDOWN success: " << yes_no(report.teardown_success) << '\n';
}

} // namespace rtsi
