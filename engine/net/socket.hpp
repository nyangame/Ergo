#pragma once
#include <string>
#include <string_view>
#include <cstdint>

class TcpSocket {
public:
    TcpSocket() = default;
    ~TcpSocket();

    TcpSocket(const TcpSocket&) = delete;
    TcpSocket& operator=(const TcpSocket&) = delete;
    TcpSocket(TcpSocket&& other) noexcept;
    TcpSocket& operator=(TcpSocket&& other) noexcept;

    bool connect(std::string_view host, uint16_t port);
    bool listen(uint16_t port, int backlog = 5);
    TcpSocket accept();
    int send(const uint8_t* data, size_t len);
    int recv(uint8_t* buffer, size_t max_len);
    void close();
    bool is_valid() const { return fd_ >= 0; }

private:
    int fd_ = -1;
    explicit TcpSocket(int fd) : fd_(fd) {}
};

class UdpSocket {
public:
    UdpSocket() = default;
    ~UdpSocket();

    UdpSocket(const UdpSocket&) = delete;
    UdpSocket& operator=(const UdpSocket&) = delete;

    bool bind(uint16_t port);
    int send_to(const uint8_t* data, size_t len,
                std::string_view host, uint16_t port);
    int recv_from(uint8_t* buffer, size_t max_len,
                  std::string& out_host, uint16_t& out_port);
    void close();
    bool is_valid() const { return fd_ >= 0; }

private:
    int fd_ = -1;
};
