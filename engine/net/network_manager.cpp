#include "network_manager.hpp"
#include "socket.hpp"

bool NetworkManager::connect(std::string_view /*host*/, uint16_t /*port*/) {
    // TCP connection would be established here
    connected_ = false;
    return connected_;
}

bool NetworkManager::host(uint16_t /*port*/) {
    // TCP server would start listening here
    hosting_ = false;
    return hosting_;
}

void NetworkManager::disconnect() {
    connected_ = false;
    hosting_ = false;
}

void NetworkManager::send(const NetMessage& /*msg*/) {
    if (!connected_ && !hosting_) return;
    // Serialize and send via socket
}

void NetworkManager::poll() {
    // Read from socket, deserialize, dispatch to handlers
}

void NetworkManager::set_handler(uint16_t type, std::function<void(const NetMessage&)> handler) {
    handlers_[type] = std::move(handler);
}
