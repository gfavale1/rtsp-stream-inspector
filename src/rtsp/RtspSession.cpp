#include "rtsi/rtsp/RtspSession.hpp"

#include <stdexcept>
#include <utility>

namespace rtsi {

std::string to_string(RtspSessionState state) {
    switch (state) {
        case RtspSessionState::Initial:
            return "initial";
        case RtspSessionState::Described:
            return "described";
        case RtspSessionState::Setup:
            return "setup";
        case RtspSessionState::Playing:
            return "playing";
        case RtspSessionState::Closed:
            return "closed";
    }

    throw std::invalid_argument("Unknown RTSP session state");
}

RtspSession::RtspSession(RtspUrl url)
    : url_(std::move(url)) {}

const RtspUrl& RtspSession::url() const noexcept {
    return url_;
}

RtspSessionState RtspSession::state() const noexcept {
    return state_;
}

std::uint32_t RtspSession::peek_next_cseq() const noexcept {
    return next_cseq_;
}

std::uint32_t RtspSession::consume_cseq() {
    return next_cseq_++;
}

bool RtspSession::has_session_id() const noexcept {
    return session_id_.has_value() && !session_id_->empty();
}

std::optional<std::string> RtspSession::session_id() const {
    return session_id_;
}

void RtspSession::set_session_id(std::string session_id) {
    if (session_id.empty()) {
        throw std::invalid_argument("RTSP session id cannot be empty");
    }

    session_id_ = std::move(session_id);
}

void RtspSession::clear_session_id() {
    session_id_.reset();
}

void RtspSession::mark_described() {
    state_ = RtspSessionState::Described;
}

void RtspSession::mark_setup() {
    state_ = RtspSessionState::Setup;
}

void RtspSession::mark_playing() {
    state_ = RtspSessionState::Playing;
}

void RtspSession::mark_closed() {
    state_ = RtspSessionState::Closed;
}

void RtspSession::apply_response(const RtspResponse& response) {
    const auto response_session_id = response.session_id();

    if (response_session_id.has_value() && !response_session_id->empty()) {
        set_session_id(*response_session_id);
    }
}

RtspRequest RtspSession::make_options_request() {
    return RtspRequest::options(url_.raw, consume_cseq());
}

RtspRequest RtspSession::make_describe_request() {
    return RtspRequest::describe(url_.raw, consume_cseq());
}

RtspRequest RtspSession::make_setup_request(
    const std::string& control_uri,
    const std::string& transport
) {
    return RtspRequest::setup(control_uri, consume_cseq(), transport);
}

RtspRequest RtspSession::make_play_request() {
    if (!has_session_id()) {
        throw std::logic_error("Cannot build PLAY request without RTSP session id");
    }

    return RtspRequest::play(url_.raw, consume_cseq(), *session_id_);
}

RtspRequest RtspSession::make_teardown_request() {
    if (!has_session_id()) {
        throw std::logic_error("Cannot build TEARDOWN request without RTSP session id");
    }

    return RtspRequest::teardown(url_.raw, consume_cseq(), *session_id_);
}

} // namespace rtsi