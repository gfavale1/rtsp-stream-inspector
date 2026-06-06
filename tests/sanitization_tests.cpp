#include "rtsi/core/LogSanitizer.hpp"
#include "rtsi/report/AnalysisReport.hpp"
#include "rtsi/report/JsonReportWriter.hpp"
#include "rtsi/report/MarkdownReportWriter.hpp"
#include "rtsi/rtsp/RtspUrl.hpp"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <string>

namespace {

std::string read_file(const std::filesystem::path& path) {
    std::ifstream input(path);
    return std::string(
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>());
}

} // namespace

TEST_CASE("RTSP log sanitizer redacts Authorization header values", "[security][sanitize]") {
    const std::string raw =
        "DESCRIBE rtsp://example.local/stream RTSP/1.0\r\n"
        "CSeq: 2\r\n"
        "Authorization: Basic ABCDEFG\r\n"
        "\r\n";

    const auto sanitized = rtsi::sanitize_rtsp_message_for_log(raw);
    REQUIRE(sanitized.find("Authorization: <redacted>") != std::string::npos);
    REQUIRE(sanitized.find("ABCDEFG") == std::string::npos);
}

TEST_CASE("RTSP log sanitizer handles Authorization case-insensitively", "[security][sanitize]") {
    const std::string raw =
        "authorization: Basic TOKEN_ONE\n"
        "AUTHORIZATION: Digest username=\"demo\", response=\"TOKEN_TWO\"\n";

    const auto sanitized = rtsi::sanitize_rtsp_message_for_log(raw);
    REQUIRE(sanitized.find("Authorization: <redacted>") != std::string::npos);
    REQUIRE(sanitized.find("TOKEN_ONE") == std::string::npos);
    REQUIRE(sanitized.find("TOKEN_TWO") == std::string::npos);
}

TEST_CASE("RtspUrl request_uri strips userinfo from endpoint", "[security][sanitize]") {
    const auto parsed = rtsi::RtspUrl::parse("rtsp://redact_user:redact_secret@example.local:554/stream1");

    REQUIRE(parsed.has_credentials());
    REQUIRE(parsed.request_uri() == "rtsp://example.local:554/stream1");
    REQUIRE(parsed.request_uri().find("redact_user") == std::string::npos);
    REQUIRE(parsed.request_uri().find("redact_secret") == std::string::npos);
}

TEST_CASE("Report writers do not serialize RTSP userinfo when populated from parsed source fields", "[security][sanitize][report]") {
    const auto parsed = rtsi::RtspUrl::parse("rtsp://redact_user:redact_secret@example.local:554/stream1");

    rtsi::AnalysisReport report;
    report.source.host = parsed.host;
    report.source.port = parsed.port;
    report.source.path = parsed.path;
    report.source.transport = "rtp_interleaved_tcp";
    report.video.codec = "H264";
    report.video.payload_type = 96;
    report.video.clock_rate = 90000;
    report.teardown_success = true;

    const auto base = std::filesystem::temp_directory_path() / "rtsi_sanitization_report_test";
    const auto json_path = base.string() + ".json";
    const auto markdown_path = base.string() + ".md";

    rtsi::JsonReportWriter json_writer;
    json_writer.write_report(report, json_path);

    rtsi::MarkdownReportWriter markdown_writer;
    markdown_writer.write_report(report, markdown_path);

    const auto json = read_file(json_path);
    const auto markdown = read_file(markdown_path);

    REQUIRE(json.find("redact_user") == std::string::npos);
    REQUIRE(json.find("redact_secret") == std::string::npos);
    REQUIRE(markdown.find("redact_user") == std::string::npos);
    REQUIRE(markdown.find("redact_secret") == std::string::npos);

    std::filesystem::remove(json_path);
    std::filesystem::remove(markdown_path);
}
