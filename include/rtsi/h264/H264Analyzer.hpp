#pragma once

#include "rtsi/h264/NalUnit.hpp"

#include <cstddef>

namespace rtsi {

struct H264AnalysisSnapshot {
    std::size_t nal_units_seen = 0;

    std::size_t sps_count = 0;
    std::size_t pps_count = 0;
    std::size_t sei_count = 0;
    std::size_t idr_count = 0;
    std::size_t non_idr_count = 0;

    std::size_t stap_a_count = 0;
    std::size_t fu_a_count = 0;
    std::size_t fu_a_start_count = 0;
    std::size_t fu_a_end_count = 0;

    std::size_t unknown_count = 0;
};

class H264Analyzer {
public:
    void update(const H264NalUnitInfo& nal_info);

    [[nodiscard]] H264AnalysisSnapshot snapshot() const noexcept;

private:
    H264AnalysisSnapshot stats_;
};

} // namespace rtsi