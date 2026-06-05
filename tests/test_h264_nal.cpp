#include "rtsi/h264/H264Analyzer.hpp"
#include "rtsi/h264/NalUnit.hpp"

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <stdexcept>
#include <vector>

TEST_CASE("NalUnitParser parses single H.264 NAL units", "[h264][nal]") {
  REQUIRE(rtsi::NalUnitParser::parse_rtp_payload({0x67}).type ==
          rtsi::H264NalUnitType::Sps);
  REQUIRE(rtsi::NalUnitParser::parse_rtp_payload({0x68}).type ==
          rtsi::H264NalUnitType::Pps);
  REQUIRE(rtsi::NalUnitParser::parse_rtp_payload({0x65}).type ==
          rtsi::H264NalUnitType::IdrSlice);
  REQUIRE(rtsi::NalUnitParser::parse_rtp_payload({0x41}).type ==
          rtsi::H264NalUnitType::NonIdrSlice);
  REQUIRE(rtsi::NalUnitParser::parse_rtp_payload({0x06}).type ==
          rtsi::H264NalUnitType::Sei);
  REQUIRE(rtsi::NalUnitParser::parse_rtp_payload({0x78}).type ==
          rtsi::H264NalUnitType::StapA);
}

TEST_CASE("NalUnitParser parses FU-A start and end fragments", "[h264][nal]") {
  const auto start = rtsi::NalUnitParser::parse_rtp_payload({0x7C, 0x85});
  REQUIRE(start.type == rtsi::H264NalUnitType::FuA);
  REQUIRE(start.original_fragment_type == rtsi::H264NalUnitType::IdrSlice);
  REQUIRE(start.is_fragment);
  REQUIRE(start.fragment_start);
  REQUIRE_FALSE(start.fragment_end);

  const auto end = rtsi::NalUnitParser::parse_rtp_payload({0x7C, 0x45});
  REQUIRE(end.type == rtsi::H264NalUnitType::FuA);
  REQUIRE(end.original_fragment_type == rtsi::H264NalUnitType::IdrSlice);
  REQUIRE(end.is_fragment);
  REQUIRE_FALSE(end.fragment_start);
  REQUIRE(end.fragment_end);
}

TEST_CASE("NalUnitParser rejects invalid H.264 RTP payloads", "[h264][nal]") {
  REQUIRE_THROWS_AS(rtsi::NalUnitParser::parse_rtp_payload({}),
                    std::invalid_argument);
  REQUIRE_THROWS_AS(rtsi::NalUnitParser::parse_rtp_payload({0x7C}),
                    std::invalid_argument);
}

TEST_CASE("H264Analyzer counts observed NAL units", "[h264][analyzer]") {
  rtsi::H264Analyzer analyzer;

  analyzer.update(rtsi::NalUnitParser::parse_rtp_payload({0x67}));
  analyzer.update(rtsi::NalUnitParser::parse_rtp_payload({0x68}));
  analyzer.update(rtsi::NalUnitParser::parse_rtp_payload({0x65}));
  analyzer.update(rtsi::NalUnitParser::parse_rtp_payload({0x41}));
  analyzer.update(rtsi::NalUnitParser::parse_rtp_payload({0x06}));
  analyzer.update(rtsi::NalUnitParser::parse_rtp_payload({0x78}));
  analyzer.update(rtsi::NalUnitParser::parse_rtp_payload({0x7C, 0x85}));
  analyzer.update(rtsi::NalUnitParser::parse_rtp_payload({0x7C, 0x45}));
  analyzer.update(rtsi::NalUnitParser::parse_rtp_payload({0x1F}));

  const auto snapshot = analyzer.snapshot();

  REQUIRE(snapshot.nal_units_seen == 9);
  REQUIRE(snapshot.sps_count == 1);
  REQUIRE(snapshot.pps_count == 1);
  REQUIRE(snapshot.idr_count == 3); // one single IDR + two FU-A IDR fragments
  REQUIRE(snapshot.non_idr_count == 1);
  REQUIRE(snapshot.sei_count == 1);
  REQUIRE(snapshot.stap_a_count == 1);
  REQUIRE(snapshot.fu_a_count == 2);
  REQUIRE(snapshot.fu_a_start_count == 1);
  REQUIRE(snapshot.fu_a_end_count == 1);
  REQUIRE(snapshot.unknown_count == 1);
}
