#include "http_client.hpp"

#include <thread>
#include <sstream>

#if ERGO_HAS_POCO
// ============================================================
// POCO HTTP backend
// ============================================================
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/URI.h>
#include <Poco/Timespan.h>
#include <Poco/StreamCopier.h>
#include <Poco/Exception.h>

namespace ergo::net {

struct HttpClient::Impl {
    int timeout_ms = 10000;

    HttpResponse execute(const std::string& method,
                         std::string_view url,
                         std::string_view body = {},
                         std::string_view content_type = {}) {
        HttpResponse result;
        try {
            Poco::URI uri(std::string(url));
            std::string path = uri.getPathAndQuery();
            if (path.empty()) path = "/";

            Poco::Net::HTTPClientSession session(uri.getHost(), uri.getPort());
            Poco::Timespan ts(0, timeout_ms * 1000);
            session.setTimeout(ts);

            Poco::Net::HTTPRequest request(method, path,
                                            Poco::Net::HTTPRequest::HTTP_1_1);
            request.setHost(uri.getHost());

            if (!body.empty()) {
                request.setContentLength(static_cast<int>(body.size()));
                if (!content_type.empty())
                    request.setContentType(std::string(content_type));
                auto& os = session.sendRequest(request);
                os.write(body.data(), static_cast<std::streamsize>(body.size()));
            } else {
                session.sendRequest(request);
            }

            Poco::Net::HTTPResponse response;
            auto& is = session.receiveResponse(response);
            result.status_code = response.getStatus();
            result.reason = response.getReason();
            for (const auto& [key, value] : response)
                result.headers[key] = value;

            std::ostringstream oss;
            Poco::StreamCopier::copyStream(is, oss);
            std::string body_str = oss.str();
            result.body.assign(body_str.begin(), body_str.end());
        } catch (const Poco::Exception& e) {
            result.status_code = -1;
            result.reason = e.displayText();
        }
        return result;
    }
};

} // namespace ergo::net

#else
// ============================================================
// POSIX HTTP backend (minimal HTTP/1.1 client)
// ============================================================
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <string>

namespace ergo::net {

struct HttpClient::Impl {
    int timeout_ms = 10000;

    struct ParsedUrl {
        std::string host;
        uint16_t port = 80;
        std::string path;
    };

    static ParsedUrl parse_url(std::string_view url) {
        ParsedUrl result;
        // Skip "http://"
        std::string_view rest = url;
        if (rest.starts_with("http://")) rest.remove_prefix(7);
        else if (rest.starts_with("https://")) rest.remove_prefix(8);

        auto slash = rest.find('/');
        std::string_view host_port = (slash != std::string_view::npos) ? rest.substr(0, slash) : rest;
        result.path = (slash != std::string_view::npos) ? std::string(rest.substr(slash)) : "/";

        auto colon = host_port.find(':');
        if (colon != std::string_view::npos) {
            result.host = std::string(host_port.substr(0, colon));
            result.port = static_cast<uint16_t>(
                std::stoi(std::string(host_port.substr(colon + 1))));
        } else {
            result.host = std::string(host_port);
        }
        return result;
    }

    HttpResponse execute(const std::string& method,
                         std::string_view url,
                         std::string_view body = {},
                         std::string_view content_type = {}) {
        HttpResponse result;
        auto parsed = parse_url(url);

        int fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) {
            result.status_code = -1;
            result.reason = "socket() failed";
            return result;
        }

        // Set timeout
        struct timeval tv;
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;
        ::setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        ::setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

        // Resolve and connect
        struct addrinfo hints{}, *res = nullptr;
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        std::string port_str = std::to_string(parsed.port);
        if (::getaddrinfo(parsed.host.c_str(), port_str.c_str(), &hints, &res) != 0 || !res) {
            ::close(fd);
            result.status_code = -1;
            result.reason = "DNS resolution failed";
            return result;
        }
        if (::connect(fd, res->ai_addr, res->ai_addrlen) < 0) {
            ::freeaddrinfo(res);
            ::close(fd);
            result.status_code = -1;
            result.reason = "connect() failed";
            return result;
        }
        ::freeaddrinfo(res);

