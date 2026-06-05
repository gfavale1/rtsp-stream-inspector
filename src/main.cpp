#include "rtsi/app/AnalyzerConfig.hpp"
#include "rtsi/app/StreamAnalyzer.hpp"
#include "rtsi/metrics/AnomalyDetector.hpp"
//#include "rtsi/net/UdpSocket.hpp"
#include "rtsi/report/AnalysisReport.hpp"
#include "rtsi/report/JsonReportWriter.hpp"
#include "rtsi/report/MarkdownReportWriter.hpp"
#include "rtsi/rtsp/InterleavedFrameReader.hpp"
#include "rtsi/rtsp/RtspClient.hpp"
#include "rtsi/rtsp/RtspUrl.hpp"
#include "rtsi/rtsp/SdpParser.hpp"

#include <CLI/CLI.hpp>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <exception>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

bool starts_with_case_insensitive(const std::string& value,
                                  const std::string& prefix) {
  if (value.size() < prefix.size()) {
    return false;
  }

  for (std::size_t i = 0; i < prefix.size(); ++i) {
    const auto left = static_cast<unsigned char>(value[i]);
    const auto right = static_cast<unsigned char>(prefix[i]);

    if (std::tolower(left) != std::tolower(right)) {
      return false;
    }
  }

  return true;
}

std::string sanitize_rtsp_message_for_log(const std::string& message) {
  std::string sanitized;
  std::size_t offset = 0;

  while (offset < message.size()) {
    const auto line_end = message.find_first_of("\r\n", offset);

    std::string line;
    std::string line_ending;

    if (line_end == std::string::npos) {
      line = message.substr(offset);
      offset = message.size();
    } else {
      line = message.substr(offset, line_end - offset);

      if (message[line_end] == '\r' && line_end + 1 < message.size() &&
          message[line_end + 1] == '\n') {
        line_ending = "\r\n";
        offset = line_end + 2;
      } else {
        line_ending = message.substr(line_end, 1);
        offset = line_end + 1;
      }
    }

    if (starts_with_case_insensitive(line, "Authorization:")) {
      sanitized += "Authorization: <redacted>";
    } else {
      sanitized += line;
    }

    sanitized += line_ending;
  }

  return sanitized;
}

void print_response_summary(const rtsi::RtspResponse &response) {
  std::cout << "Status: " << response.status_code() << " "
            << response.reason_phrase() << '\n';

  if (response.cseq().has_value()) {
    std::cout << "CSeq: " << response.cseq().value() << '\n';
  }

  if (response.header("Content-Type").has_value()) {
    std::cout << "Content-Type: " << response.header("Content-Type").value()
              << '\n';
  }

  if (response.content_length().has_value()) {
    std::cout << "Content-Length: " << response.content_length().value()
              << '\n';
  }

  if (response.session_id().has_value()) {
    std::cout << "Session: " << response.session_id().value() << '\n';
  }
}

void print_rtp_parser_stats(const rtsi::RtpStatsSnapshot &stats) {
  std::cout << "\n--- RTP PARSER STATS ---\n";
  std::cout << "Packets received: " << stats.packets_received << '\n';
  std::cout << "Packets lost estimated: " << stats.packets_lost << '\n';
  std::cout << "Out-of-order packets: " << stats.out_of_order_packets << '\n';
  std::cout << "Loss rate: " << stats.loss_rate() << '\n';
  std::cout << "Total RTP bytes: " << stats.total_rtp_bytes << '\n';
  std::cout << "Total RTP payload bytes: " << stats.total_payload_bytes << '\n';

  if (stats.first_sequence_number.has_value()) {
    std::cout << "First sequence number: "
              << stats.first_sequence_number.value() << '\n';
  }

  if (stats.last_sequence_number.has_value()) {
    std::cout << "Last sequence number: " << stats.last_sequence_number.value()
              << '\n';
  }

  if (stats.last_timestamp.has_value()) {
    std::cout << "Last RTP timestamp: " << stats.last_timestamp.value()
              << '\n';
  }

  if (stats.payload_type.has_value()) {
    std::cout << "Payload type: "
              << static_cast<int>(stats.payload_type.value()) << '\n';
  }

  if (stats.ssrc.has_value()) {
    std::cout << "SSRC: " << stats.ssrc.value() << '\n';
  }
}

