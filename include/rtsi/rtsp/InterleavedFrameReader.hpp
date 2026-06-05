#pragma once

#include "rtsi/net/TcpSocket.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace rtsi {

struct InterleavedFrame {
  std::uint8_t channel = 0;
  std::vector<std::uint8_t> payload;

  [[nodiscard]] bool is_rtp() const noexcept;
  [[nodiscard]] bool is_rtcp() const noexcept;
};

class InterleavedFrameReader {
public:
  explicit InterleavedFrameReader(TcpSocket& socket,
                                  std::string pending_data = {});

  [[nodiscard]] InterleavedFrame read_frame();

private:
  TcpSocket& socket_;
  std::string pending_;

  [[nodiscard]] std::string read_bytes(std::size_t byte_count);
};

} // namespace rtsi
