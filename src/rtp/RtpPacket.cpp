#include "rtsi/rtp/RtpPacket.hpp"

namespace rtsi {

std::size_t RtpPacket::payload_size() const noexcept {
    return payload.size();
}

} // namespace rtsi