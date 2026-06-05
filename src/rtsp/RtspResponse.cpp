#include "rtsi/rtsp/RtspResponse.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>
#include <string>

namespace rtsi {

namespace {

std::string trim(const std::string& value) {
    const auto first = std::find_if_not(value.begin(), value.end(), [](unsigned char c) {
        return std::isspace(c) != 0;
    });

    if (first == value.end()) {
        return "";
    }

    const auto last = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char c) {
        return std::isspace(c) != 0;
    }).base();

    return std::string(first, last);
}

bool iequals(const std::string& lhs, const std::string& rhs) {
    return lhs.size() == rhs.size() &&
           std::equal(lhs.begin(), lhs.end(), rhs.begin(), [](unsigned char a, unsigned char b) {
               return std::tolower(a) == std::tolower(b);
           });
}

std::string strip_trailing_carriage_return(std::string line) {
    if (!line.empty() && line.back() == '\r') {
        line.pop_back();
    }

    return line;
}

std::size_t parse_size(const std::string& value, const std::string& field_name) {
    if (value.empty()) {
        throw std::invalid_argument(field_name + " cannot be empty");
    }

    if (!std::all_of(value.begin(), value.end(), [](unsigned char c) {
            return std::isdigit(c) != 0;
        })) {
        throw std::invalid_argument(field_name + " must contain only digits");
    }

    return static_cast<std::size_t>(std::stoull(value));
}

} // namespace

const std::string& RtspResponse::version() const noexcept {
    return version_;
}

std::uint16_t RtspResponse::status_code() const noexcept {
    return status_code_;
}

const std::string& RtspResponse::reason_phrase() const noexcept {
    return reason_phrase_;
}

const std::map<std::string, std::string>& RtspResponse::headers() const noexcept {
    return headers_;
}

const std::string& RtspResponse::body() const noexcept {
    return body_;
}

bool RtspResponse::is_success() const noexcept {
    return status_code_ >= 200 && status_code_ < 300;
}

std::optional<std::string> RtspResponse::header(const std::string& name) const {
    for (const auto& [header_name, header_value] : headers_) {
        if (iequals(header_name, name)) {
            return header_value;
        }
    }

    return std::nullopt;
}

std::optional<std::uint32_t> RtspResponse::cseq() const {
    const auto value = header("CSeq");

    if (!value.has_value()) {
        return std::nullopt;
    }

    return static_cast<std::uint32_t>(parse_size(*value, "CSeq"));
}

std::optional<std::string> RtspResponse::session_id() const {
    const auto value = header("Session");

    if (!value.has_value()) {
        return std::nullopt;
    }

    const auto separator = value->find(';');

    if (separator == std::string::npos) {
        return trim(*value);
    }

    return trim(value->substr(0, separator));
}

std::optional<std::size_t> RtspResponse::content_length() const {
    const auto value = header("Content-Length");

    if (!value.has_value()) {
        return std::nullopt;
    }

    return parse_size(*value, "Content-Length");
}

RtspResponse RtspResponse::parse(const std::string& raw_response) {
    if (raw_response.empty()) {
        throw std::invalid_argument("RTSP response cannot be empty");
    }

    std::size_t header_end = raw_response.find("\r\n\r\n");
    std::size_t separator_size = 4;

    if (header_end == std::string::npos) {
        header_end = raw_response.find("\n\n");
        separator_size = 2;
    }

    if (header_end == std::string::npos) {
        throw std::invalid_argument("Invalid RTSP response: missing header/body separator");
    }

    const std::string header_block = raw_response.substr(0, header_end);
    std::string body = raw_response.substr(header_end + separator_size);

    std::istringstream stream(header_block);

    std::string status_line;
    if (!std::getline(stream, status_line)) {
        throw std::invalid_argument("Invalid RTSP response: missing status line");
    }

    status_line = strip_trailing_carriage_return(status_line);

    std::istringstream status_stream(status_line);

    RtspResponse response;

    if (!(status_stream >> response.version_)) {
        throw std::invalid_argument("Invalid RTSP response: missing version");
    }

    int status_code = 0;

    if (!(status_stream >> status_code)) {
        throw std::invalid_argument("Invalid RTSP response: missing status code");
    }

    if (status_code < 100 || status_code > 999) {
        throw std::invalid_argument("Invalid RTSP response: status code must have three digits");
    }

    response.status_code_ = static_cast<std::uint16_t>(status_code);

    std::string reason;
    std::getline(status_stream, reason);
    response.reason_phrase_ = trim(reason);

    if (response.version_ != "RTSP/1.0") {
        throw std::invalid_argument("Invalid RTSP response: unsupported RTSP version");
    }

    std::string line;

    while (std::getline(stream, line)) {
        line = strip_trailing_carriage_return(line);

        if (line.empty()) {
            continue;
        }

        const auto colon = line.find(':');

        if (colon == std::string::npos) {
            throw std::invalid_argument("Invalid RTSP response header: missing ':'");
        }

        const std::string name = trim(line.substr(0, colon));
        const std::string value = trim(line.substr(colon + 1));

        if (name.empty()) {
            throw std::invalid_argument("Invalid RTSP response header: empty name");
        }

        response.headers_[name] = value;
    }

    const auto declared_content_length = response.content_length();

    if (declared_content_length.has_value()) {
        if (body.size() < *declared_content_length) {
            throw std::invalid_argument("Invalid RTSP response: body shorter than Content-Length");
        }

        if (body.size() > *declared_content_length) {
            body = body.substr(0, *declared_content_length);
        }
    }

    response.body_ = std::move(body);

    return response;
}

} // namespace rtsi