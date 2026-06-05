#pragma once

#include "rtsi/net/TcpSocket.hpp"
#include "rtsi/rtsp/RtspRequest.hpp"
#include "rtsi/rtsp/RtspResponse.hpp"
#include "rtsi/rtsp/RtspUrl.hpp"

#include <cstdint>
#include <optional>
#include <string>

namespace rtsi {

struct RtspExchange {
  RtspResponse response;
  std::string raw_response;
  std::string serialized_request;
};

struct RtspDescribeResult {
  RtspExchange initial_exchange;
  std::optional<RtspExchange> authenticated_exchange;
  RtspResponse response;
  std::string raw_response;
  std::string serialized_request;
  bool used_basic_auth = false;
};

struct RtspSetupResult {
  RtspExchange exchange;
  std::string session_id;
};

class RtspClient {
public:
  explicit RtspClient(RtspUrl url);

  void connect(int timeout_ms);

  [[nodiscard]] const RtspUrl& url() const noexcept;
  [[nodiscard]] TcpSocket& socket() noexcept;

  [[nodiscard]] std::string request_uri() const;

  [[nodiscard]] RtspExchange options();

  [[nodiscard]] RtspDescribeResult describe_with_basic_auth_retry();

  [[nodiscard]] RtspSetupResult setup_interleaved(
      const std::string& track_uri,
      std::uint8_t rtp_channel = 0,
      std::uint8_t rtcp_channel = 1);

  [[nodiscard]] RtspExchange play(const std::string& session_id);

  [[nodiscard]] RtspExchange teardown(const std::string& session_id);

private:
  RtspUrl url_;
  TcpSocket socket_;
  std::uint32_t next_cseq_ = 1;

  [[nodiscard]] std::uint32_t consume_cseq() noexcept;

  [[nodiscard]] RtspExchange send_and_read_exchange(
      const RtspRequest& request,
      bool skip_interleaved = false);

  [[nodiscard]] std::string send_and_read_raw_response(
      const RtspRequest& request,
      bool skip_interleaved = false);

  void add_basic_auth_if_available(RtspRequest& request) const;
};

} // namespace rtsi
