#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>

namespace rtsi {

struct JitterSnapshot {
  std::size_t packets_observed = 0;

  double jitter_timestamp_units = 0.0;
  double jitter_seconds = 0.0;
  double jitter_ms = 0.0;

  double average_interarrival_gap_ms = 0.0;
  double max_interarrival_gap_ms = 0.0;
};

class JitterEstimator {
public:
  explicit JitterEstimator(std::uint32_t clock_rate = 90000);

  void reset();
  void set_clock_rate(std::uint32_t clock_rate);

  void update(std::uint32_t rtp_timestamp,
              std::chrono::steady_clock::time_point arrival_time);

  [[nodiscard]] JitterSnapshot snapshot() const noexcept;

private:
  using Clock = std::chrono::steady_clock;

  std::uint32_t clock_rate_ = 90000;

  std::size_t packets_observed_ = 0;

  bool has_first_arrival_ = false;
  Clock::time_point first_arrival_time_{};

  bool has_previous_ = false;
  std::uint32_t previous_rtp_timestamp_ = 0;
  Clock::time_point previous_arrival_time_{};

  double previous_transit_ = 0.0;
  double jitter_timestamp_units_ = 0.0;

  double total_interarrival_gap_ms_ = 0.0;
  double max_interarrival_gap_ms_ = 0.0;
};

} // namespace rtsi
