#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <string_view>

namespace rtsi {

class TcpSocket {
public:
    TcpSocket() = default;
    ~TcpSocket();

    TcpSocket(const TcpSocket&) = delete;
    TcpSocket& operator=(const TcpSocket&) = delete;

    TcpSocket(TcpSocket&& other) noexcept;
    TcpSocket& operator=(TcpSocket&& other) noexcept;

    void connect_to(
        const std::string& host,
        std::uint16_t port,
        int timeout_ms = 3000
    );

    void send_all(std::string_view data);

    [[nodiscard]] std::string receive_some(std::size_t max_bytes = 4096);

    [[nodiscard]] std::string receive_until(
        std::string_view delimiter,
        std::size_t max_bytes = 65536
    );

    void close() noexcept;

    [[nodiscard]] bool is_open() const noexcept;
    [[nodiscard]] int native_handle() const noexcept;

private:
    int fd_ = -1;

    void ensure_open() const;
};

} // namespace rtsi