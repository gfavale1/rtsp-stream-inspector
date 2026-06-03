#include "rtsi/rtsp/RtspRequest.hpp"

#include <sstream>
#include <stdexcept>
#include <utility>

namespace rtsi {

std::string to_string(RtspMethod method) {
    switch (method) {
        case RtspMethod::Options:
            return "OPTIONS";
        case RtspMethod::Describe:
            return "DESCRIBE";
        case RtspMethod::Setup:
            return "SETUP";
        case RtspMethod::Play:
            return "PLAY";
        case RtspMethod::Teardown:
            return "TEARDOWN";
    }

    throw std::invalid_argument("Unknown RTSP method");
}

RtspRequest::RtspRequest(RtspMethod method, std::string uri, std::uint32_t cseq)
    : method_(method),
      uri_(std::move(uri)),
      cseq_(cseq) {
    if (uri_.empty()) {
        throw std::invalid_argument("RTSP request URI cannot be empty");
    }

    if (cseq_ == 0) {
        throw std::invalid_argument("RTSP CSeq must be greater than zero");
    }

    set_header("CSeq", std::to_string(cseq_));
}

void RtspRequest::set_header(const std::string& name, const std::string& value) {
    if (name.empty()) {
        throw std::invalid_argument("RTSP header name cannot be empty");
    }

    headers_[name] = value;
}

void RtspRequest::set_body(std::string body) {
    body_ = std::move(body);

    if (!body_.empty()) {
        set_header("Content-Length", std::to_string(body_.size()));
    }
}

RtspMethod RtspRequest::method() const noexcept {
    return method_;
}

const std::string& RtspRequest::uri() const noexcept {
    return uri_;
}

std::uint32_t RtspRequest::cseq() const noexcept {
    return cseq_;
}

const std::map<std::string, std::string>& RtspRequest::headers() const noexcept {
    return headers_;
}

const std::string& RtspRequest::body() const noexcept {
    return body_;
}

std::optional<std::string> RtspRequest::header(const std::string& name) const {
    const auto it = headers_.find(name);

    if (it == headers_.end()) {
        return std::nullopt;
    }

    return it->second;
}

std::string RtspRequest::serialize() const {
    std::ostringstream out;

    out << to_string(method_) << ' ' << uri_ << " RTSP/1.0\r\n";

    for (const auto& [name, value] : headers_) {
        out << name << ": " << value << "\r\n";
    }

    out << "\r\n";

    if (!body_.empty()) {
        out << body_;
    }

    return out.str();
}

RtspRequest RtspRequest::options(const std::string& uri, std::uint32_t cseq) {
    return RtspRequest(RtspMethod::Options, uri, cseq);
}

RtspRequest RtspRequest::describe(const std::string& uri, std::uint32_t cseq) {
    RtspRequest request(RtspMethod::Describe, uri, cseq);
    request.set_header("Accept", "application/sdp");
    return request;
}

RtspRequest RtspRequest::setup(
    const std::string& uri,
    std::uint32_t cseq,
    const std::string& transport
) {
    if (transport.empty()) {
        throw std::invalid_argument("RTSP SETUP transport cannot be empty");
    }

    RtspRequest request(RtspMethod::Setup, uri, cseq);
    request.set_header("Transport", transport);
    return request;
}

RtspRequest RtspRequest::play(
    const std::string& uri,
    std::uint32_t cseq,
    const std::string& session_id
) {
    if (session_id.empty()) {
        throw std::invalid_argument("RTSP PLAY session id cannot be empty");
    }

    RtspRequest request(RtspMethod::Play, uri, cseq);
    request.set_header("Session", session_id);
    return request;
}

RtspRequest RtspRequest::teardown(
    const std::string& uri,
    std::uint32_t cseq,
    const std::string& session_id
) {
    if (session_id.empty()) {
        throw std::invalid_argument("RTSP TEARDOWN session id cannot be empty");
    }

    RtspRequest request(RtspMethod::Teardown, uri, cseq);
    request.set_header("Session", session_id);
    return request;
}

} // namespace rtsi