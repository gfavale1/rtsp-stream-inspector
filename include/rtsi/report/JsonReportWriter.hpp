#pragma once

#include "rtsi/app/AnalyzerConfig.hpp"

namespace rtsi {

class JsonReportWriter {
public:
    void write_dummy_report(const AnalyzerConfig& config) const;
};

} // namespace rtsi