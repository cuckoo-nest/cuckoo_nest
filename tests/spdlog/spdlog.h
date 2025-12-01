// Minimal compatibility layer for test code that includes <spdlog/spdlog.h>
// Redirects to the lightweight `logger.h` so test output appears on the console.

#ifndef MOCK_SPDLOG_HPP
#define MOCK_SPDLOG_HPP

#include "../include/logger.h"
#include <memory>
#include <string>
#include <cstdarg>
#include <vector>

namespace spdlog {

enum class level {
    trace = 0,
    debug = 1,
    info = 2,
    warn = 3,
    err = 4,
    critical = 5,
    off = 6
};

class logger {
public:
    logger(const std::string &n) {}
    template<typename... Args>
    void trace(const char* fmt, const Args&... args) { cuckoo_log::Logger::log_printf(cuckoo_log::Level::Trace, fmt, args...); }
    template<typename... Args>
    void debug(const char* fmt, const Args&... args) { cuckoo_log::Logger::log_printf(cuckoo_log::Level::Debug, fmt, args...); }
    template<typename... Args>
    void info(const char* fmt, const Args&... args) { cuckoo_log::Logger::log_printf(cuckoo_log::Level::Info, fmt, args...); }
    template<typename... Args>
    void warn(const char* fmt, const Args&... args) { cuckoo_log::Logger::log_printf(cuckoo_log::Level::Warn, fmt, args...); }
    template<typename... Args>
    void error(const char* fmt, const Args&... args) { cuckoo_log::Logger::log_printf(cuckoo_log::Level::Error, fmt, args...); }
    template<typename... Args>
    void critical(const char* fmt, const Args&... args) { cuckoo_log::Logger::log_printf(cuckoo_log::Level::Critical, fmt, args...); }
    void set_level(level) {}
    void flush() {}
};

inline std::shared_ptr<logger> get(const std::string& name) { return std::make_shared<logger>(name); }
inline std::shared_ptr<logger> default_logger() { static auto l = std::make_shared<logger>("default"); return l; }
inline void set_default_logger(std::shared_ptr<logger>) {}
inline void set_level(level) {}
inline void flush_on(level) {}
inline void flush_every(int) {}

template<typename... Args>
inline void trace(const char* fmt, const Args&... args) { cuckoo_log::Logger::log_printf(cuckoo_log::Level::Trace, fmt, args...); }
template<typename... Args>
inline void debug(const char* fmt, const Args&... args) { cuckoo_log::Logger::log_printf(cuckoo_log::Level::Debug, fmt, args...); }
template<typename... Args>
inline void info(const char* fmt, const Args&... args) { cuckoo_log::Logger::log_printf(cuckoo_log::Level::Info, fmt, args...); }
template<typename... Args>
inline void warn(const char* fmt, const Args&... args) { cuckoo_log::Logger::log_printf(cuckoo_log::Level::Warn, fmt, args...); }
template<typename... Args>
inline void error(const char* fmt, const Args&... args) { cuckoo_log::Logger::log_printf(cuckoo_log::Level::Error, fmt, args...); }
template<typename... Args>
inline void critical(const char* fmt, const Args&... args) { cuckoo_log::Logger::log_printf(cuckoo_log::Level::Critical, fmt, args...); }

} // namespace spdlog

#endif // MOCK_SPDLOG_HPP
