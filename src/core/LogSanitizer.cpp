#include "rtsi/core/LogSanitizer.hpp"

#include <algorithm>
#include <cctype>
#include <string>

namespace rtsi {
namespace {

bool starts_with_case_insensitive(const std::string& value, const std::string& prefix) {
    if (value.size() < prefix.size()) {
        return false;
    }

    return std::equal(
        prefix.begin(),
        prefix.end(),
        value.begin(),
        [](unsigned char expected, unsigned char actual) {
            return std::tolower(expected) == std::tolower(actual);
        });
}

} // namespace

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
            if (message[line_end] == '\r'
                && line_end + 1 < message.size()
                && message[line_end + 1] == '\n') {
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

} // namespace rtsi
