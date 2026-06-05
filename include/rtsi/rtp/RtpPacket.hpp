#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace rtsi {

struct RtpPacket {
    std::uint8_t version = 0;
    bool padding = false;
    bool extension = false;
    std::uint8_t csrc_count = 0;

    bool marker = false;
    std::uint8_t payload_type = 0;

    std::uint16_t sequence_number = 0;
    std::uint32_t timestamp = 0;
    std::uint32_t ssrc = 0;

    std::size_t header_size = 0;
    std::vector<std::uint8_t> payload;

    [[nodiscard]] std::size_t payload_size() const noexcept;
};

} // namespace rtsi