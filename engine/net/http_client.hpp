#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>

namespace ergo::net {

// HTTP response data
struct HttpResponse {
    int status_code = 0;
    std::string reason;
    std::unordered_map<std::string, std::string> headers;
    std::vector<uint8_t> body;

    // Convenience: body as string
    std::string body_string() const {
        return std::string(body.begin(), body.end());
    }

    bool ok() const { return status_code >= 200 && status_code < 300; }
};

// HTTP client backed by POCO HTTPClientSession.
// Satisfies: HttpRequestable
class HttpClient {
public:
    HttpClient();
    ~HttpClient();

    // Move-only
    HttpClient(HttpClient&& other) noexcept;
    HttpClient& operator=(HttpClient&& other) noexcept;
    HttpClient(const HttpClient&) = delete;
    HttpClient& operator=(const HttpClient&) = delete;

    // Set connection timeout in milliseconds
    void set_timeout(int timeout_ms);

    // Synchronous HTTP methods
    HttpResponse get(std::string_view url);
    HttpResponse post(std::string_view url, std::string_view body,
                      std::string_view content_type = "application/json");
    HttpResponse put(std::string_view url, std::string_view body,
                     std::string_view content_type = "application/json");
    HttpResponse del(std::string_view url);

    // Async HTTP GET with callback (runs on internal thread)
    using ResponseCallback = std::function<void(HttpResponse)>;
    void get_async(std::string_view url, ResponseCallback callback);
    void post_async(std::string_view url, std::string_view body,
                    ResponseCallback callback,
                    std::string_view content_type = "application/json");

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace ergo::net
