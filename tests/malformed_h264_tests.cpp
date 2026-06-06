#include "rtsi/h264/H264Analyzer.hpp"
#include "rtsi/h264/NalUnit.hpp"
#include "rtsi/metrics/AnomalyDetector.hpp"
#include "rtsi/report/AnalysisReport.hpp"

#include <catch2/catch_test_macros.hpp>

#include <stdexcept>
#include <string>
#include <vector>

namespace {

bool has_finding(const std::vector<rtsi::ReportFinding>& findings, const std::string& code) {
    for (const auto& finding : findings) {
        if (finding.code == code) {
            return true;
        }
    }
    return false;
}

} // namespace

TEST_CASE("NalUnitParser rejects empty H264 RTP payload", "[h264][malformed]") {
    REQUIRE_THROWS_AS(rtsi::NalUnitParser::parse_rtp_payload({}), std::invalid_argument);
}

TEST_CASE("NalUnitParser handles one-byte valid NAL payloads", "[h264][malformed]") {
    REQUIRE(rtsi::NalUnitParser::parse_rtp_payload({0x67}).type == rtsi::H264NalUnitType::Sps);
    REQUIRE(rtsi::NalUnitParser::parse_rtp_payload({0x68}).type == rtsi::H264NalUnitType::Pps);
    REQUIRE(rtsi::NalUnitParser::parse_rtp_payload({0x65}).type == rtsi::H264NalUnitType::IdrSlice);
    REQUIRE(rtsi::NalUnitParser::parse_rtp_payload({0x41}).type == rtsi::H264NalUnitType::NonIdrSlice);
}

TEST_CASE("NalUnitParser rejects truncated FU-A payload", "[h264][malformed]") {
    REQUIRE_THROWS_AS(rtsi::NalUnitParser::parse_rtp_payload({0x7C}), std::invalid_argument);
}

TEST_CASE("H264Analyzer records FU-A start without matching end", "[h264][malformed]") {
    rtsi::H264Analyzer analyzer;
    analyzer.update(rtsi::NalUnitParser::parse_rtp_payload({0x7C, 0x85}));

    const auto snapshot = analyzer.snapshot();
    REQUIRE(snapshot.fu_a_count == 1);
    REQUIRE(snapshot.fu_a_start_count == 1);
    REQUIRE(snapshot.fu_a_end_count == 0);
}

TEST_CASE("H264Analyzer records isolated FU-A end without crashing", "[h264][malformed]") {
    rtsi::H264Analyzer analyzer;
    analyzer.update(rtsi::NalUnitParser::parse_rtp_payload({0x7C, 0x45}));

    const auto snapshot = analyzer.snapshot();
    REQUIRE(snapshot.fu_a_count == 1);
    REQUIRE(snapshot.fu_a_start_count == 0);
    REQUIRE(snapshot.fu_a_end_count == 1);
}

TEST_CASE("NalUnitParser handles STAP-A header without enough aggregation data", "[h264][malformed]") {
    const auto stap_a_only = rtsi::NalUnitParser::parse_rtp_payload({0x78});
    REQUIRE(stap_a_only.type == rtsi::H264NalUnitType::StapA);

    const auto stap_a_oversized_child = rtsi::NalUnitParser::parse_rtp_payload({0x78, 0x7F, 0xFF, 0x65});
    REQUIRE(stap_a_oversized_child.type == rtsi::H264NalUnitType::StapA);
}

TEST_CASE("H264Analyzer counts unknown NAL type", "[h264][malformed]") {
    rtsi::H264Analyzer analyzer;
    analyzer.update(rtsi::NalUnitParser::parse_rtp_payload({0x1F}));

    const auto snapshot = analyzer.snapshot();
    REQUIRE(snapshot.nal_units_seen == 1);
    REQUIRE(snapshot.unknown_count == 1);
}

TEST_CASE("AnomalyDetector reports FU-A imbalance and missing H264 parameter sets", "[h264][anomaly][malformed]") {
    rtsi::AnalysisReport report;
    report.h264.fu_a_count = 1;
    report.h264.fu_a_start_count = 1;
    report.h264.fu_a_end_count = 0;
    report.rtp_quality.packets_observed = 2;
    report.teardown_success = true;

    const rtsi::AnomalyDetector detector;
    const auto findings = detector.analyze(report);

    REQUIRE(has_finding(findings, "incomplete_h264_fragmentation"));
    REQUIRE(has_finding(findings, "missing_h264_parameter_sets"));
}
