#pragma once

#include "rtsi/rtsp/RtspRequest.hpp"
#include "rtsi/rtsp/RtspResponse.hpp"
#include "rtsi/rtsp/RtspUrl.hpp"

#include <cstdint>
#include <optional>
#include <string>

namespace rtsi {

enum class RtspSessionState {
    Initial,
    Described,
    Setup,
    Playing,
    Closed
};

[[nodiscard]] std::string to_string(RtspSessionState state);

class RtspSession {
public:
    explicit RtspSession(RtspUrl url);

    [[nodiscard]] const RtspUrl& url() const noexcept;
    [[nodiscard]] RtspSessionState state() const noexcept;

    [[nodiscard]] std::uint32_t peek_next_cseq() const noexcept;
    [[nodiscard]] std::uint32_t consume_cseq();

    [[nodiscard]] bool has_session_id() const noexcept;
    [[nodiscard]] std::optional<std::string> session_id() const;

    void set_session_id(std::string session_id);
    void clear_session_id();

    void mark_described();
    void mark_setup();
    void mark_playing();
    void mark_closed();

    void apply_response(const RtspResponse& response);

    [[nodiscard]] RtspRequest make_options_request();
    [[nodiscard]] RtspRequest make_describe_request();

    [[nodiscard]] RtspRequest make_setup_request(
        const std::string& control_uri,
        const std::string& transport
    );

    [[nodiscard]] RtspRequest make_play_request();
    [[nodiscard]] RtspRequest make_teardown_request();

private:
    RtspUrl url_;
    std::uint32_t next_cseq_ = 1;
    std::optional<std::string> session_id_;
    RtspSessionState state_ = RtspSessionState::Initial;
};

} // namespace rtsi