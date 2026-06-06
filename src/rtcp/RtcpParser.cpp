#include "rtsi/rtcp/RtcpParser.hpp"

#include <stdexcept>

namespace rtsi {
namespace {

std::uint16_t read_be16(const std::vector<std::uint8_t>& bytes, std::size_t offset) {
    return static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(bytes.at(offset)) << 8U)
        | static_cast<std::uint16_t>(bytes.at(offset + 1U)));
}

std::uint32_t read_be24(const std::vector<std::uint8_t>& bytes, std::size_t offset) {
    return (static_cast<std::uint32_t>(bytes.at(offset)) << 16U)
        | (static_cast<std::uint32_t>(bytes.at(offset + 1U)) << 8U)
        | static_cast<std::uint32_t>(bytes.at(offset + 2U));
}

std::uint32_t read_be32(const std::vector<std::uint8_t>& bytes, std::size_t offset) {
    return (static_cast<std::uint32_t>(bytes.at(offset)) << 24U)
        | (static_cast<std::uint32_t>(bytes.at(offset + 1U)) << 16U)
        | (static_cast<std::uint32_t>(bytes.at(offset + 2U)) << 8U)
        | static_cast<std::uint32_t>(bytes.at(offset + 3U));
}

std::uint64_t read_ntp64(const std::vector<std::uint8_t>& bytes, std::size_t offset) {
    const auto msw = static_cast<std::uint64_t>(read_be32(bytes, offset));
    const auto lsw = static_cast<std::uint64_t>(read_be32(bytes, offset + 4U));
    return (msw << 32U) | lsw;
}

RtcpReportBlock parse_report_block(
    const std::vector<std::uint8_t>& bytes,
    std::size_t offset) {
    RtcpReportBlock block;
    block.ssrc = read_be32(bytes, offset);
    block.fraction_lost = bytes.at(offset + 4U);
    block.cumulative_lost = read_be24(bytes, offset + 5U);
    block.extended_highest_sequence_number = read_be32(bytes, offset + 8U);
    block.interarrival_jitter = read_be32(bytes, offset + 12U);
    block.last_sr = read_be32(bytes, offset + 16U);
    block.delay_since_last_sr = read_be32(bytes, offset + 20U);
    return block;
}

void parse_report_blocks(
    RtcpPacket& packet,
    const std::vector<std::uint8_t>& bytes,
    std::size_t offset,
    std::size_t packet_end) {
    const auto required = offset + static_cast<std::size_t>(packet.report_count) * 24U;
    if (required > packet_end) {
        throw std::runtime_error("RTCP packet is shorter than declared report block count");
    }

    for (std::uint8_t index = 0; index < packet.report_count; ++index) {
        packet.report_blocks.push_back(parse_report_block(bytes, offset));
        offset += 24U;
    }
}

void parse_sender_report(
    RtcpPacket& packet,
    const std::vector<std::uint8_t>& bytes,
    std::size_t offset,
    std::size_t packet_end) {
    if (packet_end - offset < 28U) {
        throw std::runtime_error("RTCP Sender Report is shorter than 28 bytes");
    }

    RtcpSenderInfo sender_info;
    sender_info.sender_ssrc = read_be32(bytes, offset + 4U);
    sender_info.ntp_timestamp = read_ntp64(bytes, offset + 8U);
    sender_info.rtp_timestamp = read_be32(bytes, offset + 16U);
    sender_info.sender_packet_count = read_be32(bytes, offset + 20U);
    sender_info.sender_octet_count = read_be32(bytes, offset + 24U);
    packet.sender_info = sender_info;

    parse_report_blocks(packet, bytes, offset + 28U, packet_end);
}

void parse_receiver_report(
    RtcpPacket& packet,
    const std::vector<std::uint8_t>& bytes,
    std::size_t offset,
    std::size_t packet_end) {
    if (packet_end - offset < 8U) {
        throw std::runtime_error("RTCP Receiver Report is shorter than 8 bytes");
    }

    packet.receiver_ssrc = read_be32(bytes, offset + 4U);
    parse_report_blocks(packet, bytes, offset + 8U, packet_end);
}

