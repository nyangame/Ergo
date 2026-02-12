#include "udp_socket.hpp"

#if ERGO_HAS_POCO
// ============================================================
// POCO backend
// ============================================================
#include <Poco/Net/DatagramSocket.h>
#include <Poco/Net/SocketAddress.h>
#include <Poco/Timespan.h>
#include <Poco/Exception.h>

namespace ergo::net {

struct UdpSocket::Impl {
    Poco::Net::DatagramSocket socket;
    bool bound = false;
};

UdpSocket::UdpSocket() : impl_(std::make_unique<Impl>()) {}
UdpSocket::~UdpSocket() { if (impl_) close(); }
UdpSocket::UdpSocket(UdpSocket&& other) noexcept = default;
UdpSocket& UdpSocket::operator=(UdpSocket&& other) noexcept = default;

bool UdpSocket::bind(uint16_t port) {
    try {
        Poco::Net::SocketAddress addr("0.0.0.0", port);
        impl_->socket.bind(addr);
        impl_->bound = true;
        return true;
    } catch (const Poco::Exception&) {
        impl_->bound = false;
        return false;
    }
}

int UdpSocket::send_to(const uint8_t* data, size_t len,
                        std::string_view host, uint16_t port) {
    try {
        Poco::Net::SocketAddress addr(std::string(host), port);
        return impl_->socket.sendTo(data, static_cast<int>(len), addr);
    } catch (const Poco::Exception&) { return -1; }
}

int UdpSocket::recv_from(uint8_t* buffer, size_t max_len,
                          std::string& out_host, uint16_t& out_port) {
    try {
        Poco::Net::SocketAddress sender;
        int n = impl_->socket.receiveFrom(buffer, static_cast<int>(max_len), sender);
        out_host = sender.host().toString();
        out_port = sender.port();
        return n;
    } catch (const Poco::TimeoutException&) { return 0; }
      catch (const Poco::Exception&) { return -1; }
}

void UdpSocket::set_non_blocking(bool enabled) {
    impl_->socket.setBlocking(!enabled);
}

void UdpSocket::set_timeout(int timeout_ms) {
    Poco::Timespan ts(0, timeout_ms * 1000);
    impl_->socket.setReceiveTimeout(ts);
}

void UdpSocket::close() {
    try { impl_->socket.close(); } catch (const Poco::Exception&) {}
    impl_->bound = false;
}

bool UdpSocket::is_bound() const { return impl_->bound; }

} // namespace ergo::net

#else
// ============================================================
// Native socket backend (POSIX / Winsock)
// ============================================================
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#define ERGO_CLOSE_SOCKET closesocket
using ergo_ssize_t = ptrdiff_t;
inline int ergo_socket_errno() { return WSAGetLastError(); }
inline bool ergo_is_wouldblock(int e) { return e == WSAEWOULDBLOCK; }
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#define ERGO_CLOSE_SOCKET ::close
using ergo_ssize_t = ssize_t;
inline int ergo_socket_errno() { return errno; }
inline bool ergo_is_wouldblock(int e) { return e == EAGAIN || e == EWOULDBLOCK; }
#endif
#include <cerrno>
#include <cstring>

namespace ergo::net {

struct UdpSocket::Impl {
    int fd = -1;
    bool bound = false;
};

UdpSocket::UdpSocket() : impl_(std::make_unique<Impl>()) {
    impl_->fd = ::socket(AF_INET, SOCK_DGRAM, 0);
}

UdpSocket::~UdpSocket() { if (impl_) close(); }

UdpSocket::UdpSocket(UdpSocket&& other) noexcept = default;
UdpSocket& UdpSocket::operator=(UdpSocket&& other) noexcept = default;

bool UdpSocket::bind(uint16_t port) {
    if (impl_->fd < 0) return false;

    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (::bind(impl_->fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0)
        return false;

    impl_->bound = true;
    return true;
}

int UdpSocket::send_to(const uint8_t* data, size_t len,
                        std::string_view host, uint16_t port) {
    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    std::string host_str(host);
    if (::inet_pton(AF_INET, host_str.c_str(), &addr.sin_addr) <= 0) {
        // Try DNS resolution
        struct addrinfo hints{}, *res = nullptr;
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_DGRAM;
        std::string port_str = std::to_string(port);
        if (::getaddrinfo(host_str.c_str(), port_str.c_str(), &hints, &res) != 0 || !res)
            return -1;
        addr = *reinterpret_cast<struct sockaddr_in*>(res->ai_addr);
        ::freeaddrinfo(res);
    }

    ergo_ssize_t n = ::sendto(impl_->fd,
                              reinterpret_cast<const char*>(data),
                              static_cast<int>(len), 0,
                              reinterpret_cast<struct sockaddr*>(&addr),
                              sizeof(addr));
    return n < 0 ? -1 : static_cast<int>(n);
}

int UdpSocket::recv_from(uint8_t* buffer, size_t max_len,
                          std::string& out_host, uint16_t& out_port) {
    struct sockaddr_in sender{};
    socklen_t sender_len = sizeof(sender);
    ergo_ssize_t n = ::recvfrom(impl_->fd,
                                reinterpret_cast<char*>(buffer),
                                static_cast<int>(max_len), 0,
                                reinterpret_cast<struct sockaddr*>(&sender),
                                &sender_len);
    if (n < 0) {
        if (ergo_is_wouldblock(ergo_socket_errno())) return 0;
        return -1;
    }
    char ip_buf[INET_ADDRSTRLEN]{};
    ::inet_ntop(AF_INET, &sender.sin_addr, ip_buf, sizeof(ip_buf));
    out_host = ip_buf;
    out_port = ntohs(sender.sin_port);
    return static_cast<int>(n);
}

void UdpSocket::set_non_blocking(bool enabled) {
    if (impl_->fd < 0) return;
#ifdef _WIN32
    u_long mode = enabled ? 1 : 0;
    ::ioctlsocket(impl_->fd, FIONBIO, &mode);
#else
    int flags = ::fcntl(impl_->fd, F_GETFL, 0);
    if (enabled)
        ::fcntl(impl_->fd, F_SETFL, flags | O_NONBLOCK);
    else
        ::fcntl(impl_->fd, F_SETFL, flags & ~O_NONBLOCK);
#endif
}

void UdpSocket::set_timeout(int timeout_ms) {
    if (impl_->fd < 0) return;
#ifdef _WIN32
    DWORD tv = static_cast<DWORD>(timeout_ms);
    ::setsockopt(impl_->fd, SOL_SOCKET, SO_RCVTIMEO,
                 reinterpret_cast<const char*>(&tv), sizeof(tv));
#else
    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    ::setsockopt(impl_->fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#endif
}

void UdpSocket::close() {
    if (impl_->fd >= 0) {
        ERGO_CLOSE_SOCKET(impl_->fd);
        impl_->fd = -1;
    }
    impl_->bound = false;
}

bool UdpSocket::is_bound() const { return impl_->bound; }

} // namespace ergo::net

#endif // ERGO_HAS_POCO
