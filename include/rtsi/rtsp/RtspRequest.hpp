#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>

namespace rtsi {

enum class RtspMethod { Options, Describe, Setup, Play, Teardown };

[[nodiscard]] std::string to_string(RtspMethod method);

class RtspRequest {
public:
  RtspRequest(RtspMethod method, std::string uri, std::uint32_t cseq);

  void set_header(const std::string &name, const std::string &value);
  void set_body(std::string body);

  [[nodiscard]] RtspMethod method() const noexcept;
  [[nodiscard]] const std::string &uri() const noexcept;
  [[nodiscard]] std::uint32_t cseq() const noexcept;
  [[nodiscard]] const std::map<std::string, std::string> &
  headers() const noexcept;
  [[nodiscard]] const std::string &body() const noexcept;

  [[nodiscard]] std::optional<std::string>
  header(const std::string &name) const;
  [[nodiscard]] std::string serialize() const;
  static RtspRequest options(const std::string &uri, std::uint32_t cseq);
  static RtspRequest describe(const std::string &uri, std::uint32_t cseq);
  static RtspRequest setup(const std::string &uri, std::uint32_t cseq,
                           const std::string &transport);
  static RtspRequest play(const std::string &uri, std::uint32_t cseq,
                          const std::string &session_id);
  static RtspRequest teardown(const std::string &uri, std::uint32_t cseq,
                              const std::string &session_id);

private:
  RtspMethod method_;
  std::string uri_;
  std::uint32_t cseq_;
  std::map<std::string, std::string> headers_;
  std::string body_;
};

} // namespace rtsi