#include "socket.hpp"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
using socket_t = SOCKET;
#define CLOSE_SOCKET closesocket
#define INVALID_FD INVALID_SOCKET
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <cerrno>
using socket_t = int;
#define CLOSE_SOCKET ::close
#define INVALID_FD (-1)
#endif

#include <cstring>

// --- TcpSocket ---

TcpSocket::~TcpSocket() {
    close();
}

TcpSocket::TcpSocket(TcpSocket&& other) noexcept : fd_(other.fd_) {
    other.fd_ = -1;
}

TcpSocket& TcpSocket::operator=(TcpSocket&& other) noexcept {
    if (this != &other) {
        close();
        fd_ = other.fd_;
        other.fd_ = -1;
    }
    return *this;
}

bool TcpSocket::connect(std::string_view host, uint16_t port) {
    close();
    fd_ = static_cast<int>(::socket(AF_INET, SOCK_STREAM, 0));
    if (fd_ < 0) return false;

    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    std::string host_str(host);
    if (inet_pton(AF_INET, host_str.c_str(), &addr.sin_addr) <= 0) {
        // Try DNS resolution
        struct addrinfo hints{}, *result = nullptr;
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        if (getaddrinfo(host_str.c_str(), nullptr, &hints, &result) != 0) {
            close();
            return false;
        }
        addr.sin_addr = reinterpret_cast<struct sockaddr_in*>(result->ai_addr)->sin_addr;
        freeaddrinfo(result);
    }

    if (::connect(fd_, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        close();
        return false;
    }
    return true;
}

bool TcpSocket::listen(uint16_t port, int backlog) {
    close();
    fd_ = static_cast<int>(::socket(AF_INET, SOCK_STREAM, 0));
    if (fd_ < 0) return false;

    int opt = 1;
    setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));

    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (::bind(fd_, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        close();
        return false;
    }
    if (::listen(fd_, backlog) < 0) {
        close();
        return false;
    }
    return true;
}

TcpSocket TcpSocket::accept() {
    struct sockaddr_in client_addr{};
    socklen_t addr_len = sizeof(client_addr);
    int client_fd = static_cast<int>(
        ::accept(fd_, reinterpret_cast<struct sockaddr*>(&client_addr), &addr_len));
    return TcpSocket(client_fd);
}

int TcpSocket::send(const uint8_t* data, size_t len) {
    if (fd_ < 0) return -1;
    return static_cast<int>(::send(fd_, reinterpret_cast<const char*>(data), len, 0));
}

int TcpSocket::recv(uint8_t* buffer, size_t max_len) {
    if (fd_ < 0) return -1;
    return static_cast<int>(::recv(fd_, reinterpret_cast<char*>(buffer), max_len, 0));
}

void TcpSocket::close() {
    if (fd_ >= 0) {
        CLOSE_SOCKET(fd_);
        fd_ = -1;
    }
}

// --- UdpSocket ---

UdpSocket::~UdpSocket() {
    close();
}

bool UdpSocket::bind(uint16_t port) {
    close();
    fd_ = static_cast<int>(::socket(AF_INET, SOCK_DGRAM, 0));
    if (fd_ < 0) return false;

    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (::bind(fd_, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        close();
        return false;
    }
    return true;
}

int UdpSocket::send_to(const uint8_t* data, size_t len,
                        std::string_view host, uint16_t port) {
    if (fd_ < 0) return -1;

    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    std::string host_str(host);
    inet_pton(AF_INET, host_str.c_str(), &addr.sin_addr);

    return static_cast<int>(
        ::sendto(fd_, reinterpret_cast<const char*>(data), len, 0,
                 reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)));
}

int UdpSocket::recv_from(uint8_t* buffer, size_t max_len,
                          std::string& out_host, uint16_t& out_port) {
    if (fd_ < 0) return -1;

    struct sockaddr_in addr{};
    socklen_t addr_len = sizeof(addr);

    int bytes = static_cast<int>(
        ::recvfrom(fd_, reinterpret_cast<char*>(buffer), max_len, 0,
                   reinterpret_cast<struct sockaddr*>(&addr), &addr_len));

    if (bytes >= 0) {
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip));
        out_host = ip;
        out_port = ntohs(addr.sin_port);
    }
    return bytes;
}

void UdpSocket::close() {
    if (fd_ >= 0) {
        CLOSE_SOCKET(fd_);
        fd_ = -1;
    }
}
