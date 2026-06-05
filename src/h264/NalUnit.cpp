#include "rtsi/h264/NalUnit.hpp"

#include <stdexcept>

namespace rtsi {

namespace {

H264NalUnitType to_nal_type(std::uint8_t raw_type) {
    switch (raw_type) {
        case 0:
            return H264NalUnitType::Unspecified;
        case 1:
            return H264NalUnitType::NonIdrSlice;
        case 2:
            return H264NalUnitType::DataPartitionA;
        case 3:
            return H264NalUnitType::DataPartitionB;
        case 4:
            return H264NalUnitType::DataPartitionC;
        case 5:
            return H264NalUnitType::IdrSlice;
        case 6:
            return H264NalUnitType::Sei;
        case 7:
            return H264NalUnitType::Sps;
        case 8:
            return H264NalUnitType::Pps;
        case 9:
            return H264NalUnitType::Aud;
        case 10:
            return H264NalUnitType::EndOfSequence;
        case 11:
            return H264NalUnitType::EndOfStream;
        case 12:
            return H264NalUnitType::FillerData;
        case 24:
            return H264NalUnitType::StapA;
        case 28:
            return H264NalUnitType::FuA;
        default:
            return H264NalUnitType::Unknown;
    }
}

} // namespace

bool H264NalUnitInfo::is_sps() const noexcept {
    return type == H264NalUnitType::Sps ||
           original_fragment_type == H264NalUnitType::Sps;
}

bool H264NalUnitInfo::is_pps() const noexcept {
    return type == H264NalUnitType::Pps ||
           original_fragment_type == H264NalUnitType::Pps;
}

bool H264NalUnitInfo::is_idr() const noexcept {
    return type == H264NalUnitType::IdrSlice ||
           original_fragment_type == H264NalUnitType::IdrSlice;
}

bool H264NalUnitInfo::is_non_idr() const noexcept {
    return type == H264NalUnitType::NonIdrSlice ||
           original_fragment_type == H264NalUnitType::NonIdrSlice;
}

bool H264NalUnitInfo::is_fu_a() const noexcept {
    return type == H264NalUnitType::FuA;
}

bool H264NalUnitInfo::is_stap_a() const noexcept {
    return type == H264NalUnitType::StapA;
}

H264NalUnitInfo NalUnitParser::parse_rtp_payload(
    const std::vector<std::uint8_t>& payload
) {
    if (payload.empty()) {
        throw std::invalid_argument("H.264 RTP payload cannot be empty");
    }

    H264NalUnitInfo info;
    info.rtp_payload_size = payload.size();

    const std::uint8_t nal_header = payload[0];

    info.forbidden_zero_bit = ((nal_header >> 7) & 0x01) != 0;
    info.nal_ref_idc = static_cast<std::uint8_t>((nal_header >> 5) & 0x03);

    const std::uint8_t nal_type_raw = static_cast<std::uint8_t>(nal_header & 0x1F);
    info.type = to_nal_type(nal_type_raw);

    if (info.type == H264NalUnitType::FuA) {
        if (payload.size() < 2) {
            throw std::invalid_argument("FU-A payload too short");
        }

        const std::uint8_t fu_header = payload[1];

        info.is_fragment = true;
        info.fragment_start = ((fu_header >> 7) & 0x01) != 0;
        info.fragment_end = ((fu_header >> 6) & 0x01) != 0;

        const std::uint8_t original_type_raw =
            static_cast<std::uint8_t>(fu_header & 0x1F);

        info.original_fragment_type = to_nal_type(original_type_raw);
    }

    return info;
}

std::string to_string(H264NalUnitType type) {
    switch (type) {
        case H264NalUnitType::Unspecified:
            return "Unspecified";
        case H264NalUnitType::NonIdrSlice:
            return "Non-IDR slice";
        case H264NalUnitType::DataPartitionA:
            return "Data partition A";
        case H264NalUnitType::DataPartitionB:
            return "Data partition B";
        case H264NalUnitType::DataPartitionC:
            return "Data partition C";
        case H264NalUnitType::IdrSlice:
            return "IDR slice";
        case H264NalUnitType::Sei:
            return "SEI";
        case H264NalUnitType::Sps:
            return "SPS";
        case H264NalUnitType::Pps:
            return "PPS";
        case H264NalUnitType::Aud:
            return "AUD";
        case H264NalUnitType::EndOfSequence:
            return "End of sequence";
        case H264NalUnitType::EndOfStream:
            return "End of stream";
        case H264NalUnitType::FillerData:
            return "Filler data";
        case H264NalUnitType::StapA:
            return "STAP-A";
        case H264NalUnitType::FuA:
            return "FU-A";
        case H264NalUnitType::Unknown:
            return "Unknown";
    }

    return "Unknown";
}

} // namespace rtsi