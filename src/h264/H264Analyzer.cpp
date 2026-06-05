#include "rtsi/h264/H264Analyzer.hpp"

namespace rtsi {

void H264Analyzer::update(const H264NalUnitInfo& nal_info) {
    stats_.nal_units_seen += 1;

    if (nal_info.is_sps()) {
        stats_.sps_count += 1;
    }

    if (nal_info.is_pps()) {
        stats_.pps_count += 1;
    }

    if (nal_info.type == H264NalUnitType::Sei) {
        stats_.sei_count += 1;
    }

    if (nal_info.is_idr()) {
        stats_.idr_count += 1;
    }

    if (nal_info.is_non_idr()) {
        stats_.non_idr_count += 1;
    }

    if (nal_info.is_stap_a()) {
        stats_.stap_a_count += 1;
    }

    if (nal_info.is_fu_a()) {
        stats_.fu_a_count += 1;

        if (nal_info.fragment_start) {
            stats_.fu_a_start_count += 1;
        }

        if (nal_info.fragment_end) {
            stats_.fu_a_end_count += 1;
        }
    }

    if (nal_info.type == H264NalUnitType::Unknown) {
        stats_.unknown_count += 1;
    }
}

H264AnalysisSnapshot H264Analyzer::snapshot() const noexcept {
    return stats_;
}

} // namespace rtsi