#pragma once

#include <cstdint>
#include <string>

namespace rtsi {

struct AnalyzerConfig {
    std::string url;
    std::uint32_t duration_seconds = 10;
    std::string output_path = "report.json";
};

} // namespace rtsi