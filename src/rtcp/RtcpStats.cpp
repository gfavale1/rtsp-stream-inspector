#include "rtsi/rtcp/RtcpStats.hpp"

namespace rtsi {

void RtcpStats::observe_frame() noexcept {
    ++snapshot_.frames_received;
}

void RtcpStats::observe_packet(const RtcpPacket& packet) {
    ++snapshot_.packets_parsed;

    switch (packet.packet_type) {
    case RtcpPacketType::SenderReport:
        ++snapshot_.sender_reports;
        break;
    case RtcpPacketType::ReceiverReport:
        ++snapshot_.receiver_reports;
        break;
    case RtcpPacketType::SourceDescription:
        ++snapshot_.source_description_packets;
        break;
    case RtcpPacketType::Bye:
        ++snapshot_.bye_packets;
        break;
    case RtcpPacketType::ApplicationDefined:
        ++snapshot_.app_packets;
        break;
    case RtcpPacketType::Unknown:
        ++snapshot_.unknown_packets;
        break;
    }

    if (packet.sender_info.has_value()) {
        const auto& sender_info = packet.sender_info.value();
        snapshot_.last_sender_ssrc = sender_info.sender_ssrc;
        snapshot_.last_rtp_timestamp_from_sr = sender_info.rtp_timestamp;
        snapshot_.last_sender_packet_count = sender_info.sender_packet_count;
        snapshot_.last_sender_octet_count = sender_info.sender_octet_count;
    }

    if (!packet.report_blocks.empty()) {
        const auto& block = packet.report_blocks.back();
        snapshot_.last_report_block_jitter = block.interarrival_jitter;
        snapshot_.last_fraction_lost = block.fraction_lost;
        snapshot_.last_cumulative_lost = block.cumulative_lost;
    }
}

void RtcpStats::observe_malformed_packet() noexcept {
    ++snapshot_.malformed_packets;
}

RtcpStatsSnapshot RtcpStats::snapshot() const noexcept {
    return snapshot_;
}

} // namespace rtsi
