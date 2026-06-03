#include "rtsi/app/AnalyzerConfig.hpp"
#include "rtsi/report/JsonReportWriter.hpp"
#include "rtsi/rtsp/RtspUrl.hpp"

#include <CLI/CLI.hpp>

#include <exception>
#include <iostream>

int main(int argc, char** argv) {
    CLI::App app{"Low-level RTSP/RTP stream inspection tool"};

    rtsi::AnalyzerConfig config;

    auto analyze_cmd = app.add_subcommand("analyze", "Analyze an RTSP stream");

    analyze_cmd
        ->add_option("--url", config.url, "RTSP stream URL")
        ->required();

    analyze_cmd
        ->add_option("--duration", config.duration_seconds, "Analysis duration in seconds")
        ->default_val(10);

    analyze_cmd
        ->add_option("--output", config.output_path, "Output JSON report path")
        ->default_val("report.json");

    CLI11_PARSE(app, argc, argv);

    try {
        if (*analyze_cmd) 
        {
            const auto parsed_url = rtsi::RtspUrl::parse(config.url);

            std::cout << "Parsed RTSP source: "
                    << parsed_url.host << ":"
                    << parsed_url.port
                    << parsed_url.path << '\n';
            rtsi::JsonReportWriter writer;
            writer.write_dummy_report(config);

            std::cout << "Report written to: " << config.output_path << '\n';
            return 0;
        }

        std::cout << app.help() << '\n';
        return 0;

    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }
}