void parse_sdes(
    RtcpPacket& packet,
    const std::vector<std::uint8_t>& bytes,
    std::size_t offset,
    std::size_t packet_end) {
    std::size_t cursor = offset + 4U;

    for (std::uint8_t index = 0; index < packet.report_count; ++index) {
        if (cursor + 4U > packet_end) {
            throw std::runtime_error("RTCP SDES chunk is missing SSRC");
        }

        packet.sdes_ssrcs.push_back(read_be32(bytes, cursor));
        cursor += 4U;

        while (cursor < packet_end) {
            const auto item_type = bytes.at(cursor++);
            if (item_type == 0U) {
                break;
            }
            if (cursor >= packet_end) {
                throw std::runtime_error("RTCP SDES item is missing length");
            }
            const auto item_length = bytes.at(cursor++);
            if (cursor + item_length > packet_end) {
                throw std::runtime_error("RTCP SDES item exceeds packet boundary");
            }
            cursor += item_length;
        }

        while ((cursor % 4U) != 0U && cursor < packet_end) {
            ++cursor;
        }
    }
}

void parse_bye(
    RtcpPacket& packet,
    const std::vector<std::uint8_t>& bytes,
    std::size_t offset,
    std::size_t packet_end) {
    std::size_t cursor = offset + 4U;
    const auto required = cursor + static_cast<std::size_t>(packet.report_count) * 4U;
    if (required > packet_end) {
        throw std::runtime_error("RTCP BYE is shorter than declared source count");
    }

    for (std::uint8_t index = 0; index < packet.report_count; ++index) {
        packet.bye_ssrcs.push_back(read_be32(bytes, cursor));
        cursor += 4U;
    }
}

RtcpPacket parse_at(const std::vector<std::uint8_t>& bytes, std::size_t offset) {
    if (offset + 4U > bytes.size()) {
        throw std::runtime_error("RTCP packet is shorter than 4 bytes");
    }

    const auto first = bytes.at(offset);
    RtcpPacket packet;
    packet.version = static_cast<std::uint8_t>((first >> 6U) & 0x03U);
    packet.padding = (first & 0x20U) != 0U;
    packet.report_count = static_cast<std::uint8_t>(first & 0x1FU);
    packet.packet_type = rtcp_packet_type_from_byte(bytes.at(offset + 1U));
    packet.length_words = read_be16(bytes, offset + 2U);
    packet.packet_size_bytes = (static_cast<std::size_t>(packet.length_words) + 1U) * 4U;

    if (packet.version != 2U) {
        throw std::runtime_error("Unsupported RTCP version");
    }
    if (packet.packet_size_bytes < 4U) {
        throw std::runtime_error("Invalid RTCP packet length");
    }
    if (offset + packet.packet_size_bytes > bytes.size()) {
        throw std::runtime_error("RTCP packet length exceeds available buffer");
    }

    const auto packet_end = offset + packet.packet_size_bytes;
    switch (packet.packet_type) {
    case RtcpPacketType::SenderReport:
        parse_sender_report(packet, bytes, offset, packet_end);
        break;
    case RtcpPacketType::ReceiverReport:
        parse_receiver_report(packet, bytes, offset, packet_end);
        break;
    case RtcpPacketType::SourceDescription:
        parse_sdes(packet, bytes, offset, packet_end);
        break;
    case RtcpPacketType::Bye:
        parse_bye(packet, bytes, offset, packet_end);
        break;
    case RtcpPacketType::ApplicationDefined:
    case RtcpPacketType::Unknown:
        break;
    }

    return packet;
}

} // namespace

std::vector<RtcpPacket> RtcpParser::parse_compound_packet(
    const std::vector<std::uint8_t>& bytes) {
    std::vector<RtcpPacket> packets;
    std::size_t offset = 0;

    while (offset < bytes.size()) {
        const auto packet = parse_at(bytes, offset);
        offset += packet.packet_size_bytes;
        packets.push_back(packet);
    }

    return packets;
}

RtcpPacket RtcpParser::parse_single_packet(
    const std::vector<std::uint8_t>& bytes) {
    const auto packet = parse_at(bytes, 0);
    if (packet.packet_size_bytes != bytes.size()) {
        throw std::runtime_error("RTCP single packet buffer contains trailing bytes");
    }
    return packet;
}

} // namespace rtsi
