#pragma once

#include "rtsi/app/AnalyzerConfig.hpp"
#include "rtsi/report/AnalysisReport.hpp"

#include <string>

namespace rtsi {

class JsonReportWriter {
public:
  void write_report(const AnalysisReport& report,
                    const std::string& output_path) const;

  [[deprecated("Use write_report(const AnalysisReport&, const std::string&) instead")]]
  void write_dummy_report(const AnalyzerConfig& config) const;
};

} // namespace rtsi
