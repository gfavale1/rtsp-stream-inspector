#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>

namespace rtsi {

class RtspResponse {
public:
    RtspResponse() = default;

    [[nodiscard]] const std::string& version() const noexcept;
    [[nodiscard]] std::uint16_t status_code() const noexcept;
    [[nodiscard]] const std::string& reason_phrase() const noexcept;
    [[nodiscard]] const std::map<std::string, std::string>& headers() const noexcept;
    [[nodiscard]] const std::string& body() const noexcept;

    [[nodiscard]] bool is_success() const noexcept;

    [[nodiscard]] std::optional<std::string> header(const std::string& name) const;
    [[nodiscard]] std::optional<std::uint32_t> cseq() const;
    [[nodiscard]] std::optional<std::string> session_id() const;
    [[nodiscard]] std::optional<std::size_t> content_length() const;

    static RtspResponse parse(const std::string& raw_response);

private:
    std::string version_;
    std::uint16_t status_code_ = 0;
    std::string reason_phrase_;
    std::map<std::string, std::string> headers_;
    std::string body_;
};

} // namespace rtsi