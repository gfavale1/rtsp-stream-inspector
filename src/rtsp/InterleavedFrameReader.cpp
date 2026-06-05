#include "rtsi/rtsp/InterleavedFrameReader.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace rtsi {

bool InterleavedFrame::is_rtp() const noexcept {
  return channel == 0;
}

bool InterleavedFrame::is_rtcp() const noexcept {
  return channel == 1;
}

InterleavedFrameReader::InterleavedFrameReader(TcpSocket& socket,
                                               std::string pending_data)
    : socket_(socket), pending_(std::move(pending_data)) {}

std::string InterleavedFrameReader::read_bytes(std::size_t byte_count) {
  std::string result;
  result.reserve(byte_count);

  if (!pending_.empty()) {
    const std::size_t take = std::min(byte_count, pending_.size());

    result += pending_.substr(0, take);
    pending_.erase(0, take);
  }

  if (result.size() < byte_count) {
    result += socket_.receive_exact(byte_count - result.size());
  }

  return result;
}

InterleavedFrame InterleavedFrameReader::read_frame() {
  std::string marker;

  do {
    marker = read_bytes(1);
  } while (!marker.empty() &&
           static_cast<unsigned char>(marker[0]) != 0x24);

  if (marker.empty()) {
    throw std::runtime_error("Failed to read RTSP interleaved marker");
  }

  const auto header = read_bytes(3);

  if (header.size() != 3) {
    throw std::runtime_error("Incomplete RTSP interleaved header");
  }

  InterleavedFrame frame;

  frame.channel = static_cast<std::uint8_t>(header[0]);

  const auto length = static_cast<std::uint16_t>(
      (static_cast<unsigned char>(header[1]) << 8) |
      static_cast<unsigned char>(header[2]));

  const auto payload = read_bytes(length);

  if (payload.size() != length) {
    throw std::runtime_error("Incomplete RTSP interleaved payload");
  }

  frame.payload.assign(payload.begin(), payload.end());

  return frame;
}

} // namespace rtsi
