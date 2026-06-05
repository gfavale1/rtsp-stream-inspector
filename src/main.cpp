#include "rtsi/app/AnalyzerConfig.hpp"
#include "rtsi/net/TcpSocket.hpp"
//#include "rtsi/net/UdpSocket.hpp"
#include "rtsi/report/JsonReportWriter.hpp"
#include "rtsi/rtsp/RtspAuth.hpp"
#include "rtsi/rtsp/RtspRequest.hpp"
#include "rtsi/rtsp/RtspResponse.hpp"
#include "rtsi/rtsp/RtspUrl.hpp"
#include "rtsi/rtsp/SdpParser.hpp"

#include <CLI/CLI.hpp>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <exception>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

std::string trim(std::string value) {
  while (!value.empty() &&
         std::isspace(static_cast<unsigned char>(value.front())) != 0) {
    value.erase(value.begin());
  }

  while (!value.empty() &&
         std::isspace(static_cast<unsigned char>(value.back())) != 0) {
    value.pop_back();
  }

  return value;
}

bool iequals(const std::string &lhs, const std::string &rhs) {
  return lhs.size() == rhs.size() &&
         std::equal(lhs.begin(), lhs.end(), rhs.begin(),
                    [](unsigned char a, unsigned char b) {
                      return std::tolower(a) == std::tolower(b);
                    });
}

std::optional<std::size_t>
extract_content_length_from_headers(const std::string &header_block) {
  std::istringstream stream(header_block);
  std::string line;

  while (std::getline(stream, line)) {
    if (!line.empty() && line.back() == '\r') {
      line.pop_back();
    }

    const auto colon = line.find(':');

    if (colon == std::string::npos) {
      continue;
    }

    const auto name = trim(line.substr(0, colon));
    const auto value = trim(line.substr(colon + 1));

    if (iequals(name, "Content-Length")) {
      return static_cast<std::size_t>(std::stoull(value));
    }
  }

  return std::nullopt;
}

std::string read_full_rtsp_response(rtsi::TcpSocket &socket) {
  std::string raw = socket.receive_until("\r\n\r\n", 65536);

  std::size_t header_end = raw.find("\r\n\r\n");
  std::size_t separator_size = 4;

  if (header_end == std::string::npos) {
    header_end = raw.find("\n\n");
    separator_size = 2;
  }

  if (header_end == std::string::npos) {
    throw std::runtime_error("Invalid RTSP response: missing header separator");
  }

  const auto header_block = raw.substr(0, header_end);
  const auto content_length = extract_content_length_from_headers(header_block);

  if (!content_length.has_value()) {
    return raw;
  }

  const std::size_t body_start = header_end + separator_size;

  const std::size_t already_received_body_bytes =
      raw.size() > body_start ? raw.size() - body_start : 0;

  if (already_received_body_bytes < content_length.value()) {
    const std::size_t missing =
        content_length.value() - already_received_body_bytes;

    raw += socket.receive_exact(missing);
  }

  return raw;
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


struct InterleavedFrame {
  std::uint8_t channel = 0;
  std::vector<std::uint8_t> payload;
};

std::string read_bytes(rtsi::TcpSocket &socket, std::string &pending,
                       std::size_t byte_count) {
  std::string result;
  result.reserve(byte_count);

  if (!pending.empty()) {
    const std::size_t take = std::min(byte_count, pending.size());

    result += pending.substr(0, take);
    pending.erase(0, take);
  }

  if (result.size() < byte_count) {
    result += socket.receive_exact(byte_count - result.size());
  }

  return result;
}

InterleavedFrame read_interleaved_frame(rtsi::TcpSocket &socket,
                                        std::string &pending) {
  std::string marker;

  do {
    marker = read_bytes(socket, pending, 1);
  } while (!marker.empty() &&
           static_cast<unsigned char>(marker[0]) != 0x24);

  if (marker.empty()) {
    throw std::runtime_error("Failed to read RTSP interleaved marker");
  }

  const auto header = read_bytes(socket, pending, 3);

  if (header.size() != 3) {
    throw std::runtime_error("Incomplete RTSP interleaved header");
  }

  InterleavedFrame frame;

  frame.channel = static_cast<std::uint8_t>(header[0]);

  const auto length = static_cast<std::uint16_t>(
      (static_cast<unsigned char>(header[1]) << 8) |
      static_cast<unsigned char>(header[2]));

  const auto payload = read_bytes(socket, pending, length);

  if (payload.size() != length) {
    throw std::runtime_error("Incomplete RTSP interleaved payload");
  }

  frame.payload.assign(payload.begin(), payload.end());

  return frame;
}

} // namespace

