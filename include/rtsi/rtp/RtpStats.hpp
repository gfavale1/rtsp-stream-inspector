#pragma once

#include "rtsi/rtp/RtpPacket.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>

namespace rtsi {

struct RtpStatsSnapshot {
    std::size_t packets_received = 0;
    std::size_t packets_lost = 0;
    std::size_t out_of_order_packets = 0;

    std::size_t total_rtp_bytes = 0;
    std::size_t total_payload_bytes = 0;

    std::optional<std::uint16_t> first_sequence_number;
    std::optional<std::uint16_t> last_sequence_number;

    std::optional<std::uint32_t> last_timestamp;
    std::optional<std::uint32_t> ssrc;
    std::optional<std::uint8_t> payload_type;

    [[nodiscard]] double loss_rate() const noexcept;
};

class RtpStats {
public:
    void update(const RtpPacket& packet, std::size_t rtp_packet_size);

    [[nodiscard]] RtpStatsSnapshot snapshot() const noexcept;

private:
    bool initialized_ = false;

    std::uint16_t expected_sequence_number_ = 0;

    RtpStatsSnapshot stats_;
};

} // namespace rtsi