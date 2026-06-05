#pragma once

#include "rtsi/report/AnalysisReport.hpp"

#include <string>

namespace rtsi {

class MarkdownReportWriter {
public:
  void write_report(const AnalysisReport& report,
                    const std::string& output_path) const;
};

} // namespace rtsi