int main(int argc, char **argv) {
  CLI::App app{"Low-level RTSP/RTP stream inspection tool"};

  rtsi::AnalyzerConfig analyze_config;

  auto analyze_cmd = app.add_subcommand("analyze", "Analyze an RTSP stream");

  analyze_cmd->add_option("--url", analyze_config.url, "RTSP stream URL")
      ->required();

  analyze_cmd
      ->add_option("--duration", analyze_config.duration_seconds,
                   "Analysis duration in seconds")
      ->default_val(10);

  analyze_cmd
      ->add_option("--output", analyze_config.output_path,
                   "Output JSON report path")
      ->default_val("report.json");

  std::string probe_url;
  int probe_timeout_ms = 3000;

  auto probe_cmd = app.add_subcommand(
      "probe", "Send RTSP OPTIONS, DESCRIBE and SETUP requests");

  probe_cmd->add_option("--url", probe_url, "RTSP stream URL")->required();

  probe_cmd
      ->add_option("--timeout-ms", probe_timeout_ms,
                   "TCP timeout in milliseconds")
      ->default_val(3000);

  CLI11_PARSE(app, argc, argv);

  try {
    if (*analyze_cmd) {
      const auto parsed_url = rtsi::RtspUrl::parse(analyze_config.url);

      std::cout << "Parsed RTSP source: " << parsed_url.host << ":"
                << parsed_url.port << parsed_url.path << '\n';

      rtsi::JsonReportWriter writer;
      writer.write_dummy_report(analyze_config);

      std::cout << "Report written to: " << analyze_config.output_path << '\n';

      return 0;
    }

    if (*probe_cmd) {
      const auto parsed_url = rtsi::RtspUrl::parse(probe_url);
      const auto request_uri = parsed_url.request_uri();

      std::cout << "Connecting to " << parsed_url.host << ":" << parsed_url.port
                << "...\n";

      rtsi::TcpSocket socket;
      socket.connect_to(parsed_url.host, parsed_url.port, probe_timeout_ms);

      {
        const auto request = rtsi::RtspRequest::options(request_uri, 1);
        const auto serialized_request = request.serialize();

        std::cout << "\n--- RTSP OPTIONS REQUEST ---\n";
        std::cout << serialized_request;

        socket.send_all(serialized_request);

        const auto raw_response = read_full_rtsp_response(socket);
        const auto response = rtsi::RtspResponse::parse(raw_response);

        std::cout << "\n--- RTSP OPTIONS RESPONSE ---\n";
        print_response_summary(response);

        if (!response.is_success()) {
          std::cout << "\nRaw response:\n" << raw_response << '\n';
          return 1;
        }
      }

      rtsi::RtspResponse describe_response;
      std::string raw_describe_response;

      {
        auto request = rtsi::RtspRequest::describe(request_uri, 2);
        const auto serialized_request = request.serialize();

        std::cout << "\n--- RTSP DESCRIBE REQUEST ---\n";
        std::cout << serialized_request;

        socket.send_all(serialized_request);

        raw_describe_response = read_full_rtsp_response(socket);
        describe_response = rtsi::RtspResponse::parse(raw_describe_response);

        std::cout << "\n--- RTSP DESCRIBE RESPONSE ---\n";
        print_response_summary(describe_response);

        if (describe_response.status_code() == 401 &&
            parsed_url.has_credentials()) {
          std::cout << "\nDESCRIBE requires authentication. "
                    << "Retrying with Basic authentication...\n";

          auto authenticated_request =
              rtsi::RtspRequest::describe(request_uri, 3);

          authenticated_request.set_header(
              "Authorization", rtsi::make_basic_authorization_value(
                                   parsed_url.username, parsed_url.password));

          const auto authenticated_serialized_request =
              authenticated_request.serialize();

          std::cout << "\n--- RTSP DESCRIBE AUTHENTICATED REQUEST ---\n";
          std::cout << authenticated_serialized_request;

          socket.send_all(authenticated_serialized_request);

          raw_describe_response = read_full_rtsp_response(socket);
          describe_response = rtsi::RtspResponse::parse(raw_describe_response);

          std::cout << "\n--- RTSP DESCRIBE AUTHENTICATED RESPONSE ---\n";
          print_response_summary(describe_response);
        }

        if (!describe_response.is_success()) {
          std::cout << "\nRaw response:\n" << raw_describe_response << '\n';
          return 1;
        }

        std::cout << "\n--- SDP BODY ---\n";
        std::cout << describe_response.body() << '\n';
      }

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
        //rtsi::UdpSocket rtp_socket;
        //rtp_socket.bind_to(5000, 3000);

        //std::cout << "\nListening for RTP packets on UDP port 5000...\n";
        const std::string transport ="RTP/AVP/TCP;unicast;interleaved=0-1";

        auto setup_request =
            rtsi::RtspRequest::setup(video_setup_uri, 4, transport);

        if (parsed_url.has_credentials()) {
          setup_request.set_header(
              "Authorization", rtsi::make_basic_authorization_value(
                                   parsed_url.username, parsed_url.password));
        }

        const auto serialized_setup_request = setup_request.serialize();

        std::cout << "\n--- RTSP SETUP REQUEST ---\n";
        std::cout << serialized_setup_request;

        socket.send_all(serialized_setup_request);

        const auto raw_setup_response = read_full_rtsp_response(socket);
        const auto setup_response =
            rtsi::RtspResponse::parse(raw_setup_response);

        std::cout << "\n--- RTSP SETUP RESPONSE ---\n";
        print_response_summary(setup_response);

        if (setup_response.header("Transport").has_value()) {
          std::cout << "Transport: "
                    << setup_response.header("Transport").value() << '\n';
        }

        if (!setup_response.is_success()) {
          std::cout << "\nRaw response:\n" << raw_setup_response << '\n';
          return 1;
        }

        if (!setup_response.session_id().has_value()) {
          std::cout
              << "SETUP succeeded but no RTSP Session header was found.\n";
          return 1;
        }

        std::cout << "\nRTSP session established successfully.\n";
        std::cout << "Session ID: " << setup_response.session_id().value()
                  << '\n';

        const auto session_id = setup_response.session_id().value();

        auto play_request = rtsi::RtspRequest::play(request_uri, 5, session_id);

        play_request.set_header("Range", "npt=0.000-");

        if (parsed_url.has_credentials()) {
          play_request.set_header(
              "Authorization", rtsi::make_basic_authorization_value(
                                   parsed_url.username, parsed_url.password));
        }

        const auto serialized_play_request = play_request.serialize();

        std::cout << "\n--- RTSP PLAY REQUEST ---\n";
        std::cout << serialized_play_request;

        socket.send_all(serialized_play_request);

        const auto raw_play_response = read_full_rtsp_response(socket);
        const auto play_response = rtsi::RtspResponse::parse(raw_play_response);

        std::cout << "\n--- RTSP PLAY RESPONSE ---\n";
        print_response_summary(play_response);

        if (!play_response.is_success()) {
          std::cout << "\nRaw response:\n" << raw_play_response << '\n';
          return 1;
        }

        std::string pending_tcp_data = play_response.body();

        std::cout << "\nReceiving RTP interleaved frames over TCP...\n";

        std::size_t rtp_frames_received = 0;
        std::size_t rtcp_frames_received = 0;
        std::size_t total_payload_bytes = 0;

        for (int i = 0; i < 20; ++i) {
          try {
            const auto frame = read_interleaved_frame(socket, pending_tcp_data);

            total_payload_bytes += frame.payload.size();

            if (frame.channel == 0) {
              ++rtp_frames_received;
            } else if (frame.channel == 1) {
              ++rtcp_frames_received;
            }

            std::cout << "Interleaved frame " << (i + 1)
                      << " channel=" << static_cast<int>(frame.channel)
                      << " payload_size=" << frame.payload.size() << " bytes";

            if (frame.channel == 0) {
              std::cout << " type=RTP";
            } else if (frame.channel == 1) {
              std::cout << " type=RTCP";
            } else {
              std::cout << " type=unknown";
            }

            std::cout << '\n';

          } catch (const std::exception &ex) {
            std::cout << "Interleaved RTP receive stopped: " << ex.what()
                      << '\n';
            break;
          }
        }

        std::cout << "\n--- RTP INTERLEAVED RECEIVE SUMMARY ---\n";
        std::cout << "RTP frames received: " << rtp_frames_received << '\n';
        std::cout << "RTCP frames received: " << rtcp_frames_received << '\n';
        std::cout << "Total payload bytes: " << total_payload_bytes << '\n';
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