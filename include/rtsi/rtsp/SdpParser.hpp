#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace rtsi {

struct SdpMediaTrack {
    std::string media_type;
    std::string protocol;
    std::int32_t payload_type = -1;

    std::string codec;
    std::int32_t clock_rate = -1;
    std::string encoding_parameters;

    std::string control;
    std::string fmtp;

    [[nodiscard]] bool is_video() const noexcept;
    [[nodiscard]] bool is_audio() const noexcept;
};

struct SdpDescription {
    std::string raw;
    std::string origin;
    std::string session_name;
    std::string connection_address;
    std::vector<SdpMediaTrack> tracks;

    [[nodiscard]] std::optional<SdpMediaTrack> first_video_track() const;
    [[nodiscard]] std::optional<SdpMediaTrack> first_audio_track() const;
};

class SdpParser {
public:
    [[nodiscard]] static SdpDescription parse(const std::string& raw_sdp);
};

[[nodiscard]] std::string build_control_uri(
    const std::string& base_uri,
    const std::string& control
);

} // namespace rtsi