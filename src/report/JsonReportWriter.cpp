#include "rtsi/report/JsonReportWriter.hpp"

#include <fstream>
#include <stdexcept>

#include <nlohmann/json.hpp>

namespace rtsi {

void JsonReportWriter::write_dummy_report(const AnalyzerConfig& config) const {
    nlohmann::json report;

    report["project"] = "rtsp-stream-inspector";
    report["version"] = "0.1.0";

    report["input"] = {
        {"url", config.url},
        {"duration_seconds", config.duration_seconds}
    };

    report["status"] = "dummy_report";
    report["message"] = "Initial project skeleton is working. RTSP/RTP analysis is not implemented yet.";

    report["metrics"] = {
        {"packets_received", 0},
        {"packets_lost", 0},
        {"loss_rate", 0.0},
        {"average_jitter_ms", 0.0},
        {"bitrate_kbps", 0.0}
    };

    std::ofstream file(config.output_path);

    if (!file) {
        throw std::runtime_error("Unable to open output file: " + config.output_path);
    }

    file << report.dump(4) << '\n';
}

} // namespace rtsi