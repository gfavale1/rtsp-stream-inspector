#include "rtsi/net/UdpSocket.hpp"

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

namespace rtsi {

namespace {

std::string system_error_message(const std::string& prefix) {
    return prefix + ": " + std::strerror(errno);
}

void set_socket_receive_timeout(int fd, int timeout_ms) {
    timeval timeout {};
    timeout.tv_sec = timeout_ms / 1000;
    timeout.tv_usec = (timeout_ms % 1000) * 1000;

    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1) {
        throw std::runtime_error(system_error_message("setsockopt(SO_RCVTIMEO) failed"));
    }
}

} // namespace

UdpSocket::~UdpSocket() {
    close();
}

UdpSocket::UdpSocket(UdpSocket&& other) noexcept
    : fd_(other.fd_) {
    other.fd_ = -1;
}

UdpSocket& UdpSocket::operator=(UdpSocket&& other) noexcept {
    if (this != &other) {
        close();
        fd_ = other.fd_;
        other.fd_ = -1;
    }

    return *this;
}

void UdpSocket::bind_to(std::uint16_t port, int timeout_ms) {
    if (port == 0) {
        throw std::invalid_argument("UDP port cannot be zero");
    }

    close();

    const int candidate_fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (candidate_fd == -1) {
        throw std::runtime_error(system_error_message("UDP socket creation failed"));
    }

    try {
        int reuse = 1;

        if (setsockopt(candidate_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
            throw std::runtime_error(system_error_message("setsockopt(SO_REUSEADDR) failed"));
        }

        sockaddr_in address {};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);

        if (::bind(candidate_fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == -1) {
            throw std::runtime_error(system_error_message("UDP bind failed"));
        }

        set_socket_receive_timeout(candidate_fd, timeout_ms);

        fd_ = candidate_fd;

    } catch (...) {
        ::close(candidate_fd);
        throw;
    }
}

UdpDatagram UdpSocket::receive_datagram(std::size_t max_bytes) {
    ensure_open();

    if (max_bytes == 0) {
        throw std::invalid_argument("receive_datagram max_bytes cannot be zero");
    }

    UdpDatagram datagram;
    datagram.data.resize(max_bytes);

    sockaddr_in sender {};
    socklen_t sender_length = sizeof(sender);

    const ssize_t received = recvfrom(
        fd_,
        datagram.data.data(),
        datagram.data.size(),
        0,
        reinterpret_cast<sockaddr*>(&sender),
        &sender_length
    );

    if (received == -1) {
        throw std::runtime_error(system_error_message("recvfrom failed"));
    }

    datagram.data.resize(static_cast<std::size_t>(received));

    char sender_buffer[INET_ADDRSTRLEN] {};

    const char* converted = inet_ntop(
        AF_INET,
        &sender.sin_addr,
        sender_buffer,
        sizeof(sender_buffer)
    );

    if (converted != nullptr) {
        datagram.sender_host = converted;
    }

    datagram.sender_port = ntohs(sender.sin_port);

    return datagram;
}

void UdpSocket::close() noexcept {
    if (fd_ != -1) {
        ::close(fd_);
        fd_ = -1;
    }
}

bool UdpSocket::is_open() const noexcept {
    return fd_ != -1;
}

int UdpSocket::native_handle() const noexcept {
    return fd_;
}

void UdpSocket::ensure_open() const {
    if (!is_open()) {
        throw std::logic_error("UDP socket is not open");
    }
}

} // namespace rtsi