        // Build HTTP request
        std::ostringstream req;
        req << method << " " << parsed.path << " HTTP/1.1\r\n";
        req << "Host: " << parsed.host << "\r\n";
        req << "Connection: close\r\n";
        if (!body.empty()) {
            req << "Content-Length: " << body.size() << "\r\n";
            if (!content_type.empty())
                req << "Content-Type: " << content_type << "\r\n";
        }
        req << "\r\n";
        if (!body.empty()) req << body;

        std::string request_str = req.str();
        ::send(fd, request_str.data(), request_str.size(), MSG_NOSIGNAL);

        // Read response
        std::string response_data;
        char buf[4096];
        ssize_t n;
        while ((n = ::recv(fd, buf, sizeof(buf), 0)) > 0) {
            response_data.append(buf, static_cast<size_t>(n));
        }
        ::close(fd);

        // Parse response
        auto header_end = response_data.find("\r\n\r\n");
        if (header_end == std::string::npos) {
            result.status_code = -1;
            result.reason = "Malformed HTTP response";
            return result;
        }

        // Parse status line: "HTTP/1.1 200 OK"
        auto first_line_end = response_data.find("\r\n");
        std::string status_line = response_data.substr(0, first_line_end);
        auto sp1 = status_line.find(' ');
        auto sp2 = status_line.find(' ', sp1 + 1);
        if (sp1 != std::string::npos && sp2 != std::string::npos) {
            result.status_code = std::stoi(status_line.substr(sp1 + 1, sp2 - sp1 - 1));
            result.reason = status_line.substr(sp2 + 1);
        }

        // Parse headers
        std::string header_block = response_data.substr(first_line_end + 2,
                                                         header_end - first_line_end - 2);
        std::istringstream hs(header_block);
        std::string line;
        while (std::getline(hs, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            auto colon = line.find(':');
            if (colon != std::string::npos) {
                std::string key = line.substr(0, colon);
                std::string value = line.substr(colon + 1);
                // Trim leading space
                if (!value.empty() && value[0] == ' ') value.erase(0, 1);
                result.headers[key] = value;
            }
        }

        // Body
        std::string body_str = response_data.substr(header_end + 4);
        result.body.assign(body_str.begin(), body_str.end());
        return result;
    }
};

} // namespace ergo::net

#endif // ERGO_HAS_POCO

// ============================================================
// Common interface (shared between both backends)
// ============================================================

namespace ergo::net {

HttpClient::HttpClient() : impl_(std::make_unique<Impl>()) {}
HttpClient::~HttpClient() = default;
HttpClient::HttpClient(HttpClient&& other) noexcept = default;
HttpClient& HttpClient::operator=(HttpClient&& other) noexcept = default;

void HttpClient::set_timeout(int timeout_ms) { impl_->timeout_ms = timeout_ms; }

HttpResponse HttpClient::get(std::string_view url) {
    return impl_->execute("GET", url);
}

HttpResponse HttpClient::post(std::string_view url, std::string_view body,
                               std::string_view content_type) {
    return impl_->execute("POST", url, body, content_type);
}

HttpResponse HttpClient::put(std::string_view url, std::string_view body,
                              std::string_view content_type) {
    return impl_->execute("PUT", url, body, content_type);
}

HttpResponse HttpClient::del(std::string_view url) {
    return impl_->execute("DELETE", url);
}

void HttpClient::get_async(std::string_view url, ResponseCallback callback) {
    std::string url_copy(url);
    int timeout = impl_->timeout_ms;
    std::thread([url_copy, callback, timeout]() {
        HttpClient client;
        client.set_timeout(timeout);
        auto response = client.get(url_copy);
        if (callback) callback(std::move(response));
    }).detach();
}

void HttpClient::post_async(std::string_view url, std::string_view body,
                             ResponseCallback callback,
                             std::string_view content_type) {
    std::string url_copy(url);
    std::string body_copy(body);
    std::string ct_copy(content_type);
    int timeout = impl_->timeout_ms;
    std::thread([url_copy, body_copy, ct_copy, callback, timeout]() {
        HttpClient client;
        client.set_timeout(timeout);
        auto response = client.post(url_copy, body_copy, ct_copy);
        if (callback) callback(std::move(response));
    }).detach();
}

} // namespace ergo::net
