#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <functional>
#include <unordered_map>
#include <cstdint>

struct NetMessage {
    uint16_t type = 0;
    std::vector<uint8_t> payload;
};

class NetworkManager {
public:
    bool connect(std::string_view host, uint16_t port);
    bool host(uint16_t port);
    void disconnect();

    void send(const NetMessage& msg);
    void poll();

    void set_handler(uint16_t type, std::function<void(const NetMessage&)> handler);

    bool is_connected() const { return connected_; }
    bool is_hosting() const { return hosting_; }

private:
    bool connected_ = false;
    bool hosting_ = false;
    std::unordered_map<uint16_t, std::function<void(const NetMessage&)>> handlers_;
};
