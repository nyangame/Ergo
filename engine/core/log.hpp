#pragma once
#include <cstdio>
#include <cstdarg>
#include <cstdint>

enum class LogLevel : uint8_t {
    Trace, Debug, Info, Warn, Error, Fatal
};

namespace ergo::log {

void set_level(LogLevel min_level);
void set_file(const char* path);
void close_file();

void trace(const char* category, const char* fmt, ...);
void debug(const char* category, const char* fmt, ...);
void info(const char* category, const char* fmt, ...);
void warn(const char* category, const char* fmt, ...);
void error(const char* category, const char* fmt, ...);
void fatal(const char* category, const char* fmt, ...);

} // namespace ergo::log

#define ERGO_LOG_TRACE(cat, ...) ergo::log::trace(cat, __VA_ARGS__)
#define ERGO_LOG_DEBUG(cat, ...) ergo::log::debug(cat, __VA_ARGS__)
#define ERGO_LOG_INFO(cat, ...)  ergo::log::info(cat, __VA_ARGS__)
#define ERGO_LOG_WARN(cat, ...)  ergo::log::warn(cat, __VA_ARGS__)
#define ERGO_LOG_ERROR(cat, ...) ergo::log::error(cat, __VA_ARGS__)
#define ERGO_LOG_FATAL(cat, ...) ergo::log::fatal(cat, __VA_ARGS__)
