#include "rtsi/app/AnalyzerConfig.hpp"
#include "rtsi/net/TcpSocket.hpp"
#include "rtsi/report/JsonReportWriter.hpp"
#include "rtsi/rtsp/RtspRequest.hpp"
#include "rtsi/rtsp/RtspUrl.hpp"

#include <CLI/CLI.hpp>

#include <exception>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    CLI::App app{"Low-level RTSP/RTP stream inspection tool"};

    rtsi::AnalyzerConfig analyze_config;

    auto analyze_cmd = app.add_subcommand("analyze", "Analyze an RTSP stream");

    analyze_cmd
        ->add_option("--url", analyze_config.url, "RTSP stream URL")
        ->required();

    analyze_cmd
        ->add_option("--duration", analyze_config.duration_seconds, "Analysis duration in seconds")
        ->default_val(10);

    analyze_cmd
        ->add_option("--output", analyze_config.output_path, "Output JSON report path")
        ->default_val("report.json");

    std::string probe_url;
    int probe_timeout_ms = 3000;

    auto probe_cmd = app.add_subcommand("probe", "Send an RTSP OPTIONS request and print the raw response");

    probe_cmd
        ->add_option("--url", probe_url, "RTSP stream URL")
        ->required();

    probe_cmd
        ->add_option("--timeout-ms", probe_timeout_ms, "TCP timeout in milliseconds")
        ->default_val(3000);

    CLI11_PARSE(app, argc, argv);

    try {
        if (*analyze_cmd) {
            const auto parsed_url = rtsi::RtspUrl::parse(analyze_config.url);

            std::cout << "Parsed RTSP source: "
                      << parsed_url.host << ":"
                      << parsed_url.port
                      << parsed_url.path << '\n';

            rtsi::JsonReportWriter writer;
            writer.write_dummy_report(analyze_config);

            std::cout << "Report written to: " << analyze_config.output_path << '\n';
            return 0;
        }

        if (*probe_cmd) {
            const auto parsed_url = rtsi::RtspUrl::parse(probe_url);

            std::cout << "Connecting to "
                      << parsed_url.host << ":"
                      << parsed_url.port << "...\n";

            rtsi::TcpSocket socket;
            socket.connect_to(parsed_url.host, parsed_url.port, probe_timeout_ms);

            const auto request = rtsi::RtspRequest::options(probe_url, 1);
            const auto serialized_request = request.serialize();

            std::cout << "\n--- RTSP REQUEST ---\n";
            std::cout << serialized_request;

            socket.send_all(serialized_request);

            const auto response = socket.receive_until("\r\n\r\n", 65536);

            std::cout << "\n--- RTSP RESPONSE ---\n";
            std::cout << response << '\n';

            return 0;
        }

        std::cout << app.help() << '\n';
        return 0;

    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }
}