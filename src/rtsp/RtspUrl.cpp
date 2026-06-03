#include "rtsi/rtsp/RtspUrl.hpp"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <string>

namespace rtsi {

// utility - lowercase strings
std::string to_lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    return value;
}



bool is_digits(const std::string &value) {
  return !value.empty() &&
         std::all_of(value.begin(), value.end(),
                     [](unsigned char c) { return std::isdigit(c) != 0; });
}

std::uint16_t parse_port_number(const std::string& port_text) {
    if(!is_digits(port_text))
    {
        throw std::invalid_argument("Invalid RTSP port: port must contains only digits");
    }

    const int port = std::stoi(port_text);

    if(port <= 0 || port > 65535)
    {
        throw std::invalid_argument("Invalid RTSP port: port must be between 1 and 65535");
    }

    return static_cast<std::uint16_t>(port);
}

bool RtspUrl::has_credentials() const noexcept {
    return !username.empty();
}

// utility - build string
std::string RtspUrl::authority() const {
    std::string result;

    if (has_credentials()) 
    {
        result += username;

        if (!password.empty()) 
        {
            result += ":";
            result += password;
        }

        result += "@";
    }

    result += host;
    result += ":";
    result += std::to_string(port);

    return result;
}

// parser
RtspUrl RtspUrl::parse(const std::string& raw_url) {
    if(raw_url.empty())
    {
        throw std::invalid_argument("RTSP URL cannot be empty");
    }

    const auto scheme_separator = raw_url.find("://");

    if(scheme_separator == std::string::npos)
    {
        throw std::invalid_argument("Invalid RTSP URL: missing scheme operator");
    }

    RtspUrl parsed;
    parsed.raw = raw_url;
    parsed.scheme = to_lower(raw_url.substr(0, scheme_separator));

    if(parsed.scheme != "rtsp")
    {
        throw std::invalid_argument("Invalid RTSP URL: only rtsp:// is supported");
    }

    std::string remainder = raw_url.substr(scheme_separator + 3);

    if(remainder.empty()) 
    {
        throw std::invalid_argument("Invalid RTSP URL: missing authority");
    }

     std::string authority_part;
    const auto path_start = remainder.find('/');

    if (path_start == std::string::npos) 
    {
        authority_part = remainder;
        parsed.path = "/";
    } else {
        authority_part = remainder.substr(0, path_start);
        parsed.path = remainder.substr(path_start);

        if (parsed.path.empty()) 
        {
            parsed.path = "/";
        }
    }

    if (authority_part.empty()) 
    {
        throw std::invalid_argument("Invalid RTSP URL: empty authority");
    }

    const auto at_position = authority_part.find('@');

    std::string host_port_part;

    if (at_position != std::string::npos) {
        const std::string user_info = authority_part.substr(0, at_position);
        host_port_part = authority_part.substr(at_position + 1);

        if (user_info.empty()) 
        {
            throw std::invalid_argument("Invalid RTSP URL: empty user info");
        }

        if (host_port_part.empty()) 
        {
            throw std::invalid_argument("Invalid RTSP URL: missing host after credentials");
        }

        const auto colon_position = user_info.find(':');

        if (colon_position == std::string::npos) 
        {
            parsed.username = user_info;
        } else {
            parsed.username = user_info.substr(0, colon_position);
            parsed.password = user_info.substr(colon_position + 1);
        }

        if (parsed.username.empty()) 
        {
            throw std::invalid_argument("Invalid RTSP URL: username cannot be empty");
        }
    } else {
        host_port_part = authority_part;
    }

    if (host_port_part.empty()) 
    {
        throw std::invalid_argument("Invalid RTSP URL: missing host");
    }

    if (host_port_part.front() == '[') 
    {
        throw std::invalid_argument("IPv6 RTSP URLs are not supported yet");
    }

    const auto port_separator = host_port_part.rfind(':');

    if (port_separator == std::string::npos) 
    {
        parsed.host = host_port_part;
        parsed.port = 554;
        parsed.has_explicit_port = false;
    } else {
        parsed.host = host_port_part.substr(0, port_separator);
        const std::string port_text = host_port_part.substr(port_separator + 1);

        if (parsed.host.empty()) 
        {
            throw std::invalid_argument("Invalid RTSP URL: host cannot be empty");
        }

        parsed.port = parse_port_number(port_text);
        parsed.has_explicit_port = true;
    }

    if (parsed.host.empty()) 
    {
        throw std::invalid_argument("Invalid RTSP URL: host cannot be empty");
    }

    if (parsed.path.empty()) 
    {
        parsed.path = "/";
    }

    return parsed;
}

} // namespace rtsi