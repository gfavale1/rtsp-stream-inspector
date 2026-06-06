#include "rtsi/rtcp/RtcpPacket.hpp"

namespace rtsi {

bool RtcpPacket::is_sender_report() const noexcept {
    return packet_type == RtcpPacketType::SenderReport;
}

bool RtcpPacket::is_receiver_report() const noexcept {
    return packet_type == RtcpPacketType::ReceiverReport;
}

bool RtcpPacket::is_sdes() const noexcept {
    return packet_type == RtcpPacketType::SourceDescription;
}

bool RtcpPacket::is_bye() const noexcept {
    return packet_type == RtcpPacketType::Bye;
}

RtcpPacketType rtcp_packet_type_from_byte(std::uint8_t value) noexcept {
    switch (value) {
    case 200:
        return RtcpPacketType::SenderReport;
    case 201:
        return RtcpPacketType::ReceiverReport;
    case 202:
        return RtcpPacketType::SourceDescription;
    case 203:
        return RtcpPacketType::Bye;
    case 204:
        return RtcpPacketType::ApplicationDefined;
    default:
        return RtcpPacketType::Unknown;
    }
}

std::string to_string(RtcpPacketType type) {
    switch (type) {
    case RtcpPacketType::SenderReport:
        return "sender_report";
    case RtcpPacketType::ReceiverReport:
        return "receiver_report";
    case RtcpPacketType::SourceDescription:
        return "source_description";
    case RtcpPacketType::Bye:
        return "bye";
    case RtcpPacketType::ApplicationDefined:
        return "application_defined";
    case RtcpPacketType::Unknown:
        return "unknown";
    }
    return "unknown";
}

} // namespace rtsi
