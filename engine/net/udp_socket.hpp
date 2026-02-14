#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <memory>

namespace ergo::net {

// UDP socket wrapper backed by POCO DatagramSocket.
// Satisfies: DatagramSendable, DatagramReceivable
class UdpSocket {
public:
    UdpSocket();
    ~UdpSocket();

    // Move-only
    UdpSocket(UdpSocket&& other) noexcept;
    UdpSocket& operator=(UdpSocket&& other) noexcept;
    UdpSocket(const UdpSocket&) = delete;
    UdpSocket& operator=(const UdpSocket&) = delete;

    // Bind to a local port for receiving
    bool bind(uint16_t port);

    // Send datagram to specific host:port. Returns bytes sent or -1 on error.
    int send_to(const uint8_t* data, size_t len,
                std::string_view host, uint16_t port);

    // Receive datagram. Returns bytes received or -1 on error.
    // out_host and out_port are filled with the sender's address.
    int recv_from(uint8_t* buffer, size_t max_len,
                  std::string& out_host, uint16_t& out_port);

    // Set non-blocking mode
    void set_non_blocking(bool enabled);

    // Set receive timeout in milliseconds
    void set_timeout(int timeout_ms);

    // Close the socket
    void close();

    // Query
    bool is_bound() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace ergo::net
