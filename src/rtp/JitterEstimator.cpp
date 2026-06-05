#include "rtsi/rtp/JitterEstimator.hpp"

#include <algorithm>
#include <cmath>

namespace rtsi {
namespace {

constexpr std::uint32_t kDefaultClockRate = 90000;

std::uint32_t sanitize_clock_rate(std::uint32_t clock_rate) noexcept {
  return clock_rate == 0 ? kDefaultClockRate : clock_rate;
}

} // namespace

JitterEstimator::JitterEstimator(std::uint32_t clock_rate) {
  set_clock_rate(clock_rate);
}

void JitterEstimator::reset() {
  packets_observed_ = 0;
  has_first_arrival_ = false;
  first_arrival_time_ = {};
  has_previous_ = false;
  previous_rtp_timestamp_ = 0;
  previous_arrival_time_ = {};
  previous_transit_ = 0.0;
  jitter_timestamp_units_ = 0.0;
  total_interarrival_gap_ms_ = 0.0;
  max_interarrival_gap_ms_ = 0.0;
}

void JitterEstimator::set_clock_rate(std::uint32_t clock_rate) {
  clock_rate_ = sanitize_clock_rate(clock_rate);
}

void JitterEstimator::update(
    std::uint32_t rtp_timestamp,
    std::chrono::steady_clock::time_point arrival_time) {
  if (!has_first_arrival_) {
    first_arrival_time_ = arrival_time;
    has_first_arrival_ = true;
  }

  const auto elapsed_from_first = arrival_time - first_arrival_time_;
  const double arrival_seconds =
      std::chrono::duration<double>(elapsed_from_first).count();
  const double arrival_timestamp_units =
      arrival_seconds * static_cast<double>(clock_rate_);

  const double transit =
      arrival_timestamp_units - static_cast<double>(rtp_timestamp);

  if (has_previous_) {
    const double gap_ms =
        std::chrono::duration<double, std::milli>(arrival_time -
                                                 previous_arrival_time_)
            .count();

    total_interarrival_gap_ms_ += gap_ms;
    max_interarrival_gap_ms_ = std::max(max_interarrival_gap_ms_, gap_ms);

    double delta = transit - previous_transit_;
    if (delta < 0.0) {
      delta = -delta;
    }

    jitter_timestamp_units_ += (delta - jitter_timestamp_units_) / 16.0;
  }

  previous_rtp_timestamp_ = rtp_timestamp;
  previous_arrival_time_ = arrival_time;
  previous_transit_ = transit;
  has_previous_ = true;
  ++packets_observed_;
}

JitterSnapshot JitterEstimator::snapshot() const noexcept {
  JitterSnapshot result;
  result.packets_observed = packets_observed_;
  result.jitter_timestamp_units = jitter_timestamp_units_;

  const auto clock_rate = sanitize_clock_rate(clock_rate_);
  result.jitter_seconds =
      jitter_timestamp_units_ / static_cast<double>(clock_rate);
  result.jitter_ms = result.jitter_seconds * 1000.0;

  if (packets_observed_ > 1) {
    result.average_interarrival_gap_ms =
        total_interarrival_gap_ms_ /
        static_cast<double>(packets_observed_ - 1);
  }

  result.max_interarrival_gap_ms = max_interarrival_gap_ms_;
  return result;
}

} // namespace rtsi
