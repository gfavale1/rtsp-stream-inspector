#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace rtsi {

enum class RtcpPacketType : std::uint8_t {
    SenderReport = 200,
    ReceiverReport = 201,
    SourceDescription = 202,
    Bye = 203,
    ApplicationDefined = 204,
    Unknown = 0,
};

struct RtcpReportBlock {
    std::uint32_t ssrc = 0;
    std::uint8_t fraction_lost = 0;
    std::uint32_t cumulative_lost = 0;
    std::uint32_t extended_highest_sequence_number = 0;
    std::uint32_t interarrival_jitter = 0;
    std::uint32_t last_sr = 0;
    std::uint32_t delay_since_last_sr = 0;
};

struct RtcpSenderInfo {
    std::uint32_t sender_ssrc = 0;
    std::uint64_t ntp_timestamp = 0;
    std::uint32_t rtp_timestamp = 0;
    std::uint32_t sender_packet_count = 0;
    std::uint32_t sender_octet_count = 0;
};

struct RtcpPacket {
    std::uint8_t version = 0;
    bool padding = false;
    std::uint8_t report_count = 0;
    RtcpPacketType packet_type = RtcpPacketType::Unknown;
    std::uint16_t length_words = 0;
    std::size_t packet_size_bytes = 0;

    std::optional<RtcpSenderInfo> sender_info;
    std::optional<std::uint32_t> receiver_ssrc;
    std::vector<RtcpReportBlock> report_blocks;

    std::vector<std::uint32_t> sdes_ssrcs;
    std::vector<std::uint32_t> bye_ssrcs;

    [[nodiscard]] bool is_sender_report() const noexcept;
    [[nodiscard]] bool is_receiver_report() const noexcept;
    [[nodiscard]] bool is_sdes() const noexcept;
    [[nodiscard]] bool is_bye() const noexcept;
};

[[nodiscard]] RtcpPacketType rtcp_packet_type_from_byte(std::uint8_t value) noexcept;
[[nodiscard]] std::string to_string(RtcpPacketType type);

} // namespace rtsi
