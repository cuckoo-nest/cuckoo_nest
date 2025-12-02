#pragma once
#include "../spdlog.h"

// Mock basic file sink for testing
namespace spdlog {
namespace sinks {
    class basic_file_sink_mt : public sink {
    public:
        basic_file_sink_mt(const std::string&) {}
        void set_level(spdlog::level log_level) {}
    };
}
}
