#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace rtsi {

struct UdpDatagram {
    std::vector<std::uint8_t> data;
    std::string sender_host;
    std::uint16_t sender_port = 0;
};

class UdpSocket {
public:
    UdpSocket() = default;
    ~UdpSocket();

    UdpSocket(const UdpSocket&) = delete;
    UdpSocket& operator=(const UdpSocket&) = delete;

    UdpSocket(UdpSocket&& other) noexcept;
    UdpSocket& operator=(UdpSocket&& other) noexcept;

    void bind_to(std::uint16_t port, int timeout_ms = 3000);

    [[nodiscard]] UdpDatagram receive_datagram(std::size_t max_bytes = 65536);

    void close() noexcept;

    [[nodiscard]] bool is_open() const noexcept;
    [[nodiscard]] int native_handle() const noexcept;

private:
    int fd_ = -1;

    void ensure_open() const;
};

} // namespace rtsi