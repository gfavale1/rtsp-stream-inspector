#pragma once

#include "rtsi/rtcp/RtcpPacket.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>

namespace rtsi {

struct RtcpStatsSnapshot {
    std::size_t frames_received = 0;
    std::size_t packets_parsed = 0;
    std::size_t malformed_packets = 0;

    std::size_t sender_reports = 0;
    std::size_t receiver_reports = 0;
    std::size_t source_description_packets = 0;
    std::size_t bye_packets = 0;
    std::size_t app_packets = 0;
    std::size_t unknown_packets = 0;

    std::optional<std::uint32_t> last_sender_ssrc;
    std::optional<std::uint32_t> last_rtp_timestamp_from_sr;
    std::optional<std::uint32_t> last_sender_packet_count;
    std::optional<std::uint32_t> last_sender_octet_count;

    std::optional<std::uint32_t> last_report_block_jitter;
    std::optional<std::uint8_t> last_fraction_lost;
    std::optional<std::uint32_t> last_cumulative_lost;
};

class RtcpStats {
public:
    void observe_frame() noexcept;
    void observe_packet(const RtcpPacket& packet);
    void observe_malformed_packet() noexcept;

    [[nodiscard]] RtcpStatsSnapshot snapshot() const noexcept;

private:
    RtcpStatsSnapshot snapshot_{};
};

} // namespace rtsi
