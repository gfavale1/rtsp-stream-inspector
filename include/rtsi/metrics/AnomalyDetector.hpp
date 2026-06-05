#pragma once

#include "rtsi/report/AnalysisReport.hpp"

#include <string>
#include <vector>

namespace rtsi {

enum class AnomalySeverity {
  Ok,
  Warning,
  Critical,
};

struct AnomalyFinding {
  AnomalySeverity severity = AnomalySeverity::Ok;
  std::string code;
  std::string message;
};

class AnomalyDetector {
public:
  [[nodiscard]] std::vector<ReportFinding>
  analyze(const AnalysisReport& report) const;
};

[[nodiscard]] std::string to_string(AnomalySeverity severity);

} // namespace rtsi
