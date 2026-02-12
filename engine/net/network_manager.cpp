#include "network_manager.hpp"
#include "tcp_socket.hpp"

#include <unordered_map>
#include <vector>
#include <mutex>
#include <thread>
#include <atomic>
#include <cstring>

namespace ergo::net {

// ============================================================
// Wire protocol
// ============================================================
// Each message on the wire: [uint16_t type][uint32_t payload_len][payload...]
static constexpr size_t HEADER_SIZE = sizeof(uint16_t) + sizeof(uint32_t);
static constexpr size_t MAX_PAYLOAD_SIZE = 1024 * 1024; // 1 MB

// ============================================================
// Pimpl implementation
// ============================================================

struct NetworkManager::Impl {
    enum class Mode { None, Client, Server };

    Mode mode = Mode::None;
    std::atomic<bool> active{false};

    // --- Client state ---
    TcpSocket client_socket;

    // --- Server state ---
    TcpSocket server_socket;
    struct ClientInfo {
        uint32_t id;
        TcpSocket socket;
        std::vector<uint8_t> recv_buffer;
    };
    std::vector<std::unique_ptr<ClientInfo>> clients;
    uint32_t next_client_id = 1;
    int max_clients = 16;
    std::thread accept_thread;

    // --- Receive buffer (client mode) ---
    std::vector<uint8_t> recv_buffer;

    // --- Handlers ---
    std::unordered_map<uint16_t, MessageHandler> handlers;
    EventHandler event_handler;
    std::mutex handler_mutex;

    // --- Helpers ---

    static std::vector<uint8_t> encode_message(const NetMessage& msg) {
        std::vector<uint8_t> wire(HEADER_SIZE + msg.payload.size());
        uint16_t type_net = msg.type;
        uint32_t len_net = static_cast<uint32_t>(msg.payload.size());
        std::memcpy(wire.data(), &type_net, sizeof(type_net));
        std::memcpy(wire.data() + sizeof(uint16_t), &len_net, sizeof(len_net));
        if (!msg.payload.empty()) {
            std::memcpy(wire.data() + HEADER_SIZE, msg.payload.data(), msg.payload.size());
        }
        return wire;
    }

    // Try to extract complete messages from a buffer.
    // Returns extracted messages; consumed bytes are removed from buffer.
    static std::vector<NetMessage> extract_messages(std::vector<uint8_t>& buffer) {
        std::vector<NetMessage> messages;
        size_t offset = 0;
        while (offset + HEADER_SIZE <= buffer.size()) {
            uint16_t type;
            uint32_t len;
            std::memcpy(&type, buffer.data() + offset, sizeof(type));
            std::memcpy(&len, buffer.data() + offset + sizeof(uint16_t), sizeof(len));

            if (len > MAX_PAYLOAD_SIZE) {
                // Protocol error: skip all data
                buffer.clear();
                return messages;
            }

            if (offset + HEADER_SIZE + len > buffer.size()) {
                break; // incomplete message
            }

            NetMessage msg;
            msg.type = type;
            msg.payload.resize(len);
            if (len > 0) {
                std::memcpy(msg.payload.data(),
                            buffer.data() + offset + HEADER_SIZE, len);
            }
            messages.push_back(std::move(msg));
            offset += HEADER_SIZE + len;
        }
        // Remove consumed bytes
        if (offset > 0) {
            buffer.erase(buffer.begin(), buffer.begin() + static_cast<ptrdiff_t>(offset));
        }
        return messages;
    }

    void dispatch(uint32_t client_id, const NetMessage& msg) {
        std::lock_guard<std::mutex> lock(handler_mutex);
        auto it = handlers.find(msg.type);
        if (it != handlers.end() && it->second) {
            it->second(client_id, msg);
        }
    }

    void fire_event(uint32_t client_id, Event event) {
        std::lock_guard<std::mutex> lock(handler_mutex);
        if (event_handler) {
            event_handler(client_id, event);
        }
    }

    void send_to_socket(TcpSocket& sock, const NetMessage& msg) {
        auto wire = encode_message(msg);
        sock.send(wire.data(), wire.size());
    }

    // Poll a single socket's receive buffer
    void poll_socket(TcpSocket& sock, std::vector<uint8_t>& buf,
                     uint32_t client_id) {
        uint8_t tmp[4096];
        int n = sock.recv(tmp, sizeof(tmp));
        if (n > 0) {
            buf.insert(buf.end(), tmp, tmp + n);
            auto msgs = extract_messages(buf);
            for (auto& m : msgs) {
                dispatch(client_id, m);
            }
        } else if (n < 0) {
            fire_event(client_id, Event::Disconnected);
        }
    }

