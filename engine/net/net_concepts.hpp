#pragma once

#include <concepts>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <functional>

// ============================================================
// Network message
// ============================================================

struct NetMessage {
    uint16_t type = 0;
    std::vector<uint8_t> payload;
};

// ============================================================
// Network concepts (concept-based design: no inheritance)
// ============================================================

// SocketConnectable: can establish a connection to a remote host
template<typename T>
concept SocketConnectable = requires(T t, std::string_view host, uint16_t port) {
    { t.connect(host, port) } -> std::same_as<bool>;
    { t.close() } -> std::same_as<void>;
    { t.is_connected() } -> std::same_as<bool>;
};

// SocketListenable: can listen for incoming connections
template<typename T>
concept SocketListenable = requires(T t, uint16_t port) {
    { t.listen(port) } -> std::same_as<bool>;
    { t.close() } -> std::same_as<void>;
};

// StreamSendable: can send a stream of bytes (TCP)
template<typename T>
concept StreamSendable = requires(T t, const uint8_t* data, size_t len) {
    { t.send(data, len) } -> std::convertible_to<int>;
};

// StreamReceivable: can receive a stream of bytes (TCP)
template<typename T>
concept StreamReceivable = requires(T t, uint8_t* buf, size_t max_len) {
    { t.recv(buf, max_len) } -> std::convertible_to<int>;
};

// DatagramSendable: can send datagrams to a specific host:port (UDP)
template<typename T>
concept DatagramSendable = requires(T t, const uint8_t* data, size_t len,
                                     std::string_view host, uint16_t port) {
    { t.send_to(data, len, host, port) } -> std::convertible_to<int>;
};

// DatagramReceivable: can receive datagrams with sender info (UDP)
template<typename T>
concept DatagramReceivable = requires(T t, uint8_t* buf, size_t max_len,
                                       std::string& out_host, uint16_t& out_port) {
    { t.recv_from(buf, max_len, out_host, out_port) } -> std::convertible_to<int>;
};

// NetworkManageable: high-level network manager
template<typename T>
concept NetworkManageable = requires(T t, std::string_view host, uint16_t port,
                                      const NetMessage& msg,
                                      std::function<void(const NetMessage&)> handler) {
    { t.connect(host, port) } -> std::same_as<bool>;
    { t.host_server(port) } -> std::same_as<bool>;
    { t.send(msg) } -> std::same_as<void>;
    { t.poll() } -> std::same_as<void>;
    { t.shutdown() } -> std::same_as<void>;
    { t.is_active() } -> std::same_as<bool>;
};

// HttpRequestable: can perform HTTP requests
template<typename T>
concept HttpRequestable = requires(T t, std::string_view url) {
    { t.get(url) };
    { t.post(url, std::string_view{}) };
};
