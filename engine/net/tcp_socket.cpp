#include "tcp_socket.hpp"

#if ERGO_HAS_POCO
// ============================================================
// POCO backend
// ============================================================
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/SocketAddress.h>
#include <Poco/Timespan.h>
#include <Poco/Exception.h>

namespace ergo::net {

struct TcpSocket::Impl {
    enum class Mode { None, Client, Server };
    Mode mode = Mode::None;
    Poco::Net::StreamSocket stream;
    Poco::Net::ServerSocket server;
    bool connected = false;
    bool listening = false;
};

TcpSocket::TcpSocket() : impl_(std::make_unique<Impl>()) {}
TcpSocket::~TcpSocket() { if (impl_) close(); }
TcpSocket::TcpSocket(TcpSocket&& other) noexcept = default;
TcpSocket& TcpSocket::operator=(TcpSocket&& other) noexcept = default;

bool TcpSocket::connect(std::string_view host, uint16_t port) {
    try {
        Poco::Net::SocketAddress addr(std::string(host), port);
        impl_->stream.connect(addr);
        impl_->mode = Impl::Mode::Client;
        impl_->connected = true;
        return true;
    } catch (const Poco::Exception&) {
        impl_->connected = false;
        return false;
    }
}

bool TcpSocket::listen(uint16_t port, int backlog) {
    try {
        Poco::Net::SocketAddress addr("0.0.0.0", port);
        impl_->server.bind(addr);
        impl_->server.listen(backlog);
        impl_->mode = Impl::Mode::Server;
        impl_->listening = true;
        return true;
    } catch (const Poco::Exception&) {
        impl_->listening = false;
        return false;
    }
}

TcpSocket TcpSocket::accept() {
    TcpSocket client;
    try {
        client.impl_->stream = impl_->server.acceptConnection();
        client.impl_->mode = Impl::Mode::Client;
        client.impl_->connected = true;
    } catch (const Poco::Exception&) {}
    return client;
}

int TcpSocket::send(const uint8_t* data, size_t len) {
    try {
        return impl_->stream.sendBytes(data, static_cast<int>(len));
    } catch (const Poco::Exception&) {
        impl_->connected = false;
        return -1;
    }
}

int TcpSocket::recv(uint8_t* buffer, size_t max_len) {
    try {
        int n = impl_->stream.receiveBytes(buffer, static_cast<int>(max_len));
        if (n == 0) impl_->connected = false;
        return n;
    } catch (const Poco::TimeoutException&) {
        return 0;
    } catch (const Poco::Exception&) {
        impl_->connected = false;
        return -1;
    }
}

void TcpSocket::set_non_blocking(bool enabled) {
    if (impl_->mode == Impl::Mode::Client)
        impl_->stream.setBlocking(!enabled);
    else if (impl_->mode == Impl::Mode::Server)
        impl_->server.setBlocking(!enabled);
}

void TcpSocket::set_timeout(int timeout_ms) {
    Poco::Timespan ts(0, timeout_ms * 1000);
    if (impl_->mode == Impl::Mode::Client) {
        impl_->stream.setSendTimeout(ts);
        impl_->stream.setReceiveTimeout(ts);
    } else if (impl_->mode == Impl::Mode::Server) {
        impl_->server.setReceiveTimeout(ts);
    }
}

void TcpSocket::close() {
    try {
        if (impl_->connected) { impl_->stream.shutdown(); impl_->stream.close(); }
        if (impl_->listening) { impl_->server.close(); }
    } catch (const Poco::Exception&) {}
    impl_->connected = false;
    impl_->listening = false;
    impl_->mode = Impl::Mode::None;
}

bool TcpSocket::is_connected() const { return impl_->connected; }
bool TcpSocket::is_listening() const { return impl_->listening; }

std::string TcpSocket::remote_address() const {
    try { return impl_->stream.peerAddress().host().toString(); }
    catch (const Poco::Exception&) { return {}; }
}

uint16_t TcpSocket::remote_port() const {
    try { return impl_->stream.peerAddress().port(); }
    catch (const Poco::Exception&) { return 0; }
}

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
#define ERGO_SHUT_RDWR SD_BOTH
#define MSG_NOSIGNAL 0
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
#define ERGO_SHUT_RDWR SHUT_RDWR
using ergo_ssize_t = ssize_t;
inline int ergo_socket_errno() { return errno; }
inline bool ergo_is_wouldblock(int e) { return e == EAGAIN || e == EWOULDBLOCK; }
#endif
#include <cerrno>
#include <cstring>

namespace ergo::net {

struct TcpSocket::Impl {
    enum class Mode { None, Client, Server };
    Mode mode = Mode::None;
    int fd = -1;
    bool connected = false;
    bool listening = false;
};

TcpSocket::TcpSocket() : impl_(std::make_unique<Impl>()) {}

TcpSocket::~TcpSocket() { if (impl_) close(); }

TcpSocket::TcpSocket(TcpSocket&& other) noexcept = default;
TcpSocket& TcpSocket::operator=(TcpSocket&& other) noexcept = default;

bool TcpSocket::connect(std::string_view host, uint16_t port) {
    impl_->fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (impl_->fd < 0) return false;

    struct addrinfo hints{}, *res = nullptr;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    std::string host_str(host);
    std::string port_str = std::to_string(port);
    if (::getaddrinfo(host_str.c_str(), port_str.c_str(), &hints, &res) != 0 || !res) {
        ERGO_CLOSE_SOCKET(impl_->fd);
        impl_->fd = -1;
        return false;
    }

    int rc = ::connect(impl_->fd, res->ai_addr, static_cast<int>(res->ai_addrlen));
    ::freeaddrinfo(res);
    if (rc < 0) {
        ERGO_CLOSE_SOCKET(impl_->fd);
        impl_->fd = -1;
        return false;
    }

    impl_->mode = Impl::Mode::Client;
    impl_->connected = true;
    return true;
}

bool TcpSocket::listen(uint16_t port, int backlog) {
    impl_->fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (impl_->fd < 0) return false;

    int opt = 1;
    ::setsockopt(impl_->fd, SOL_SOCKET, SO_REUSEADDR,
                 reinterpret_cast<const char*>(&opt), sizeof(opt));

    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (::bind(impl_->fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        ERGO_CLOSE_SOCKET(impl_->fd);
        impl_->fd = -1;
        return false;
    }
    if (::listen(impl_->fd, backlog) < 0) {
        ERGO_CLOSE_SOCKET(impl_->fd);
        impl_->fd = -1;
        return false;
    }

    impl_->mode = Impl::Mode::Server;
    impl_->listening = true;
    return true;
}

TcpSocket TcpSocket::accept() {
    TcpSocket client;
    struct sockaddr_in client_addr{};
    socklen_t len = sizeof(client_addr);
    int cfd = ::accept(impl_->fd, reinterpret_cast<struct sockaddr*>(&client_addr), &len);
    if (cfd >= 0) {
        client.impl_->fd = cfd;
        client.impl_->mode = Impl::Mode::Client;
        client.impl_->connected = true;
    }
    return client;
}

int TcpSocket::send(const uint8_t* data, size_t len) {
    ergo_ssize_t n = ::send(impl_->fd,
                            reinterpret_cast<const char*>(data),
                            static_cast<int>(len), MSG_NOSIGNAL);
    if (n < 0) {
        impl_->connected = false;
        return -1;
    }
    return static_cast<int>(n);
}

int TcpSocket::recv(uint8_t* buffer, size_t max_len) {
    ergo_ssize_t n = ::recv(impl_->fd,
                            reinterpret_cast<char*>(buffer),
                            static_cast<int>(max_len), 0);
    if (n == 0) {
        impl_->connected = false;
        return 0;
    }
    if (n < 0) {
        if (ergo_is_wouldblock(ergo_socket_errno())) return 0;
        impl_->connected = false;
        return -1;
    }
    return static_cast<int>(n);
}

void TcpSocket::set_non_blocking(bool enabled) {
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

void TcpSocket::set_timeout(int timeout_ms) {
    if (impl_->fd < 0) return;
#ifdef _WIN32
    DWORD tv = static_cast<DWORD>(timeout_ms);
    ::setsockopt(impl_->fd, SOL_SOCKET, SO_RCVTIMEO,
                 reinterpret_cast<const char*>(&tv), sizeof(tv));
    ::setsockopt(impl_->fd, SOL_SOCKET, SO_SNDTIMEO,
                 reinterpret_cast<const char*>(&tv), sizeof(tv));
#else
    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    ::setsockopt(impl_->fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    ::setsockopt(impl_->fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
#endif
}

void TcpSocket::close() {
    if (impl_->fd >= 0) {
        ::shutdown(impl_->fd, ERGO_SHUT_RDWR);
        ERGO_CLOSE_SOCKET(impl_->fd);
        impl_->fd = -1;
    }
    impl_->connected = false;
    impl_->listening = false;
    impl_->mode = Impl::Mode::None;
}

bool TcpSocket::is_connected() const { return impl_->connected; }
bool TcpSocket::is_listening() const { return impl_->listening; }

std::string TcpSocket::remote_address() const {
    if (impl_->fd < 0) return {};
    struct sockaddr_in addr{};
    socklen_t len = sizeof(addr);
    if (::getpeername(impl_->fd, reinterpret_cast<struct sockaddr*>(&addr), &len) < 0)
        return {};
    char buf[INET_ADDRSTRLEN]{};
    ::inet_ntop(AF_INET, &addr.sin_addr, buf, sizeof(buf));
    return buf;
}

uint16_t TcpSocket::remote_port() const {
    if (impl_->fd < 0) return 0;
    struct sockaddr_in addr{};
    socklen_t len = sizeof(addr);
    if (::getpeername(impl_->fd, reinterpret_cast<struct sockaddr*>(&addr), &len) < 0)
        return 0;
    return ntohs(addr.sin_port);
}

} // namespace ergo::net

#endif // ERGO_HAS_POCO
