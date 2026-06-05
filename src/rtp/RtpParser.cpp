#include "rtsi/rtp/RtpParser.hpp"

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace rtsi {

namespace {

std::uint16_t read_u16_be(const std::vector<std::uint8_t>& data, std::size_t offset) {
    return static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(data[offset]) << 8) |
        static_cast<std::uint16_t>(data[offset + 1])
    );
}

std::uint32_t read_u32_be(const std::vector<std::uint8_t>& data, std::size_t offset) {
    return (static_cast<std::uint32_t>(data[offset]) << 24) |
           (static_cast<std::uint32_t>(data[offset + 1]) << 16) |
           (static_cast<std::uint32_t>(data[offset + 2]) << 8) |
           static_cast<std::uint32_t>(data[offset + 3]);
}

} // namespace

RtpPacket RtpParser::parse(const std::vector<std::uint8_t>& data) {
    if (data.size() < 12) {
        throw std::invalid_argument("RTP packet too short: minimum header size is 12 bytes");
    }

    RtpPacket packet;

    const std::uint8_t first = data[0];
    const std::uint8_t second = data[1];

    packet.version = static_cast<std::uint8_t>((first >> 6) & 0x03);
    packet.padding = ((first >> 5) & 0x01) != 0;
    packet.extension = ((first >> 4) & 0x01) != 0;
    packet.csrc_count = static_cast<std::uint8_t>(first & 0x0F);

    if (packet.version != 2) {
        throw std::invalid_argument("Unsupported RTP version");
    }

    packet.marker = ((second >> 7) & 0x01) != 0;
    packet.payload_type = static_cast<std::uint8_t>(second & 0x7F);

    packet.sequence_number = read_u16_be(data, 2);
    packet.timestamp = read_u32_be(data, 4);
    packet.ssrc = read_u32_be(data, 8);

    std::size_t header_size = 12 + static_cast<std::size_t>(packet.csrc_count) * 4;

    if (data.size() < header_size) {
        throw std::invalid_argument("RTP packet too short for CSRC list");
    }

    if (packet.extension) {
        if (data.size() < header_size + 4) {
            throw std::invalid_argument("RTP packet too short for extension header");
        }

        const std::uint16_t extension_length_words =
            read_u16_be(data, header_size + 2);

        header_size += 4 + static_cast<std::size_t>(extension_length_words) * 4;

        if (data.size() < header_size) {
            throw std::invalid_argument("RTP packet too short for extension payload");
        }
    }

    packet.header_size = header_size;

    packet.payload.assign(
        data.begin() + static_cast<std::ptrdiff_t>(header_size),
        data.end()
    );

    return packet;
}

} // namespace rtsi