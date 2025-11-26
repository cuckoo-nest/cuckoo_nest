#pragma once
#include "../spdlog.h"

// Mock stdout color sinks for testing
namespace spdlog {
namespace sinks {
    class stdout_color_sink_mt : public spdlog::sinks::sink {
    public:
        void set_level(spdlog::level log_level) override {}
        void log(const spdlog::details::log_msg&) override {}
        void flush() override {}
    };
}
}
