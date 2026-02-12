#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <functional>
#include <memory>
#include "net_concepts.hpp"

namespace ergo::net {

// High-level network manager for game networking.
// Provides message-based communication over TCP.
// Satisfies: NetworkManageable
class NetworkManager {
public:
    // Connection event types
    enum class Event : uint8_t {
        Connected,
        Disconnected,
        Error,
    };

    using MessageHandler = std::function<void(uint32_t client_id, const NetMessage&)>;
    using EventHandler = std::function<void(uint32_t client_id, Event event)>;

    NetworkManager();
    ~NetworkManager();

    // Move-only
    NetworkManager(NetworkManager&&) noexcept;
    NetworkManager& operator=(NetworkManager&&) noexcept;
    NetworkManager(const NetworkManager&) = delete;
    NetworkManager& operator=(const NetworkManager&) = delete;

    // --- Client mode ---
    bool connect(std::string_view host, uint16_t port);

    // --- Server mode ---
    bool host_server(uint16_t port, int max_clients = 16);

    // --- Common ---

    // Send a message. In client mode, sends to server.
    // In server mode, sends to a specific client (or broadcast if client_id == 0).
    void send(const NetMessage& msg, uint32_t client_id = 0);

    // Process incoming data. Call once per frame from game loop.
    void poll();

    // Register a handler for a specific message type
    void set_handler(uint16_t msg_type, MessageHandler handler);

    // Register an event handler (connect/disconnect/error)
    void set_event_handler(EventHandler handler);

    // Disconnect and clean up
    void shutdown();

    // State queries
    bool is_active() const;
    bool is_server() const;
    bool is_client() const;
    uint32_t client_count() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace ergo::net
