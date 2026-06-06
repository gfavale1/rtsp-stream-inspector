#include "rtsi/metrics/AnomalyDetector.hpp"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <string>
#include <vector>

namespace {

const rtsi::ReportFinding* find_by_code(
    const std::vector<rtsi::ReportFinding>& findings,
    const std::string& code) {
  const auto it = std::find_if(
      findings.begin(), findings.end(),
      [&](const rtsi::ReportFinding& finding) {
        return finding.code == code;
      });

  if (it == findings.end()) {
    return nullptr;
  }

  return &(*it);
}

rtsi::AnalysisReport healthy_report() {
  rtsi::AnalysisReport report;
  report.rtp.packets_lost = 0;
  report.rtp.out_of_order_packets = 0;
  report.h264.unknown_count = 0;
  report.h264.sps_count = 1;
  report.h264.pps_count = 1;
  report.h264.fu_a_start_count = 2;
  report.h264.fu_a_end_count = 2;
  report.rtp_quality.packets_observed = 3;
  report.rtp_quality.jitter_ms = 10.0;
  report.rtp_quality.max_interarrival_gap_ms = 40.0;
    report.rtcp.frames_received = 2;
    report.rtcp.packets_parsed = 2;
    report.rtcp.sender_reports = 1;
    report.rtcp.malformed_packets = 0;
  report.teardown_success = true;
  return report;
}

} // namespace

TEST_CASE("AnomalyDetector reports OK findings for a healthy capture", "[metrics][anomaly]") {
  const rtsi::AnomalyDetector detector;
  const auto findings = detector.analyze(healthy_report());

  REQUIRE(find_by_code(findings, "no_packet_loss") != nullptr);
  REQUIRE(find_by_code(findings, "no_packet_loss")->severity == "ok");

  REQUIRE(find_by_code(findings, "no_out_of_order_packets") != nullptr);
  REQUIRE(find_by_code(findings, "no_out_of_order_packets")->severity == "ok");

  REQUIRE(find_by_code(findings, "no_unknown_h264_nal_units") != nullptr);
  REQUIRE(find_by_code(findings, "no_unknown_h264_nal_units")->severity == "ok");

  REQUIRE(find_by_code(findings, "h264_parameter_sets_observed") != nullptr);
  REQUIRE(find_by_code(findings, "h264_parameter_sets_observed")->severity == "ok");

  REQUIRE(find_by_code(findings, "h264_fragmentation_balanced") != nullptr);
  REQUIRE(find_by_code(findings, "h264_fragmentation_balanced")->severity == "ok");

  REQUIRE(find_by_code(findings, "rtp_jitter_within_basic_threshold") != nullptr);
  REQUIRE(find_by_code(findings, "rtp_jitter_within_basic_threshold")->severity == "ok");

  REQUIRE(find_by_code(findings, "no_large_interarrival_gap") != nullptr);
  REQUIRE(find_by_code(findings, "no_large_interarrival_gap")->severity == "ok");

  REQUIRE(find_by_code(findings, "rtcp_observed") != nullptr);
    REQUIRE(find_by_code(findings, "rtcp_observed")->severity == "ok");
    REQUIRE(find_by_code(findings, "rtcp_sender_report_observed") != nullptr);
    REQUIRE(find_by_code(findings, "rtcp_sender_report_observed")->severity == "ok");
    REQUIRE(find_by_code(findings, "no_malformed_rtcp_packets") != nullptr);
    REQUIRE(find_by_code(findings, "no_malformed_rtcp_packets")->severity == "ok");
    REQUIRE(find_by_code(findings, "rtsp_teardown_success") != nullptr);
  REQUIRE(find_by_code(findings, "rtsp_teardown_success")->severity == "ok");

  REQUIRE(find_by_code(findings, "unencrypted_rtsp") != nullptr);
  REQUIRE(find_by_code(findings, "unencrypted_rtsp")->severity == "warning");
}

TEST_CASE("AnomalyDetector reports warnings for problematic captures", "[metrics][anomaly]") {
  auto report = healthy_report();
  report.rtp.packets_lost = 3;
  report.rtp.out_of_order_packets = 2;
  report.h264.unknown_count = 1;
  report.h264.sps_count = 0;
  report.h264.pps_count = 1;
  report.h264.fu_a_start_count = 3;
  report.h264.fu_a_end_count = 1;
  report.rtp_quality.packets_observed = 3;
  report.rtp_quality.jitter_ms = 100.0;
  report.rtp_quality.max_interarrival_gap_ms = 800.0;
  report.teardown_success = false;

  const rtsi::AnomalyDetector detector;
  const auto findings = detector.analyze(report);

  REQUIRE(find_by_code(findings, "packet_loss_detected") != nullptr);
  REQUIRE(find_by_code(findings, "packet_loss_detected")->severity == "warning");

  REQUIRE(find_by_code(findings, "out_of_order_packets_detected") != nullptr);
  REQUIRE(find_by_code(findings, "out_of_order_packets_detected")->severity == "warning");

  REQUIRE(find_by_code(findings, "unknown_h264_nal_units") != nullptr);
  REQUIRE(find_by_code(findings, "unknown_h264_nal_units")->severity == "warning");

  REQUIRE(find_by_code(findings, "missing_h264_parameter_sets") != nullptr);
  REQUIRE(find_by_code(findings, "missing_h264_parameter_sets")->severity == "warning");

  REQUIRE(find_by_code(findings, "incomplete_h264_fragmentation") != nullptr);
  REQUIRE(find_by_code(findings, "incomplete_h264_fragmentation")->severity == "warning");

  REQUIRE(find_by_code(findings, "high_rtp_jitter") != nullptr);
  REQUIRE(find_by_code(findings, "high_rtp_jitter")->severity == "warning");

  REQUIRE(find_by_code(findings, "large_interarrival_gap") != nullptr);
  REQUIRE(find_by_code(findings, "large_interarrival_gap")->severity == "warning");

  REQUIRE(find_by_code(findings, "rtsp_teardown_failed") != nullptr);
  REQUIRE(find_by_code(findings, "rtsp_teardown_failed")->severity == "warning");
}


TEST_CASE("AnomalyDetector reports RTCP warnings", "[metrics][anomaly][rtcp]") {
    auto report = healthy_report();
    report.rtcp.frames_received = 3;
    report.rtcp.packets_parsed = 2;
    report.rtcp.sender_reports = 0;
    report.rtcp.malformed_packets = 1;

    const rtsi::AnomalyDetector detector;
    const auto findings = detector.analyze(report);

    REQUIRE(find_by_code(findings, "no_rtcp_sender_report") != nullptr);
    REQUIRE(find_by_code(findings, "no_rtcp_sender_report")->severity == "warning");
    REQUIRE(find_by_code(findings, "malformed_rtcp_packets") != nullptr);
    REQUIRE(find_by_code(findings, "malformed_rtcp_packets")->severity == "warning");
}
