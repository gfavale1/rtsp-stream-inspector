#pragma once

#include <string>

namespace rtsi {

[[nodiscard]] std::string sanitize_rtsp_message_for_log(const std::string& message);

} // namespace rtsi
