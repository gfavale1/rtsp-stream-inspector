#pragma once

#include "rtsi/rtp/RtpPacket.hpp"

#include <cstdint>
#include <vector>

namespace rtsi {

class RtpParser {
public:
    [[nodiscard]] static RtpPacket parse(const std::vector<std::uint8_t>& data);
};

} // namespace rtsi