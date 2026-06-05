#include "rtsi/rtp/JitterEstimator.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <cmath>

using Clock = std::chrono::steady_clock;

TEST_CASE("JitterEstimator starts with zero-valued metrics", "[rtp][jitter]") {
  const rtsi::JitterEstimator estimator(90000);
  const auto snapshot = estimator.snapshot();

  REQUIRE(snapshot.packets_observed == 0);
  REQUIRE(snapshot.jitter_ms == 0.0);
  REQUIRE(snapshot.average_interarrival_gap_ms == 0.0);
  REQUIRE(snapshot.max_interarrival_gap_ms == 0.0);
}

TEST_CASE("JitterEstimator reports near-zero jitter for regular RTP arrivals", "[rtp][jitter]") {
  rtsi::JitterEstimator estimator(90000);
  const Clock::time_point start{};

  estimator.update(0, start);
  estimator.update(3000, start + std::chrono::microseconds(33333));
  estimator.update(6000, start + std::chrono::microseconds(66666));

  const auto snapshot = estimator.snapshot();

  REQUIRE(snapshot.packets_observed == 3);
  REQUIRE(snapshot.jitter_ms == Catch::Approx(0.0).margin(0.01));
  REQUIRE(snapshot.average_interarrival_gap_ms ==
          Catch::Approx(33.333).margin(0.01));
  REQUIRE(snapshot.max_interarrival_gap_ms ==
          Catch::Approx(33.333).margin(0.01));
}

TEST_CASE("JitterEstimator detects irregular RTP arrivals", "[rtp][jitter]") {
  rtsi::JitterEstimator estimator(90000);
  const Clock::time_point start{};

  estimator.update(0, start);
  estimator.update(3000, start + std::chrono::microseconds(33333));
  estimator.update(6000, start + std::chrono::microseconds(100000));

  const auto snapshot = estimator.snapshot();

  REQUIRE(snapshot.packets_observed == 3);
  REQUIRE(snapshot.jitter_ms > 0.0);
  REQUIRE(snapshot.max_interarrival_gap_ms ==
          Catch::Approx(66.667).margin(0.01));
}

TEST_CASE("JitterEstimator handles zero clock rate without crashing", "[rtp][jitter]") {
  rtsi::JitterEstimator estimator(0);
  const Clock::time_point start{};

  estimator.update(0, start);
  estimator.update(3000, start + std::chrono::microseconds(33333));

  const auto snapshot = estimator.snapshot();

  REQUIRE(snapshot.packets_observed == 2);
  REQUIRE(std::isfinite(snapshot.jitter_ms));
  REQUIRE(std::isfinite(snapshot.average_interarrival_gap_ms));
  REQUIRE(std::isfinite(snapshot.max_interarrival_gap_ms));
}
