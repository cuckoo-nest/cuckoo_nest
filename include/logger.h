// Simple lightweight header-only logger for cuckoo_nest
#pragma once

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <cstdlib>
#include <cstdarg>
#include <fstream>
#include <memory>

namespace cuckoo_log {

enum class Level { Trace = 0, Debug, Info, Warn, Error, Critical, Off };

inline const char* level_name(Level l) {
    switch (l) {
    case Level::Trace: return "TRACE";
    case Level::Debug: return "DEBUG";
    case Level::Info:  return "INFO";
    case Level::Warn:  return "WARN";
    case Level::Error: return "ERROR";
    case Level::Critical: return "CRITICAL";
    default: return "UNKNOWN";
    }
}

class Logger {
public:
    // C++11-compatible header-only storage: use function-local statics
    static Level &level_ref() {
        static Level lvl = Level::Info;
        return lvl;
    }

    static std::mutex &mtx_ref() {
        static std::mutex m;
        return m;
    }

    static std::unique_ptr<std::ofstream> &file_out_ref() {
        static std::unique_ptr<std::ofstream> f;
        return f;
    }

    static void log(Level lv, const std::string &msg) {
        if (lv < level_ref()) return;
        std::lock_guard<std::mutex> lk(mtx_ref());
        auto now = std::chrono::system_clock::now();
        auto t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        std::tm tm{};
#if defined(_WIN32)
        localtime_s(&tm, &t);
#else
        localtime_r(&t, &tm);
#endif
        char timebuf[64] = {0};
        if (std::strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", &tm) == 0) {
            timebuf[0] = '\0';
        }
        std::ostringstream oss;
        oss << timebuf << '.' << std::setfill('0') << std::setw(3) << ms.count()
            << " [" << level_name(lv) << "] " << msg << '\n';
        const std::string line = oss.str();
        if (lv >= Level::Error) std::cerr << line;
        else std::cout << line;

        auto &fo = file_out_ref();
        if (fo && fo->is_open()) {
            (*fo) << line;
            fo->flush();
        }
    }

    template<typename F>
    static void log_stream(Level lv, F &&f) {
        std::ostringstream oss;
        f(oss);
        log(lv, oss.str());
    }

    static void log_printf(Level lv, const char* fmt, ...) {
        if (lv < level_ref()) return;
        va_list ap;
        va_start(ap, fmt);
        int size = std::vsnprintf(nullptr, 0, fmt, ap);
        va_end(ap);
        if (size < 0) return;
        std::string buf(size + 1, '\0');
        va_start(ap, fmt);
        std::vsnprintf(&buf[0], buf.size(), fmt, ap);
        va_end(ap);
        if (!buf.empty() && buf.back() == '\0') buf.pop_back();
        log(lv, buf);
    }

    static void set_level(Level lv) { level_ref() = lv; }
    static void set_level_from_env(const char* envvar = "CUCKOO_LOG_LEVEL") {
        const char* v = std::getenv(envvar);
        if (!v) return;
        std::string s(v);
        for (auto &c : s) c = tolower(c);
        if (s == "trace") set_level(Level::Trace);
        else if (s == "debug") set_level(Level::Debug);
        else if (s == "info") set_level(Level::Info);
        else if (s == "warn" || s == "warning") set_level(Level::Warn);
        else if (s == "error") set_level(Level::Error);
        else if (s == "critical") set_level(Level::Critical);
    }

    static void set_file(const std::string &path) {
        try {
            // C++11 doesn't have make_unique
            file_out_ref().reset(new std::ofstream(path, std::ios::app));
            if (!file_out_ref()->is_open()) {
                file_out_ref().reset();
            }
        } catch (...) {
            file_out_ref().reset();
        }
    }

    static void set_file_from_env(const char* envvar = "CUCKOO_LOG_FILE") {
        const char* v = std::getenv(envvar);
        if (!v) return;
        set_file(std::string(v));
    }

    // Query whether a file sink is configured and open
    static bool file_enabled() {
        auto &fo = file_out_ref();
        return (fo && fo->is_open());
    }
};

} // namespace cuckoo_log

// Convenience macros
#define LOG_TRACE_STREAM(expr) cuckoo_log::Logger::log_stream(cuckoo_log::Level::Trace, [&](std::ostream &o){ o << expr; })
#define LOG_DEBUG_STREAM(expr) cuckoo_log::Logger::log_stream(cuckoo_log::Level::Debug, [&](std::ostream &o){ o << expr; })
#define LOG_INFO_STREAM(expr)  cuckoo_log::Logger::log_stream(cuckoo_log::Level::Info,  [&](std::ostream &o){ o << expr; })
#define LOG_WARN_STREAM(expr)  cuckoo_log::Logger::log_stream(cuckoo_log::Level::Warn,  [&](std::ostream &o){ o << expr; })
#define LOG_ERROR_STREAM(expr) cuckoo_log::Logger::log_stream(cuckoo_log::Level::Error, [&](std::ostream &o){ o << expr; })
#define LOG_CRIT_STREAM(expr)  cuckoo_log::Logger::log_stream(cuckoo_log::Level::Critical, [&](std::ostream &o){ o << expr; })

#define LOG_TRACE(fmt, ...) cuckoo_log::Logger::log_printf(cuckoo_log::Level::Trace, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) cuckoo_log::Logger::log_printf(cuckoo_log::Level::Debug, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)  cuckoo_log::Logger::log_printf(cuckoo_log::Level::Info,  fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  cuckoo_log::Logger::log_printf(cuckoo_log::Level::Warn,  fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) cuckoo_log::Logger::log_printf(cuckoo_log::Level::Error, fmt, ##__VA_ARGS__)
#define LOG_CRIT(fmt, ...)  cuckoo_log::Logger::log_printf(cuckoo_log::Level::Critical, fmt, ##__VA_ARGS__)
