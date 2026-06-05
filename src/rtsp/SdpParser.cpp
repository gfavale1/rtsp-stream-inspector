#include "rtsi/rtsp/SdpParser.hpp"

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

std::string strip_cr(std::string value) {
    if (!value.empty() && value.back() == '\r') {
        value.pop_back();
    }

    return value;
}

bool starts_with(const std::string& value, const std::string& prefix) {
    return value.rfind(prefix, 0) == 0;
}

std::vector<std::string> split_by_char(const std::string& value, char delimiter) {
    std::vector<std::string> parts;
    std::string current;
    std::istringstream stream(value);

    while (std::getline(stream, current, delimiter)) {
        parts.push_back(current);
    }

    return parts;
}

std::int32_t parse_int(const std::string& text, const std::string& field_name) {
    if (text.empty()) {
        throw std::invalid_argument(field_name + " cannot be empty");
    }

    if (!std::all_of(text.begin(), text.end(), [](unsigned char c) {
            return std::isdigit(c) != 0;
        })) {
        throw std::invalid_argument(field_name + " must contain only digits");
    }

    return std::stoi(text);
}

void parse_media_line(const std::string& value, SdpDescription& description) {
    std::istringstream stream(value);

    SdpMediaTrack track;

    std::string port;
    std::string payload_type;

    if (!(stream >> track.media_type >> port >> track.protocol >> payload_type)) {
        throw std::invalid_argument("Invalid SDP media line: " + value);
    }

    track.payload_type = parse_int(payload_type, "SDP payload type");

    description.tracks.push_back(track);
}

void parse_rtpmap_attribute(const std::string& value, SdpMediaTrack& track) {
    const auto colon = value.find(':');

    if (colon == std::string::npos) {
        return;
    }

    const std::string after_colon = value.substr(colon + 1);

    const auto space = after_colon.find(' ');

    if (space == std::string::npos) {
        return;
    }

    const std::string payload_text = after_colon.substr(0, space);
    const auto payload_type = parse_int(payload_text, "SDP rtpmap payload type");

    if (payload_type != track.payload_type) {
        return;
    }

    const std::string encoding = after_colon.substr(space + 1);
    const auto encoding_parts = split_by_char(encoding, '/');

    if (!encoding_parts.empty()) {
        track.codec = trim(encoding_parts[0]);
    }

    if (encoding_parts.size() >= 2) {
        track.clock_rate = parse_int(trim(encoding_parts[1]), "SDP clock rate");
    }

    if (encoding_parts.size() >= 3) {
        track.encoding_parameters = trim(encoding_parts[2]);
    }
}

void parse_fmtp_attribute(const std::string& value, SdpMediaTrack& track) {
    const auto colon = value.find(':');

    if (colon == std::string::npos) {
        return;
    }

    const std::string after_colon = value.substr(colon + 1);

    const auto space = after_colon.find(' ');

    if (space == std::string::npos) {
        return;
    }

    const std::string payload_text = after_colon.substr(0, space);
    const auto payload_type = parse_int(payload_text, "SDP fmtp payload type");

    if (payload_type != track.payload_type) {
        return;
    }

    track.fmtp = trim(after_colon.substr(space + 1));
}

void parse_control_attribute(const std::string& value, SdpMediaTrack& track) {
    const std::string prefix = "a=control:";

    if (!starts_with(value, prefix)) {
        return;
    }

    track.control = trim(value.substr(prefix.size()));
}

} // namespace

bool SdpMediaTrack::is_video() const noexcept {
    return media_type == "video";
}

bool SdpMediaTrack::is_audio() const noexcept {
    return media_type == "audio";
}

std::optional<SdpMediaTrack> SdpDescription::first_video_track() const {
    for (const auto& track : tracks) {
        if (track.is_video()) {
            return track;
        }
    }

    return std::nullopt;
}

std::optional<SdpMediaTrack> SdpDescription::first_audio_track() const {
    for (const auto& track : tracks) {
        if (track.is_audio()) {
            return track;
        }
    }

    return std::nullopt;
}

SdpDescription SdpParser::parse(const std::string& raw_sdp) {
    if (raw_sdp.empty()) {
        throw std::invalid_argument("SDP body cannot be empty");
    }

    SdpDescription description;
    description.raw = raw_sdp;

    std::istringstream stream(raw_sdp);
    std::string line;

    SdpMediaTrack* current_track = nullptr;

    while (std::getline(stream, line)) {
        line = trim(strip_cr(line));

        if (line.empty()) {
            continue;
        }

        if (starts_with(line, "o=")) {
            description.origin = line.substr(2);
            continue;
        }

        if (starts_with(line, "s=")) {
            description.session_name = line.substr(2);
            continue;
        }

        if (starts_with(line, "c=")) {
            description.connection_address = line.substr(2);
            continue;
        }

        if (starts_with(line, "m=")) {
            parse_media_line(line.substr(2), description);
            current_track = &description.tracks.back();
            continue;
        }

        if (current_track == nullptr) {
            continue;
        }

        if (starts_with(line, "a=rtpmap:")) {
            parse_rtpmap_attribute(line, *current_track);
            continue;
        }

        if (starts_with(line, "a=fmtp:")) {
            parse_fmtp_attribute(line, *current_track);
            continue;
        }

        if (starts_with(line, "a=control:")) {
            parse_control_attribute(line, *current_track);
            continue;
        }
    }

    if (description.tracks.empty()) {
        throw std::invalid_argument("SDP does not contain media tracks");
    }

    return description;
}

std::string build_control_uri(
    const std::string& base_uri,
    const std::string& control
) {
    if (base_uri.empty()) {
        throw std::invalid_argument("Base RTSP URI cannot be empty");
    }

    if (control.empty()) {
        throw std::invalid_argument("SDP control attribute cannot be empty");
    }

    if (starts_with(control, "rtsp://")) {
        return control;
    }

    if (control == "*") {
        return base_uri;
    }

    if (!base_uri.empty() && base_uri.back() == '/') {
        return base_uri + control;
    }

    return base_uri + "/" + control;
}

} // namespace rtsi