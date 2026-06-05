#include "rtsi/rtsp/RtspAuth.hpp"

#include <stdexcept>
#include <string>

namespace rtsi {

std::string base64_encode(std::string_view input) {
    static constexpr char table[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::string output;
    output.reserve(((input.size() + 2) / 3) * 4);

    for (std::size_t i = 0; i < input.size(); i += 3) {
        const unsigned char b0 = static_cast<unsigned char>(input[i]);
        const bool has_b1 = i + 1 < input.size();
        const bool has_b2 = i + 2 < input.size();

        const unsigned char b1 = has_b1 ? static_cast<unsigned char>(input[i + 1]) : 0;
        const unsigned char b2 = has_b2 ? static_cast<unsigned char>(input[i + 2]) : 0;

        output.push_back(table[(b0 >> 2) & 0x3F]);
        output.push_back(table[((b0 & 0x03) << 4) | ((b1 >> 4) & 0x0F)]);

        if (has_b1) {
            output.push_back(table[((b1 & 0x0F) << 2) | ((b2 >> 6) & 0x03)]);
        } else {
            output.push_back('=');
        }

        if (has_b2) {
            output.push_back(table[b2 & 0x3F]);
        } else {
            output.push_back('=');
        }
    }

    return output;
}

std::string make_basic_authorization_value(
    std::string_view username,
    std::string_view password
) {
    if (username.empty()) {
        throw std::invalid_argument("Basic authentication username cannot be empty");
    }

    const std::string user_pass = std::string(username) + ":" + std::string(password);

    return "Basic " + base64_encode(user_pass);
}

} // namespace rtsi