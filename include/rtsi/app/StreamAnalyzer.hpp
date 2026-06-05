#pragma once

#include "rtsi/metrics/MetricsCollector.hpp"
#include "rtsi/report/AnalysisReport.hpp"
#include "rtsi/rtsp/InterleavedFrameReader.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>

namespace rtsi {

struct StreamAnalyzerConfig {
  int frame_count = 300;
  int packet_log_limit = 20;
  int rtp_clock_rate = 90000;
};

enum class InterleavedFrameType {
  Rtp,
  Rtcp,
  Unknown,
};

struct StreamAnalyzerPacketLogEntry {
  std::size_t frame_index = 0;
  std::uint8_t channel = 0;
  std::size_t payload_size = 0;
  InterleavedFrameType type = InterleavedFrameType::Unknown;
};

struct StreamAnalyzerResult {
  std::size_t rtp_frames_received = 0;
  std::size_t rtcp_frames_received = 0;
  std::size_t interleaved_frames_received = 0;
  std::size_t total_payload_bytes = 0;

  MetricsCollectorSnapshot metrics;
  AnalysisReport report;
  std::optional<std::string> stop_reason;
};

class StreamAnalyzer {
public:
  using PacketLogCallback =
      std::function<void(const StreamAnalyzerPacketLogEntry&)>;

  [[nodiscard]] StreamAnalyzerResult analyze(
      InterleavedFrameReader& reader,
      const StreamAnalyzerConfig& config,
      PacketLogCallback on_packet_log = {}) const;
};

} // namespace rtsi