void print_h264_nal_stats(const rtsi::H264AnalysisSnapshot &stats) {
  std::cout << "\n--- H264 NAL STATS ---\n";
  std::cout << "NAL units seen: " << stats.nal_units_seen << '\n';
  std::cout << "SPS: " << stats.sps_count << '\n';
  std::cout << "PPS: " << stats.pps_count << '\n';
  std::cout << "IDR slices: " << stats.idr_count << '\n';
  std::cout << "Non-IDR slices: " << stats.non_idr_count << '\n';
  std::cout << "FU-A packets: " << stats.fu_a_count << '\n';
  std::cout << "FU-A starts: " << stats.fu_a_start_count << '\n';
  std::cout << "FU-A ends: " << stats.fu_a_end_count << '\n';
  std::cout << "Unknown NAL units: " << stats.unknown_count << '\n';
}

std::string finding_label(const std::string& severity) {
  if (severity == "ok") {
    return "[OK]";
  }

  if (severity == "warning") {
    return "[WARN]";
  }

  if (severity == "critical") {
    return "[CRITICAL]";
  }

  return "[INFO]";
}

void print_findings(const std::vector<rtsi::ReportFinding>& findings) {
  std::cout << "\n--- FINDINGS ---\n";

  for (const auto& finding : findings) {
    std::cout << finding_label(finding.severity) << ' '
              << finding.message << '\n';
  }
}

} // namespace

