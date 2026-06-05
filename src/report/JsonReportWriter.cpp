#include "rtsi/report/JsonReportWriter.hpp"

#include <nlohmann/json.hpp>

#include <fstream>
#include <optional>
#include <stdexcept>

namespace rtsi {
namespace {

template <typename T>
void assign_optional(nlohmann::json& object, const char* key,
                     const std::optional<T>& value) {
  if (value.has_value()) {
    object[key] = value.value();
  } else {
    object[key] = nullptr;
  }
}

} // namespace

void JsonReportWriter::write_report(const AnalysisReport& report,
                                    const std::string& output_path) const {
  nlohmann::json json_report;

  json_report["source"] = {
      {"host", report.source.host},
      {"port", report.source.port},
      {"path", report.source.path},
      {"transport", report.source.transport},
  };

  json_report["video"] = {
      {"codec", report.video.codec},
      {"payload_type", report.video.payload_type},
      {"clock_rate", report.video.clock_rate},
      {"control", report.video.control},
  };

  json_report["interleaved"] = {
      {"rtp_frames_received", report.interleaved.rtp_frames_received},
      {"rtcp_frames_received", report.interleaved.rtcp_frames_received},
      {"total_payload_bytes",
       report.interleaved.total_interleaved_payload_bytes},
  };

  nlohmann::json rtp_json;
  rtp_json["packets_received"] = report.rtp.packets_received;
  rtp_json["packets_lost"] = report.rtp.packets_lost;
  rtp_json["out_of_order_packets"] = report.rtp.out_of_order_packets;
  rtp_json["loss_rate"] = report.rtp.loss_rate();
  rtp_json["total_rtp_bytes"] = report.rtp.total_rtp_bytes;
  rtp_json["total_payload_bytes"] = report.rtp.total_payload_bytes;
  assign_optional(rtp_json, "first_sequence_number",
                  report.rtp.first_sequence_number);
  assign_optional(rtp_json, "last_sequence_number",
                  report.rtp.last_sequence_number);
  assign_optional(rtp_json, "last_timestamp", report.rtp.last_timestamp);
  if (report.rtp.payload_type.has_value()) {
    rtp_json["payload_type"] =
        static_cast<int>(report.rtp.payload_type.value());
  } else {
    rtp_json["payload_type"] = nullptr;
  }
  assign_optional(rtp_json, "ssrc", report.rtp.ssrc);
  json_report["rtp"] = rtp_json;

  json_report["h264"] = {
      {"nal_units_seen", report.h264.nal_units_seen},
      {"sps", report.h264.sps_count},
      {"pps", report.h264.pps_count},
      {"sei", report.h264.sei_count},
      {"idr_slices", report.h264.idr_count},
      {"non_idr_slices", report.h264.non_idr_count},
      {"stap_a_packets", report.h264.stap_a_count},
      {"fu_a_packets", report.h264.fu_a_count},
      {"fu_a_starts", report.h264.fu_a_start_count},
      {"fu_a_ends", report.h264.fu_a_end_count},
      {"unknown_nal_units", report.h264.unknown_count},
  };

  json_report["stream_metrics"] = {
      {"capture_duration_seconds", report.stream.capture_seconds},
      {"rtp_bitrate_mbps", report.stream.rtp_bitrate_mbps},
      {"h264_payload_bitrate_mbps",
       report.stream.h264_payload_bitrate_mbps},
      {"rtp_packets_per_second", report.stream.rtp_packets_per_second},
      {"average_rtp_packet_size", report.stream.average_rtp_packet_size},
      {"average_h264_payload_size",
       report.stream.average_h264_payload_size},
  };

  json_report["teardown_success"] = report.teardown_success;

  std::ofstream file(output_path);
  if (!file) {
    throw std::runtime_error("Unable to open output file: " + output_path);
  }

  file << json_report.dump(2) << '\n';
}

void JsonReportWriter::write_dummy_report(const AnalyzerConfig& config) const {
  nlohmann::json report;
  report["project"] = "rtsp-stream-inspector";
  report["version"] = "0.1.0";
  report["input"] = {
      {"url", config.url},
      {"duration_seconds", config.duration_seconds},
  };
  report["status"] = "dummy_report";
  report["message"] = "Initial project skeleton is working. RTSP/RTP analysis is not implemented yet.";
  report["metrics"] = {
      {"packets_received", 0},
      {"packets_lost", 0},
      {"loss_rate", 0.0},
      {"average_jitter_ms", 0.0},
      {"bitrate_kbps", 0.0},
  };

  std::ofstream file(config.output_path);
  if (!file) {
    throw std::runtime_error("Unable to open output file: " + config.output_path);
  }

  file << report.dump(2) << '\n';
}

} // namespace rtsi
