#pragma once

#include "rtsi/h264/H264Analyzer.hpp"
#include "rtsi/metrics/StreamMetrics.hpp"
#include "rtsi/rtp/RtpStats.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace rtsi {

struct ReportMetadata {
  std::string tool = "rtsp-stream-inspector";
  std::string version = "0.1.0";
  std::string schema_version = "1.0";
  std::string command;
  std::string generated_at_utc;
};

struct AnalysisConfigurationReport {
  std::size_t frames_requested = 0;
  std::size_t packet_log_limit = 0;
  int timeout_ms = 0;
  std::string transport = "rtp_interleaved_tcp";
};

struct SourceReport {
  std::string host;
  std::uint16_t port = 554;
  std::string path;
  std::string transport = "rtp_interleaved_tcp";
};

struct VideoTrackReport {
  std::string codec;
  int payload_type = -1;
  int clock_rate = -1;
  std::string control;
};

struct InterleavedCaptureReport {
  std::size_t rtp_frames_received = 0;
  std::size_t rtcp_frames_received = 0;
  std::size_t total_interleaved_payload_bytes = 0;
};

struct RtpQualityReport {
  std::size_t packets_observed = 0;
  double jitter_timestamp_units = 0.0;
  double jitter_seconds = 0.0;
  double jitter_ms = 0.0;
  double average_interarrival_gap_ms = 0.0;
  double max_interarrival_gap_ms = 0.0;
};

struct ReportFinding {
  std::string severity;
  std::string code;
  std::string message;
};

struct AnalysisReport {
  ReportMetadata metadata;
  AnalysisConfigurationReport configuration;

  SourceReport source;
  VideoTrackReport video;
  InterleavedCaptureReport interleaved;

  RtpStatsSnapshot rtp;
  H264AnalysisSnapshot h264;
  StreamMetricsSnapshot stream;
  RtpQualityReport rtp_quality;

  bool teardown_success = false;
  std::vector<ReportFinding> findings;
};

} // namespace rtsi