    void run_accept_loop() {
        server_socket.set_non_blocking(true);
        while (active.load()) {
            auto client = server_socket.accept();
            if (client.is_connected()) {
                uint32_t id = next_client_id++;
                auto info = std::make_unique<ClientInfo>();
                info->id = id;
                info->socket = std::move(client);
                info->socket.set_non_blocking(true);
                info->socket.set_timeout(0);
                clients.push_back(std::move(info));
                fire_event(id, Event::Connected);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
};

// ============================================================
// Construction / Move
// ============================================================

NetworkManager::NetworkManager() : impl_(std::make_unique<Impl>()) {}

NetworkManager::~NetworkManager() { shutdown(); }

NetworkManager::NetworkManager(NetworkManager&&) noexcept = default;
NetworkManager& NetworkManager::operator=(NetworkManager&&) noexcept = default;

// ============================================================
// Client mode
// ============================================================

bool NetworkManager::connect(std::string_view host, uint16_t port) {
    if (impl_->active.load()) return false;

    if (!impl_->client_socket.connect(host, port)) {
        return false;
    }
    impl_->client_socket.set_non_blocking(true);
    impl_->client_socket.set_timeout(0);
    impl_->mode = Impl::Mode::Client;
    impl_->active.store(true);
    impl_->fire_event(0, Event::Connected);
    return true;
}

// ============================================================
// Server mode
// ============================================================

bool NetworkManager::host_server(uint16_t port, int max_clients) {
    if (impl_->active.load()) return false;

    if (!impl_->server_socket.listen(port)) {
        return false;
    }
    impl_->max_clients = max_clients;
    impl_->mode = Impl::Mode::Server;
    impl_->active.store(true);

    // Start background accept thread
    impl_->accept_thread = std::thread([this]() {
        impl_->run_accept_loop();
    });

    return true;
}

// ============================================================
// Send
// ============================================================

void NetworkManager::send(const NetMessage& msg, uint32_t client_id) {
    if (!impl_->active.load()) return;

    if (impl_->mode == Impl::Mode::Client) {
        impl_->send_to_socket(impl_->client_socket, msg);
    } else if (impl_->mode == Impl::Mode::Server) {
        if (client_id == 0) {
            // Broadcast
            for (auto& c : impl_->clients) {
                impl_->send_to_socket(c->socket, msg);
            }
        } else {
            for (auto& c : impl_->clients) {
                if (c->id == client_id) {
                    impl_->send_to_socket(c->socket, msg);
                    break;
                }
            }
        }
    }
}

// ============================================================
// Poll (call each frame)
// ============================================================

void NetworkManager::poll() {
    if (!impl_->active.load()) return;

    if (impl_->mode == Impl::Mode::Client) {
        impl_->poll_socket(impl_->client_socket, impl_->recv_buffer, 0);
    } else if (impl_->mode == Impl::Mode::Server) {
        // Poll all connected clients
        for (auto it = impl_->clients.begin(); it != impl_->clients.end();) {
            auto& c = *it;
            if (!c->socket.is_connected()) {
                impl_->fire_event(c->id, Event::Disconnected);
                it = impl_->clients.erase(it);
            } else {
                impl_->poll_socket(c->socket, c->recv_buffer, c->id);
                ++it;
            }
        }
    }
}

// ============================================================
// Handlers
// ============================================================

void NetworkManager::set_handler(uint16_t msg_type, MessageHandler handler) {
    std::lock_guard<std::mutex> lock(impl_->handler_mutex);
    impl_->handlers[msg_type] = std::move(handler);
}

void NetworkManager::set_event_handler(EventHandler handler) {
    std::lock_guard<std::mutex> lock(impl_->handler_mutex);
    impl_->event_handler = std::move(handler);
}

// ============================================================
// Shutdown
// ============================================================

void NetworkManager::shutdown() {
    if (!impl_) return;
    impl_->active.store(false);

    if (impl_->accept_thread.joinable()) {
        impl_->accept_thread.join();
    }

    impl_->client_socket.close();
    impl_->server_socket.close();
    for (auto& c : impl_->clients) {
        c->socket.close();
    }
    impl_->clients.clear();
    impl_->recv_buffer.clear();
    impl_->mode = Impl::Mode::None;
}

// ============================================================
// Queries
// ============================================================

bool NetworkManager::is_active() const { return impl_->active.load(); }
bool NetworkManager::is_server() const { return impl_->mode == Impl::Mode::Server; }
bool NetworkManager::is_client() const { return impl_->mode == Impl::Mode::Client; }

uint32_t NetworkManager::client_count() const {
    return static_cast<uint32_t>(impl_->clients.size());
}

} // namespace ergo::net
