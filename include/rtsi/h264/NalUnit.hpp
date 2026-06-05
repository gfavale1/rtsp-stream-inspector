#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace rtsi {

enum class H264NalUnitType : std::uint8_t {
    Unspecified = 0,
    NonIdrSlice = 1,
    DataPartitionA = 2,
    DataPartitionB = 3,
    DataPartitionC = 4,
    IdrSlice = 5,
    Sei = 6,
    Sps = 7,
    Pps = 8,
    Aud = 9,
    EndOfSequence = 10,
    EndOfStream = 11,
    FillerData = 12,
    StapA = 24,
    FuA = 28,
    Unknown = 255
};

struct H264NalUnitInfo {
    H264NalUnitType type = H264NalUnitType::Unknown;
    H264NalUnitType original_fragment_type = H264NalUnitType::Unknown;

    bool forbidden_zero_bit = false;
    std::uint8_t nal_ref_idc = 0;

    bool is_fragment = false;
    bool fragment_start = false;
    bool fragment_end = false;

    std::size_t rtp_payload_size = 0;

    [[nodiscard]] bool is_sps() const noexcept;
    [[nodiscard]] bool is_pps() const noexcept;
    [[nodiscard]] bool is_idr() const noexcept;
    [[nodiscard]] bool is_non_idr() const noexcept;
    [[nodiscard]] bool is_fu_a() const noexcept;
    [[nodiscard]] bool is_stap_a() const noexcept;
};

class NalUnitParser {
public:
    [[nodiscard]] static H264NalUnitInfo parse_rtp_payload(
        const std::vector<std::uint8_t>& payload
    );
};

[[nodiscard]] std::string to_string(H264NalUnitType type);

} // namespace rtsi