#include "log.hpp"
#include <cstdio>
#include <cstdarg>
#include <ctime>
#include <mutex>

namespace {

LogLevel g_min_level = LogLevel::Info;
FILE* g_log_file = nullptr;
std::mutex g_log_mutex;

const char* level_str(LogLevel level) {
    switch (level) {
        case LogLevel::Trace: return "TRACE";
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info:  return "INFO ";
        case LogLevel::Warn:  return "WARN ";
        case LogLevel::Error: return "ERROR";
        case LogLevel::Fatal: return "FATAL";
    }
    return "?????";
}

const char* level_color(LogLevel level) {
    switch (level) {
        case LogLevel::Trace: return "\033[90m";
        case LogLevel::Debug: return "\033[36m";
        case LogLevel::Info:  return "\033[32m";
        case LogLevel::Warn:  return "\033[33m";
        case LogLevel::Error: return "\033[31m";
        case LogLevel::Fatal: return "\033[35;1m";
    }
    return "";
}

void log_impl(LogLevel level, const char* category, const char* fmt, va_list args) {
    if (level < g_min_level) return;

    std::lock_guard<std::mutex> lock(g_log_mutex);

    std::time_t now = std::time(nullptr);
    std::tm tm_buf;
#ifdef _WIN32
    localtime_s(&tm_buf, &now);
#else
    localtime_r(&now, &tm_buf);
#endif

    char time_str[32];
    std::strftime(time_str, sizeof(time_str), "%H:%M:%S", &tm_buf);

    // Console output with color
    std::fprintf(stderr, "%s[%s][%s][%s]\033[0m ",
                 level_color(level), time_str, level_str(level), category);
    std::vfprintf(stderr, fmt, args);
    std::fprintf(stderr, "\n");

    // File output without color
    if (g_log_file) {
        std::fprintf(g_log_file, "[%s][%s][%s] ", time_str, level_str(level), category);
        std::vfprintf(g_log_file, fmt, args);
        std::fprintf(g_log_file, "\n");
        std::fflush(g_log_file);
    }
}

} // anonymous namespace

namespace ergo::log {

void set_level(LogLevel min_level) {
    g_min_level = min_level;
}

void set_file(const char* path) {
    close_file();
    g_log_file = std::fopen(path, "w");
}

void close_file() {
    if (g_log_file) {
        std::fclose(g_log_file);
        g_log_file = nullptr;
    }
}

void trace(const char* category, const char* fmt, ...) {
    va_list args; va_start(args, fmt); log_impl(LogLevel::Trace, category, fmt, args); va_end(args);
}
void debug(const char* category, const char* fmt, ...) {
    va_list args; va_start(args, fmt); log_impl(LogLevel::Debug, category, fmt, args); va_end(args);
}
void info(const char* category, const char* fmt, ...) {
    va_list args; va_start(args, fmt); log_impl(LogLevel::Info, category, fmt, args); va_end(args);
}
void warn(const char* category, const char* fmt, ...) {
    va_list args; va_start(args, fmt); log_impl(LogLevel::Warn, category, fmt, args); va_end(args);
}
void error(const char* category, const char* fmt, ...) {
    va_list args; va_start(args, fmt); log_impl(LogLevel::Error, category, fmt, args); va_end(args);
}
void fatal(const char* category, const char* fmt, ...) {
    va_list args; va_start(args, fmt); log_impl(LogLevel::Fatal, category, fmt, args); va_end(args);
}

} // namespace ergo::log
