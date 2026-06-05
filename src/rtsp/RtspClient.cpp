#include "rtsi/rtsp/RtspClient.hpp"

#include "rtsi/rtsp/RtspAuth.hpp"
#include "rtsi/rtsp/RtspRequest.hpp"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstddef>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace rtsi {
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

bool iequals(const std::string& lhs, const std::string& rhs) {
  return lhs.size() == rhs.size() &&
         std::equal(lhs.begin(), lhs.end(), rhs.begin(),
                    [](unsigned char a, unsigned char b) {
                      return std::tolower(a) == std::tolower(b);
                    });
}

std::optional<std::size_t>
extract_content_length_from_headers(const std::string& header_block) {
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

std::string read_full_rtsp_response(TcpSocket& socket) {
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

std::string read_rtsp_response_skipping_interleaved(TcpSocket& socket) {
  std::string rolling_prefix;

  while (true) {
    const auto byte = socket.receive_exact(1);

    if (byte.empty()) {
      throw std::runtime_error("Failed to read RTSP response prefix");
    }

    const auto value = static_cast<unsigned char>(byte[0]);

    if (value == 0x24) {
      const auto header = socket.receive_exact(3);

      if (header.size() != 3) {
        throw std::runtime_error(
            "Incomplete interleaved frame header before RTSP response");
      }

      const auto length = static_cast<std::uint16_t>(
          (static_cast<unsigned char>(header[1]) << 8) |
          static_cast<unsigned char>(header[2]));

      if (length > 0) {
        static_cast<void>(socket.receive_exact(length));
      }

      rolling_prefix.clear();
      continue;
    }

    rolling_prefix += byte;

    if (rolling_prefix.size() > 5) {
      rolling_prefix.erase(0, rolling_prefix.size() - 5);
    }

    if (rolling_prefix == "RTSP/") {
      break;
    }
  }

  std::string raw_response = "RTSP/";
  raw_response += socket.receive_until("\r\n\r\n", 65536);

  std::size_t header_end = raw_response.find("\r\n\r\n");
  std::size_t separator_size = 4;

  if (header_end == std::string::npos) {
    header_end = raw_response.find("\n\n");
    separator_size = 2;
  }

  if (header_end == std::string::npos) {
    throw std::runtime_error("Invalid RTSP response: missing header separator");
  }

  const auto header_block = raw_response.substr(0, header_end);
  const auto content_length = extract_content_length_from_headers(header_block);

  if (!content_length.has_value()) {
    return raw_response;
  }

  const std::size_t body_start = header_end + separator_size;

  const std::size_t already_received_body_bytes =
      raw_response.size() > body_start ? raw_response.size() - body_start : 0;

  if (already_received_body_bytes < content_length.value()) {
    const std::size_t missing =
        content_length.value() - already_received_body_bytes;

    raw_response += socket.receive_exact(missing);
  }

  return raw_response;
}

} // namespace

RtspClient::RtspClient(RtspUrl url) : url_(std::move(url)) {}

void RtspClient::connect(int timeout_ms) {
  socket_.connect_to(url_.host, url_.port, timeout_ms);
}

const RtspUrl& RtspClient::url() const noexcept { return url_; }

TcpSocket& RtspClient::socket() noexcept { return socket_; }

std::string RtspClient::request_uri() const { return url_.request_uri(); }

std::uint32_t RtspClient::consume_cseq() noexcept { return next_cseq_++; }

RtspExchange RtspClient::options() {
  const auto request = RtspRequest::options(request_uri(), consume_cseq());
  return send_and_read_exchange(request);
}

RtspDescribeResult RtspClient::describe_with_basic_auth_retry() {
  auto request = RtspRequest::describe(request_uri(), consume_cseq());
  auto initial_exchange = send_and_read_exchange(request);

  RtspDescribeResult result;
  result.initial_exchange = initial_exchange;
  result.response = initial_exchange.response;
  result.raw_response = initial_exchange.raw_response;
  result.serialized_request = initial_exchange.serialized_request;

  if (initial_exchange.response.status_code() == 401 &&
      url_.has_credentials()) {
    auto authenticated_request =
        RtspRequest::describe(request_uri(), consume_cseq());
    add_basic_auth_if_available(authenticated_request);

    auto authenticated_exchange =
        send_and_read_exchange(authenticated_request);

    result.authenticated_exchange = authenticated_exchange;
    result.response = authenticated_exchange.response;
    result.raw_response = authenticated_exchange.raw_response;
    result.serialized_request = authenticated_exchange.serialized_request;
    result.used_basic_auth = true;
  }

  return result;
}

RtspSetupResult RtspClient::setup_interleaved(const std::string& track_uri,
                                              std::uint8_t rtp_channel,
                                              std::uint8_t rtcp_channel) {
  const std::string transport =
      "RTP/AVP/TCP;unicast;interleaved=" +
      std::to_string(static_cast<int>(rtp_channel)) + "-" +
      std::to_string(static_cast<int>(rtcp_channel));

  auto request = RtspRequest::setup(track_uri, consume_cseq(), transport);
  add_basic_auth_if_available(request);

  auto exchange = send_and_read_exchange(request);

  RtspSetupResult result;
  result.exchange = exchange;

  if (exchange.response.session_id().has_value()) {
    result.session_id = exchange.response.session_id().value();
  }

  return result;
}

RtspExchange RtspClient::play(const std::string& session_id) {
  auto request = RtspRequest::play(request_uri(), consume_cseq(), session_id);
  request.set_header("Range", "npt=0.000-");
  add_basic_auth_if_available(request);

  return send_and_read_exchange(request);
}

RtspExchange RtspClient::teardown(const std::string& session_id) {
  auto request = RtspRequest::teardown(request_uri(), consume_cseq(), session_id);
  add_basic_auth_if_available(request);

  return send_and_read_exchange(request, true);
}

RtspExchange RtspClient::send_and_read_exchange(const RtspRequest& request,
                                                bool skip_interleaved) {
  RtspExchange exchange;
  exchange.serialized_request = request.serialize();
  exchange.raw_response = send_and_read_raw_response(request, skip_interleaved);
  exchange.response = RtspResponse::parse(exchange.raw_response);

  return exchange;
}

std::string RtspClient::send_and_read_raw_response(const RtspRequest& request,
                                                   bool skip_interleaved) {
  const auto serialized_request = request.serialize();
  socket_.send_all(serialized_request);

  if (skip_interleaved) {
    return read_rtsp_response_skipping_interleaved(socket_);
  }

  return read_full_rtsp_response(socket_);
}

void RtspClient::add_basic_auth_if_available(RtspRequest& request) const {
  if (!url_.has_credentials()) {
    return;
  }

  request.set_header("Authorization", make_basic_authorization_value(
                                      url_.username, url_.password));
}

} // namespace rtsi
