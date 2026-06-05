#include "rtsi/net/TcpSocket.hpp"

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>

#include <fcntl.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

namespace rtsi {

namespace {

std::string system_error_message(const std::string &prefix) {
  return prefix + ": " + std::strerror(errno);
}

void set_blocking_mode(int fd, bool blocking) {
  const int flags = fcntl(fd, F_GETFL, 0);

  if (flags == -1) {
    throw std::runtime_error(system_error_message("fcntl(F_GETFL) failed"));
  }

  int new_flags = flags;

  if (blocking) {
    new_flags &= ~O_NONBLOCK;
  } else {
    new_flags |= O_NONBLOCK;
  }

  if (fcntl(fd, F_SETFL, new_flags) == -1) {
    throw std::runtime_error(system_error_message("fcntl(F_SETFL) failed"));
  }
}

void set_socket_timeouts(int fd, int timeout_ms) {
  timeval timeout{};
  timeout.tv_sec = timeout_ms / 1000;
  timeout.tv_usec = (timeout_ms % 1000) * 1000;

  if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) ==
      -1) {
    throw std::runtime_error(
        system_error_message("setsockopt(SO_RCVTIMEO) failed"));
  }

  if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) ==
      -1) {
    throw std::runtime_error(
        system_error_message("setsockopt(SO_SNDTIMEO) failed"));
  }
}

bool wait_for_connect(int fd, int timeout_ms) {
  fd_set write_fds;
  FD_ZERO(&write_fds);
  FD_SET(fd, &write_fds);

  timeval timeout{};
  timeout.tv_sec = timeout_ms / 1000;
  timeout.tv_usec = (timeout_ms % 1000) * 1000;

  const int result = select(fd + 1, nullptr, &write_fds, nullptr, &timeout);

  if (result <= 0) {
    return false;
  }

  int socket_error = 0;
  socklen_t length = sizeof(socket_error);

  if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &socket_error, &length) == -1) {
    return false;
  }

  if (socket_error != 0) {
    errno = socket_error;
    return false;
  }

  return true;
}

} // namespace

TcpSocket::~TcpSocket() { close(); }

TcpSocket::TcpSocket(TcpSocket &&other) noexcept : fd_(other.fd_) {
  other.fd_ = -1;
}

TcpSocket &TcpSocket::operator=(TcpSocket &&other) noexcept {
  if (this != &other) {
    close();
    fd_ = other.fd_;
    other.fd_ = -1;
  }

  return *this;
}

void TcpSocket::connect_to(const std::string &host, std::uint16_t port,
                           int timeout_ms) {
  if (host.empty()) {
    throw std::invalid_argument("TCP host cannot be empty");
  }

  if (port == 0) {
    throw std::invalid_argument("TCP port cannot be zero");
  }

  close();

  addrinfo hints{};
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  addrinfo *result = nullptr;

  const std::string port_text = std::to_string(port);

  const int gai_result =
      getaddrinfo(host.c_str(), port_text.c_str(), &hints, &result);

  if (gai_result != 0) {
    throw std::runtime_error("getaddrinfo failed for host '" + host +
                             "': " + gai_strerror(gai_result));
  }

  std::string last_error;

  for (addrinfo *current = result; current != nullptr;
       current = current->ai_next) {
    const int candidate_fd =
        socket(current->ai_family, current->ai_socktype, current->ai_protocol);

    if (candidate_fd == -1) {
      last_error = system_error_message("socket creation failed");
      continue;
    }

    try {
      set_blocking_mode(candidate_fd, false);

      const int connect_result =
          ::connect(candidate_fd, current->ai_addr, current->ai_addrlen);

      bool connected = false;

      if (connect_result == 0) {
        connected = true;
      } else if (errno == EINPROGRESS) {
        connected = wait_for_connect(candidate_fd, timeout_ms);
      }

      if (connected) {
        set_blocking_mode(candidate_fd, true);
        set_socket_timeouts(candidate_fd, timeout_ms);

        fd_ = candidate_fd;
        freeaddrinfo(result);
        return;
      }

      last_error = system_error_message("connect failed");
      ::close(candidate_fd);

    } catch (...) {
      ::close(candidate_fd);
      freeaddrinfo(result);
      throw;
    }
  }

  freeaddrinfo(result);

  throw std::runtime_error("Unable to connect to " + host + ":" +
                           std::to_string(port) +
                           ". Last error: " + last_error);
}

void TcpSocket::send_all(std::string_view data) {
  ensure_open();

  std::size_t total_sent = 0;

  while (total_sent < data.size()) {
    const ssize_t sent =
        ::send(fd_, data.data() + total_sent, data.size() - total_sent, 0);

    if (sent == -1) {
      throw std::runtime_error(system_error_message("send failed"));
    }

    if (sent == 0) {
      throw std::runtime_error("send failed: connection closed");
    }

    total_sent += static_cast<std::size_t>(sent);
  }
}

std::string TcpSocket::receive_some(std::size_t max_bytes) {
  ensure_open();

  if (max_bytes == 0) {
    throw std::invalid_argument("receive_some max_bytes cannot be zero");
  }

  std::string buffer(max_bytes, '\0');

  const ssize_t received = ::recv(fd_, buffer.data(), buffer.size(), 0);

  if (received == -1) {
    throw std::runtime_error(system_error_message("recv failed"));
  }

  if (received == 0) {
    return "";
  }

  buffer.resize(static_cast<std::size_t>(received));
  return buffer;
}

std::string TcpSocket::receive_exact(std::size_t byte_count) {
  ensure_open();

  std::string result;
  result.reserve(byte_count);

  while (result.size() < byte_count) {
    const std::size_t remaining = byte_count - result.size();

    std::string buffer(remaining, '\0');

    const ssize_t received = ::recv(fd_, buffer.data(), buffer.size(), 0);

    if (received == -1) {
      throw std::runtime_error(system_error_message("recv failed"));
    }

    if (received == 0) {
      throw std::runtime_error(
          "Connection closed while receiving exact number of bytes");
    }

    buffer.resize(static_cast<std::size_t>(received));
    result += buffer;
  }

  return result;
}

std::string TcpSocket::receive_until(std::string_view delimiter,
                                     std::size_t max_bytes) {
  ensure_open();

  if (delimiter.empty()) {
    throw std::invalid_argument("receive_until delimiter cannot be empty");
  }

  std::string result;

  while (result.size() < max_bytes) {
    const auto chunk = receive_some(4096);

    if (chunk.empty()) {
      break;
    }

    result += chunk;

    if (result.find(delimiter) != std::string::npos) {
      break;
    }
  }

  if (result.size() > max_bytes) {
    throw std::runtime_error("receive_until exceeded maximum allowed bytes");
  }

  return result;
}

void TcpSocket::close() noexcept {
  if (fd_ != -1) {
    ::close(fd_);
    fd_ = -1;
  }
}

bool TcpSocket::is_open() const noexcept { return fd_ != -1; }

int TcpSocket::native_handle() const noexcept { return fd_; }

void TcpSocket::ensure_open() const {
  if (!is_open()) {
    throw std::logic_error("TCP socket is not open");
  }
}

} // namespace rtsi