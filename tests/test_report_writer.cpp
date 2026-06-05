#include "rtsi/report/JsonReportWriter.hpp"
#include "rtsi/report/MarkdownReportWriter.hpp"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace {

std::string read_file(const std::filesystem::path& path) {
  std::ifstream file(path);
  std::ostringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

rtsi::AnalysisReport sample_report() {
  rtsi::AnalysisReport report;

  report.source.host = "192.0.2.10";
  report.source.port = 554;
  report.source.path = "/stream2";
  report.source.transport = "rtp_interleaved_tcp";

  report.video.codec = "H264";
  report.video.payload_type = 96;
  report.video.clock_rate = 90000;
  report.video.control = "track1";

  report.interleaved.rtp_frames_received = 10;
  report.interleaved.rtcp_frames_received = 2;
  report.interleaved.total_interleaved_payload_bytes = 1234;

  report.rtp.packets_received = 10;
  report.rtp.packets_lost = 0;
  report.rtp.out_of_order_packets = 0;
  report.rtp.total_rtp_bytes = 1000;
  report.rtp.total_payload_bytes = 800;
  report.rtp.first_sequence_number = 100;
  report.rtp.last_sequence_number = 109;
  report.rtp.last_timestamp = 9000;
  report.rtp.payload_type = 96;
  report.rtp.ssrc = 0x01020304;

  report.h264.nal_units_seen = 10;
  report.h264.sps_count = 1;
  report.h264.pps_count = 1;
  report.h264.idr_count = 2;
  report.h264.non_idr_count = 6;
  report.h264.fu_a_count = 2;
  report.h264.fu_a_start_count = 1;
  report.h264.fu_a_end_count = 1;

  report.stream.capture_seconds = 2.0;
  report.stream.rtp_bitrate_mbps = 0.004;
  report.stream.h264_payload_bitrate_mbps = 0.0032;
  report.stream.rtp_packets_per_second = 5.0;
  report.stream.average_rtp_packet_size = 100.0;
  report.stream.average_h264_payload_size = 80.0;

  report.rtp_quality.packets_observed = 10;
  report.rtp_quality.jitter_timestamp_units = 90.0;
  report.rtp_quality.jitter_seconds = 0.001;
  report.rtp_quality.jitter_ms = 1.0;
  report.rtp_quality.average_interarrival_gap_ms = 33.333;
  report.rtp_quality.max_interarrival_gap_ms = 40.0;

  report.teardown_success = true;
  report.findings.push_back({"ok", "no_packet_loss", "No RTP packet loss detected."});
  report.findings.push_back({"warning", "unencrypted_rtsp", "RTSP traffic is not encrypted."});

  return report;
}

} // namespace

TEST_CASE("JsonReportWriter writes parseable analysis reports", "[report][json]") {
  const auto path = std::filesystem::temp_directory_path() /
                    "rtsp_stream_inspector_report_test.json";

  const auto report = sample_report();
  rtsi::JsonReportWriter writer;
  writer.write_report(report, path.string());

  const auto content = read_file(path);
  const auto parsed = nlohmann::json::parse(content);

  REQUIRE(parsed.contains("source"));
  REQUIRE(parsed.contains("video"));
  REQUIRE(parsed.contains("interleaved"));
  REQUIRE(parsed.contains("rtp"));
  REQUIRE(parsed.contains("h264"));
  REQUIRE(parsed.contains("stream_metrics"));
  REQUIRE(parsed.contains("rtp_quality"));
  REQUIRE(parsed.contains("findings"));
  REQUIRE(parsed["source"]["host"] == "192.0.2.10");
  REQUIRE(parsed["video"]["codec"] == "H264");
  REQUIRE(parsed["rtp"]["packets_received"] == 10);
  REQUIRE(parsed["rtp_quality"]["packets_observed"] == 10);
  REQUIRE(parsed["rtp_quality"]["jitter_ms"] == 1.0);
  REQUIRE(parsed["findings"].is_array());
  REQUIRE(parsed["findings"].size() == 2);

  REQUIRE(content.find("Authorization") == std::string::npos);
  REQUIRE(content.find("camera1") == std::string::npos);
  REQUIRE(content.find("CameraTest2026") == std::string::npos);
}

TEST_CASE("MarkdownReportWriter writes human-readable analysis reports", "[report][markdown]") {
  const auto path = std::filesystem::temp_directory_path() /
                    "rtsp_stream_inspector_report_test.md";

  const auto report = sample_report();
  rtsi::MarkdownReportWriter writer;
  writer.write_report(report, path.string());

  const auto content = read_file(path);

  REQUIRE(content.find("# RTSP Stream Inspection Report") != std::string::npos);
  REQUIRE(content.find("## Source") != std::string::npos);
  REQUIRE(content.find("## Video Track") != std::string::npos);
  REQUIRE(content.find("## RTP Statistics") != std::string::npos);
  REQUIRE(content.find("## H.264 NAL Statistics") != std::string::npos);
  REQUIRE(content.find("## Stream Metrics") != std::string::npos);
  REQUIRE(content.find("## RTP Quality Metrics") != std::string::npos);
  REQUIRE(content.find("Packets observed for jitter: 10") != std::string::npos);
  REQUIRE(content.find("## Findings") != std::string::npos);
  REQUIRE(content.find("[OK] No RTP packet loss detected.") != std::string::npos);
  REQUIRE(content.find("[WARN] RTSP traffic is not encrypted.") != std::string::npos);

  REQUIRE(content.find("Authorization") == std::string::npos);
  REQUIRE(content.find("camera1") == std::string::npos);
  REQUIRE(content.find("CameraTest2026") == std::string::npos);
}