int main(int argc, char **argv) {
  CLI::App app{"Low-level RTSP/RTP stream inspection tool"};

  std::string analyze_url;
  int analyze_timeout_ms = 3000;
  int analyze_frame_count = 300;
  int analyze_packet_log_limit = 20;
  std::string analyze_output_path;
  std::string analyze_markdown_output_path;

  auto analyze_cmd = app.add_subcommand(
      "analyze", "Analyze an RTSP stream and generate RTP/H264 metrics");

  analyze_cmd->add_option("--url", analyze_url, "RTSP stream URL")
      ->required();

  analyze_cmd
      ->add_option("--timeout-ms", analyze_timeout_ms,
                   "TCP timeout in milliseconds")
      ->default_val(3000);

  analyze_cmd
      ->add_option("--frames", analyze_frame_count,
                   "Number of interleaved RTP/RTCP frames to read")
      ->default_val(300);

  analyze_cmd
      ->add_option("--packet-log-limit", analyze_packet_log_limit,
                   "Maximum number of interleaved frames to print in detail")
      ->default_val(20);

  analyze_cmd
      ->add_option("--output", analyze_output_path,
                   "Optional JSON report output path");

  analyze_cmd
      ->add_option("--markdown", analyze_markdown_output_path,
                   "Optional Markdown report output path");

  std::string probe_url;
  int probe_timeout_ms = 3000;
  int probe_frame_count = 300;
  int packet_log_limit = 20;
  std::string probe_output_path;
  std::string probe_markdown_output_path;

  auto probe_cmd = app.add_subcommand(
      "probe", "Send RTSP OPTIONS, DESCRIBE and SETUP requests");

  probe_cmd->add_option("--url", probe_url, "RTSP stream URL")->required();

  probe_cmd
      ->add_option("--timeout-ms", probe_timeout_ms,
                   "TCP timeout in milliseconds")
      ->default_val(3000);

  probe_cmd
      ->add_option("--frames", probe_frame_count,
                   "Number of interleaved RTP/RTCP frames to read")
      ->default_val(300);

  probe_cmd
      ->add_option("--packet-log-limit", packet_log_limit,
                   "Maximum number of interleaved frames to print in detail")
      ->default_val(20);

  probe_cmd
      ->add_option("--output", probe_output_path,
                   "Optional JSON report output path");

  probe_cmd
      ->add_option("--markdown", probe_markdown_output_path,
                   "Optional Markdown report output path");

  CLI11_PARSE(app, argc, argv);

  if (packet_log_limit < 0) {
    packet_log_limit = 0;
  }

  if (analyze_packet_log_limit < 0) {
    analyze_packet_log_limit = 0;
  }

  try {
    if (*analyze_cmd || *probe_cmd) {
      const bool using_analyze_command = static_cast<bool>(*analyze_cmd);

      const auto& selected_url =
          using_analyze_command ? analyze_url : probe_url;
      const int selected_timeout_ms =
          using_analyze_command ? analyze_timeout_ms : probe_timeout_ms;
      const int selected_frame_count =
          using_analyze_command ? analyze_frame_count : probe_frame_count;
      const int selected_packet_log_limit =
          using_analyze_command ? analyze_packet_log_limit : packet_log_limit;
      const auto& selected_output_path =
          using_analyze_command ? analyze_output_path : probe_output_path;
      const auto& selected_markdown_output_path =
          using_analyze_command ? analyze_markdown_output_path
                                : probe_markdown_output_path;

      const auto parsed_url = rtsi::RtspUrl::parse(selected_url);

      std::cout << "Connecting to " << parsed_url.host << ":" << parsed_url.port
                << "...\n";

      rtsi::RtspClient client(parsed_url);
      client.connect(selected_timeout_ms);

      const auto request_uri = client.request_uri();

      const auto options_exchange = client.options();

      std::cout << "\n--- RTSP OPTIONS REQUEST ---\n";
      std::cout << sanitize_rtsp_message_for_log(
          options_exchange.serialized_request);

      std::cout << "\n--- RTSP OPTIONS RESPONSE ---\n";
      print_response_summary(options_exchange.response);

      if (!options_exchange.response.is_success()) {
        std::cout << "\nRaw response:\n" << options_exchange.raw_response << '\n';
        return 1;
      }

      const auto describe_result = client.describe_with_basic_auth_retry();

      std::cout << "\n--- RTSP DESCRIBE REQUEST ---\n";
      std::cout << sanitize_rtsp_message_for_log(
          describe_result.initial_exchange.serialized_request);

      std::cout << "\n--- RTSP DESCRIBE RESPONSE ---\n";
      print_response_summary(describe_result.initial_exchange.response);

      if (describe_result.used_basic_auth &&
          describe_result.authenticated_exchange.has_value()) {
        std::cout << "\nDESCRIBE requires authentication. "
                  << "Retrying with Basic authentication...\n";

        std::cout << "\n--- RTSP DESCRIBE AUTHENTICATED REQUEST ---\n";
        std::cout << sanitize_rtsp_message_for_log(
            describe_result.authenticated_exchange->serialized_request);

        std::cout << "\n--- RTSP DESCRIBE AUTHENTICATED RESPONSE ---\n";
        print_response_summary(
            describe_result.authenticated_exchange->response);
      }

      const auto describe_response = describe_result.response;

      if (!describe_response.is_success()) {
        std::cout << "\nRaw response:\n" << describe_result.raw_response << '\n';
        return 1;
      }

      std::cout << "\n--- SDP BODY ---\n";
      std::cout << describe_response.body() << '\n';

      const auto sdp = rtsi::SdpParser::parse(describe_response.body());

      std::cout << "\n--- SDP SUMMARY ---\n";
      std::cout << "Session: " << sdp.session_name << '\n';
      std::cout << "Tracks: " << sdp.tracks.size() << '\n';

      const auto video_track = sdp.first_video_track();

      if (!video_track.has_value()) {
        std::cout << "No video track found in SDP.\n";
        return 1;
      }

      std::cout << "Video codec: " << video_track->codec << '\n';
      std::cout << "Video payload type: " << video_track->payload_type << '\n';
      std::cout << "Video clock rate: " << video_track->clock_rate << '\n';
      std::cout << "Video control: " << video_track->control << '\n';

      const auto video_setup_uri =
          rtsi::build_control_uri(request_uri, video_track->control);

      std::cout << "Video SETUP URI: " << video_setup_uri << '\n';

      const auto audio_track = sdp.first_audio_track();

      if (audio_track.has_value()) {
        std::cout << "Audio codec: " << audio_track->codec << '\n';
        std::cout << "Audio payload type: " << audio_track->payload_type
                  << '\n';
        std::cout << "Audio clock rate: " << audio_track->clock_rate << '\n';
        std::cout << "Audio control: " << audio_track->control << '\n';
      }

      {
        const auto setup_result = client.setup_interleaved(video_setup_uri);
        const auto& setup_exchange = setup_result.exchange;

        std::cout << "\n--- RTSP SETUP REQUEST ---\n";
        std::cout << sanitize_rtsp_message_for_log(
            setup_exchange.serialized_request);

        std::cout << "\n--- RTSP SETUP RESPONSE ---\n";
        print_response_summary(setup_exchange.response);

        if (setup_exchange.response.header("Transport").has_value()) {
          std::cout << "Transport: "
                    << setup_exchange.response.header("Transport").value()
                    << '\n';
        }

        if (!setup_exchange.response.is_success()) {
          std::cout << "\nRaw response:\n" << setup_exchange.raw_response << '\n';
          return 1;
        }

        if (setup_result.session_id.empty()) {
          std::cout
              << "SETUP succeeded but no RTSP Session header was found.\n";
          return 1;
        }

        std::cout << "\nRTSP session established successfully.\n";
        std::cout << "Session ID: " << setup_result.session_id << '\n';

        const auto session_id = setup_result.session_id;

        const auto play_exchange = client.play(session_id);

        std::cout << "\n--- RTSP PLAY REQUEST ---\n";
        std::cout << sanitize_rtsp_message_for_log(
            play_exchange.serialized_request);

        std::cout << "\n--- RTSP PLAY RESPONSE ---\n";
        print_response_summary(play_exchange.response);

        if (!play_exchange.response.is_success()) {
          std::cout << "\nRaw response:\n" << play_exchange.raw_response << '\n';
          return 1;
        }

        rtsi::InterleavedFrameReader interleaved_reader(
            client.socket(), play_exchange.response.body());

        std::cout << "\nReceiving RTP interleaved frames over TCP...\n";

        rtsi::StreamAnalyzer stream_analyzer;

        rtsi::StreamAnalyzerConfig stream_config;
        stream_config.frame_count = selected_frame_count;
        stream_config.packet_log_limit = selected_packet_log_limit;
        stream_config.rtp_clock_rate = video_track->clock_rate;

        const auto analysis_result = stream_analyzer.analyze(
            interleaved_reader,
            stream_config,
            [](const rtsi::StreamAnalyzerPacketLogEntry& entry) {
              std::cout << "Interleaved frame " << entry.frame_index
                        << " channel=" << static_cast<int>(entry.channel)
                        << " payload_size=" << entry.payload_size << " bytes";

              if (entry.type == rtsi::InterleavedFrameType::Rtp) {
                std::cout << " type=RTP";
              } else if (entry.type == rtsi::InterleavedFrameType::Rtcp) {
                std::cout << " type=RTCP";
              } else {
                std::cout << " type=unknown";
              }

              std::cout << '\n';
            });

        if (analysis_result.stop_reason.has_value()) {
          std::cout << "Interleaved RTP receive stopped: "
                    << analysis_result.stop_reason.value() << '\n';
        }

        const auto packet_log_limit_count =
            static_cast<std::size_t>(selected_packet_log_limit);

        if (analysis_result.interleaved_frames_received >
            packet_log_limit_count) {
          std::cout << "Packet detail log limited to first "
                    << selected_packet_log_limit
                    << " frames out of "
                    << analysis_result.interleaved_frames_received
                    << " captured frames.\n";
        }

        const auto metrics = analysis_result.metrics;

        auto report = analysis_result.report;
        report.source.host = parsed_url.host;
        report.source.port = parsed_url.port;
        report.source.path = parsed_url.path;
        report.source.transport = "rtp_interleaved_tcp";
        report.video.codec = video_track->codec;
        report.video.payload_type = video_track->payload_type;
        report.video.clock_rate = video_track->clock_rate;
        report.video.control = video_track->control;

        std::cout << "\n--- RTP INTERLEAVED RECEIVE SUMMARY ---\n";
        std::cout << "RTP frames received: "
                  << analysis_result.rtp_frames_received << '\n';
        std::cout << "RTCP frames received: "
                  << analysis_result.rtcp_frames_received << '\n';
        std::cout << "Total payload bytes: "
                  << analysis_result.total_payload_bytes << '\n';

        const auto stats = metrics.rtp;
        print_rtp_parser_stats(stats);

        const auto h264_stats = metrics.h264;
        print_h264_nal_stats(h264_stats);

        const auto stream_metrics = metrics.stream;

        std::cout << "\n--- STREAM METRICS ---\n";
        std::cout << std::fixed << std::setprecision(3);
        std::cout << "Capture duration: " << stream_metrics.capture_seconds
                  << " s\n";
        std::cout << "RTP bitrate: " << stream_metrics.rtp_bitrate_mbps
                  << " Mbps\n";
        std::cout << "H264 payload bitrate: "
                  << stream_metrics.h264_payload_bitrate_mbps << " Mbps\n";
        std::cout << "RTP packets/sec: "
                  << stream_metrics.rtp_packets_per_second << " pps\n";
        std::cout << "Average RTP packet size: "
                  << stream_metrics.average_rtp_packet_size << " bytes\n";
        std::cout << "Average H264 payload size: "
                  << stream_metrics.average_h264_payload_size << " bytes\n";

        const auto rtp_quality = report.rtp_quality;

        std::cout << "\n--- RTP QUALITY METRICS ---\n";
        std::cout << "Packets observed for jitter: "
                  << rtp_quality.packets_observed << '\n';
        std::cout << "RTP jitter: " << rtp_quality.jitter_ms << " ms\n";
        std::cout << "Average inter-arrival gap: "
                  << rtp_quality.average_interarrival_gap_ms << " ms\n";
        std::cout << "Max inter-arrival gap: "
                  << rtp_quality.max_interarrival_gap_ms << " ms\n";

        bool teardown_success = false;

        try {
          const auto teardown_exchange = client.teardown(session_id);

          std::cout << "\n--- RTSP TEARDOWN REQUEST ---\n";
          std::cout << sanitize_rtsp_message_for_log(
              teardown_exchange.serialized_request);

          std::cout << "\n--- RTSP TEARDOWN RESPONSE ---\n";
          print_response_summary(teardown_exchange.response);

          teardown_success = teardown_exchange.response.is_success();

          if (!teardown_exchange.response.is_success()) {
            std::cout << "\nRaw response:\n"
                      << teardown_exchange.raw_response << '\n';
          }

        } catch (const std::exception &ex) {
          std::cout << "\nRTSP TEARDOWN response could not be read: "
                    << ex.what() << '\n';
        }

        report.teardown_success = teardown_success;

        const rtsi::AnomalyDetector anomaly_detector;
        report.findings = anomaly_detector.analyze(report);
        print_findings(report.findings);

        if (!selected_output_path.empty()) {
          rtsi::JsonReportWriter writer;
          writer.write_report(report, selected_output_path);
          std::cout << "\nJSON report written to: " << selected_output_path << '\n';
        }

        if (!selected_markdown_output_path.empty()) {
          rtsi::MarkdownReportWriter writer;
          writer.write_report(report, selected_markdown_output_path);
          std::cout << "Markdown report written to: "
                    << selected_markdown_output_path << '\n';
        }
      }

      return 0;
    }

    std::cout << app.help() << '\n';
    return 0;

  } catch (const std::exception &ex) {
    std::cerr << "Error: " << ex.what() << '\n';
    return 1;
  }
}
