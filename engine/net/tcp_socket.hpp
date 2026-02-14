#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <memory>

namespace ergo::net {

// TCP socket wrapper backed by POCO StreamSocket / ServerSocket.
// Satisfies: SocketConnectable, SocketListenable, StreamSendable, StreamReceivable
class TcpSocket {
public:
    TcpSocket();
    ~TcpSocket();

    // Move-only
    TcpSocket(TcpSocket&& other) noexcept;
    TcpSocket& operator=(TcpSocket&& other) noexcept;
    TcpSocket(const TcpSocket&) = delete;
    TcpSocket& operator=(const TcpSocket&) = delete;

    // Client: connect to remote host
    bool connect(std::string_view host, uint16_t port);

    // Server: listen on a port for incoming connections
    bool listen(uint16_t port, int backlog = 16);

    // Server: accept a pending connection (blocking)
    TcpSocket accept();

    // Send raw bytes. Returns bytes sent or -1 on error.
    int send(const uint8_t* data, size_t len);

    // Receive raw bytes. Returns bytes received, 0 on disconnect, -1 on error.
    int recv(uint8_t* buffer, size_t max_len);

    // Set non-blocking mode
    void set_non_blocking(bool enabled);

    // Set send/receive timeout in milliseconds
    void set_timeout(int timeout_ms);

    // Close the socket
    void close();

    // Query state
    bool is_connected() const;
    bool is_listening() const;

    // Remote endpoint info
    std::string remote_address() const;
    uint16_t remote_port() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace ergo::net
