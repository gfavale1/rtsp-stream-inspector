#pragma once

#include <string>
#include <string_view>

namespace rtsi {

[[nodiscard]] std::string base64_encode(std::string_view input);

[[nodiscard]] std::string make_basic_authorization_value(
    std::string_view username,
    std::string_view password
);

} // namespace rtsi