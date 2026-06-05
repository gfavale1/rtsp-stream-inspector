#pragma once

#include <cstdint>
#include <string>

namespace rtsi {

struct RtspUrl {
  std::string raw;
  std::string scheme;
  std::string username;
  std::string password;
  std::string host;
  std::uint16_t port = 554;
  std::string path = "/";

  // functions
  bool has_explicit_port = false;

  // nodiscard - do not ignore return value
  [[nodiscard]] bool has_credentials() const noexcept;

  // const - don't modify the object that calls
  [[nodiscard]] std::string authority() const;

  [[nodiscard]] std::string request_uri() const;
  
  static RtspUrl parse(const std::string &raw_url);
};
} // namespace rtsi