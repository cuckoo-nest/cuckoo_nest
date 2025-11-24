#ifndef MOCK_SPDLOG_HPP
#define MOCK_SPDLOG_HPP

// Mock spdlog for unit tests
// This provides a minimal interface compatible with spdlog to avoid dependencies in tests

#include <string>
#include <memory>
#include <iostream>
#include <chrono>

namespace spdlog {

// Mock log levels
enum class level {
    trace = 0,
    debug = 1,
    info = 2,
    warn = 3,
    err = 4,
    critical = 5,
    off = 6
};

// Mock logger class
class logger {
public:
    logger(const std::string& name) : name_(name) {}
    
    template<typename... Args>
    void trace(const char* fmt, const Args&... args) {
        // No-op in tests
    }
    
    template<typename... Args>
    void debug(const char* fmt, const Args&... args) {
        // No-op in tests
    }
    
    template<typename... Args>
    void info(const char* fmt, const Args&... args) {
        // No-op in tests
    }
    
    template<typename... Args>
    void warn(const char* fmt, const Args&... args) {
        // No-op in tests
    }
    
    template<typename... Args>
    void error(const char* fmt, const Args&... args) {
        // No-op in tests
    }
    
    template<typename... Args>
    void critical(const char* fmt, const Args&... args) {
        // No-op in tests
    }
    
    void set_level(level log_level) {
        // No-op in tests
    }
    
    void flush() {
        // No-op in tests
    }

private:
    std::string name_;
};

// Mock registry functions
inline std::shared_ptr<logger> get(const std::string& name) {
    return std::make_shared<logger>(name);
}

inline std::shared_ptr<logger> default_logger() {
    static auto logger = std::make_shared<spdlog::logger>("default");
    return logger;
}

inline void set_default_logger(std::shared_ptr<logger> logger) {
    // No-op in tests
}

inline void set_level(level log_level) {
    // No-op in tests
}

inline void flush_on(level log_level) {
    // No-op in tests
}

inline void flush_every(std::chrono::seconds interval) {
    // No-op in tests
}

// Mock sink for creating loggers
namespace sinks {
    class sink {
    public:
        virtual ~sink() = default;
    };
    using sink_ptr = std::shared_ptr<sink>;
}

// Mock logger creation functions
template<typename... Args>
inline std::shared_ptr<logger> stdout_color_mt(const std::string& logger_name) {
    return std::make_shared<logger>(logger_name);
}

template<typename... Args>
inline std::shared_ptr<logger> basic_logger_mt(const std::string& logger_name, const std::string& filename) {
    return std::make_shared<logger>(logger_name);
}

template<typename... Args>
inline std::shared_ptr<logger> rotating_logger_mt(
    const std::string& logger_name,
    const std::string& filename,
    size_t max_file_size,
    size_t max_files) {
    return std::make_shared<logger>(logger_name);
}

// Mock free functions for logging (what the source code actually uses)
template<typename... Args>
inline void trace(const char* fmt, const Args&... args) {
    // No-op in tests
}

template<typename... Args>
inline void debug(const char* fmt, const Args&... args) {
    // No-op in tests
}

template<typename... Args>
inline void info(const char* fmt, const Args&... args) {
    // No-op in tests
}

template<typename... Args>
inline void warn(const char* fmt, const Args&... args) {
    // No-op in tests
}

template<typename... Args>
inline void error(const char* fmt, const Args&... args) {
    // No-op in tests
}

template<typename... Args>
inline void critical(const char* fmt, const Args&... args) {
    // No-op in tests
}

} // namespace spdlog

#endif // MOCK_SPDLOG_HPP
