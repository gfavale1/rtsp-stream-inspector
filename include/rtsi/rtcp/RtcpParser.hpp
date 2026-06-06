#pragma once

#include "rtsi/rtcp/RtcpPacket.hpp"

#include <cstdint>
#include <vector>

namespace rtsi {

class RtcpParser {
public:
    [[nodiscard]] static std::vector<RtcpPacket> parse_compound_packet(
        const std::vector<std::uint8_t>& bytes);

    [[nodiscard]] static RtcpPacket parse_single_packet(
        const std::vector<std::uint8_t>& bytes);
};

} // namespace rtsi
