#include "rtsi/metrics/AnomalyDetector.hpp"

#include <string>
#include <utility>
#include <vector>

namespace rtsi {
namespace {

constexpr double kJitterWarningThresholdMs = 50.0;
constexpr double kLargeInterarrivalGapThresholdMs = 500.0;

ReportFinding make_finding(
    AnomalySeverity severity,
    std::string code,
    std::string message
) {
    ReportFinding finding;
    finding.severity = to_string(severity);
    finding.code = std::move(code);
    finding.message = std::move(message);
    return finding;
}

} // namespace

std::string to_string(AnomalySeverity severity) {
    switch (severity) {
    case AnomalySeverity::Ok:
        return "ok";
    case AnomalySeverity::Warning:
        return "warning";
    case AnomalySeverity::Critical:
        return "critical";
    }

    return "unknown";
}

std::vector<ReportFinding> AnomalyDetector::analyze(const AnalysisReport& report) const {
    std::vector<ReportFinding> findings;

    if (report.rtp.packets_lost == 0) {
        findings.push_back(make_finding(
            AnomalySeverity::Ok,
            "no_packet_loss",
            "No RTP packet loss detected."
        ));
    } else {
        findings.push_back(make_finding(
            AnomalySeverity::Warning,
            "packet_loss_detected",
            "Estimated RTP packet loss was detected."
        ));
    }

    if (report.rtp.out_of_order_packets == 0) {
        findings.push_back(make_finding(
            AnomalySeverity::Ok,
            "no_out_of_order_packets",
            "No out-of-order RTP packets detected."
        ));
    } else {
        findings.push_back(make_finding(
            AnomalySeverity::Warning,
            "out_of_order_packets_detected",
            "Out-of-order RTP packets were detected."
        ));
    }

    if (report.h264.unknown_count == 0) {
        findings.push_back(make_finding(
            AnomalySeverity::Ok,
            "no_unknown_h264_nal_units",
            "No unknown H.264 NAL units detected."
        ));
    } else {
        findings.push_back(make_finding(
            AnomalySeverity::Warning,
            "unknown_h264_nal_units",
            "Unknown H.264 NAL units were detected."
        ));
    }

    if (report.h264.sps_count > 0 && report.h264.pps_count > 0) {
        findings.push_back(make_finding(
            AnomalySeverity::Ok,
            "h264_parameter_sets_observed",
            "SPS and PPS NAL units were observed."
        ));
    } else {
        findings.push_back(make_finding(
            AnomalySeverity::Warning,
            "missing_h264_parameter_sets",
            "SPS or PPS NAL units were not observed during the capture."
        ));
    }

    if (report.h264.fu_a_start_count == report.h264.fu_a_end_count) {
        findings.push_back(make_finding(
            AnomalySeverity::Ok,
            "h264_fragmentation_balanced",
            "Observed FU-A fragmentation start/end counters are balanced."
        ));
    } else {
        findings.push_back(make_finding(
            AnomalySeverity::Warning,
            "incomplete_h264_fragmentation",
            "FU-A fragmentation start/end counters are not balanced. The capture may have started or stopped in the middle of a fragmented frame."
        ));
    }

    if (report.rtp_quality.packets_observed < 2) {
        findings.push_back(make_finding(
            AnomalySeverity::Warning,
            "insufficient_jitter_samples",
            "Not enough RTP packets were observed to estimate jitter reliably."
        ));
    } else {
        if (report.rtp_quality.jitter_ms <= kJitterWarningThresholdMs) {
            findings.push_back(make_finding(
                AnomalySeverity::Ok,
                "rtp_jitter_within_basic_threshold",
                "RTP jitter is within the basic threshold."
            ));
        } else {
            findings.push_back(make_finding(
                AnomalySeverity::Warning,
                "high_rtp_jitter",
                "RTP jitter is above the basic threshold."
            ));
        }

        if (report.rtp_quality.max_interarrival_gap_ms <= kLargeInterarrivalGapThresholdMs) {
            findings.push_back(make_finding(
                AnomalySeverity::Ok,
                "no_large_interarrival_gap",
                "No large RTP inter-arrival gap detected."
            ));
        } else {
            findings.push_back(make_finding(
                AnomalySeverity::Warning,
                "large_interarrival_gap",
                "A large RTP inter-arrival gap was detected."
            ));
        }
    }

    if (report.rtcp.frames_received > 0) {
        findings.push_back(make_finding(
            AnomalySeverity::Ok,
            "rtcp_observed",
            "RTCP traffic was observed."
        ));

        if (report.rtcp.sender_reports > 0) {
            findings.push_back(make_finding(
                AnomalySeverity::Ok,
                "rtcp_sender_report_observed",
                "At least one RTCP Sender Report was observed."
            ));
        } else {
            findings.push_back(make_finding(
                AnomalySeverity::Warning,
                "no_rtcp_sender_report",
                "No RTCP Sender Report was observed during the capture."
            ));
        }
    } else {
        findings.push_back(make_finding(
            AnomalySeverity::Warning,
            "no_rtcp_observed",
            "No RTCP traffic was observed during the capture."
        ));
    }

    if (report.rtcp.malformed_packets == 0) {
        findings.push_back(make_finding(
            AnomalySeverity::Ok,
            "no_malformed_rtcp_packets",
            "No malformed RTCP packets were detected."
        ));
    } else {
        findings.push_back(make_finding(
            AnomalySeverity::Warning,
            "malformed_rtcp_packets",
            "Malformed RTCP packets were detected."
        ));
    }

    if (report.teardown_success) {
        findings.push_back(make_finding(
            AnomalySeverity::Ok,
            "rtsp_teardown_success",
            "RTSP TEARDOWN completed successfully."
        ));
    } else {
        findings.push_back(make_finding(
            AnomalySeverity::Warning,
            "rtsp_teardown_failed",
            "RTSP TEARDOWN did not complete successfully."
        ));
    }

    findings.push_back(make_finding(
        AnomalySeverity::Warning,
        "unencrypted_rtsp",
        "RTSP traffic is not encrypted; credentials and media metadata may be exposed on the network."
    ));

    findings.push_back(make_finding(
        AnomalySeverity::Warning,
        "basic_auth_may_be_in_use",
        "If Basic authentication is used over plain RTSP, credentials are only Base64-encoded and not encrypted."
    ));

    return findings;
}

} // namespace rtsi
