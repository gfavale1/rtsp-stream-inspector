#include "rtsi/rtp/RtpStats.hpp"

namespace rtsi {

namespace {

std::uint16_t next_sequence(std::uint16_t sequence) {
    return static_cast<std::uint16_t>(sequence + 1);
}

std::uint16_t sequence_distance(std::uint16_t expected, std::uint16_t actual) {
    return static_cast<std::uint16_t>(actual - expected);
}

} // namespace

double RtpStatsSnapshot::loss_rate() const noexcept {
    const auto expected_total = packets_received + packets_lost;

    if (expected_total == 0) {
        return 0.0;
    }

    return static_cast<double>(packets_lost) /
           static_cast<double>(expected_total);
}

void RtpStats::update(const RtpPacket& packet, std::size_t rtp_packet_size) {
    if (!initialized_) {
        initialized_ = true;

        expected_sequence_number_ = next_sequence(packet.sequence_number);

        stats_.first_sequence_number = packet.sequence_number;
        stats_.last_sequence_number = packet.sequence_number;
        stats_.last_timestamp = packet.timestamp;
        stats_.ssrc = packet.ssrc;
        stats_.payload_type = packet.payload_type;

        stats_.packets_received += 1;
        stats_.total_rtp_bytes += rtp_packet_size;
        stats_.total_payload_bytes += packet.payload_size();

        return;
    }

    const auto distance =
        sequence_distance(expected_sequence_number_, packet.sequence_number);

    if (distance == 0) {
        expected_sequence_number_ = next_sequence(packet.sequence_number);
    } else if (distance < 32768) {
        stats_.packets_lost += distance;
        expected_sequence_number_ = next_sequence(packet.sequence_number);
    } else {
        stats_.out_of_order_packets += 1;
    }

    stats_.packets_received += 1;
    stats_.total_rtp_bytes += rtp_packet_size;
    stats_.total_payload_bytes += packet.payload_size();

    stats_.last_sequence_number = packet.sequence_number;
    stats_.last_timestamp = packet.timestamp;
    stats_.ssrc = packet.ssrc;
    stats_.payload_type = packet.payload_type;
}

RtpStatsSnapshot RtpStats::snapshot() const noexcept {
    return stats_;
}

} // namespace rtsi