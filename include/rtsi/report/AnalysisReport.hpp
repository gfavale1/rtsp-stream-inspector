#pragma once

#include "rtsi/h264/H264Analyzer.hpp"
#include "rtsi/metrics/StreamMetrics.hpp"
#include "rtsi/rtp/RtpStats.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace rtsi {

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

struct ReportFinding {
  std::string severity;
  std::string code;
  std::string message;
};

struct AnalysisReport {
  SourceReport source;
  VideoTrackReport video;
  InterleavedCaptureReport interleaved;

  RtpStatsSnapshot rtp;
  H264AnalysisSnapshot h264;
  StreamMetricsSnapshot stream;

  bool teardown_success = false;

  std::vector<ReportFinding> findings;
};

} // namespace rtsi
