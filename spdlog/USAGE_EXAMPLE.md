# spdlog Usage Example for Cuckoo Nest Project

## Overview
spdlog has been integrated into this project in **header-only mode** for the ARM build and with a **mock implementation** for unit tests.

## Using spdlog in Source Code

### Basic Usage

```cpp
#include <spdlog/spdlog.h>

int main() {
    // Simple logging
    spdlog::info("Application started");
    spdlog::warn("Warning message");
    spdlog::error("Error occurred: {}", error_code);
    spdlog::debug("Debug info: x={}, y={}", x, y);
    
    return 0;
}
```

### Creating a Logger with File Output

```cpp
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

void setup_logging() {
    // Create a file logger
    auto file_logger = spdlog::basic_logger_mt("file_logger", "logs/app.log");
    spdlog::set_default_logger(file_logger);
    
    // Set log level
    spdlog::set_level(spdlog::level::debug);
    
    spdlog::info("Logging to file started");
}
```

### Multiple Sinks (Console + File)

```cpp
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

void setup_multi_sink_logging() {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/app.log");
    
    std::vector<spdlog::sink_ptr> sinks {console_sink, file_sink};
    auto logger = std::make_shared<spdlog::logger>("multi_sink", sinks.begin(), sinks.end());
    
    spdlog::set_default_logger(logger);
    spdlog::info("Logging to console and file");
}
```

### Format Patterns

```cpp
#include <spdlog/spdlog.h>

void setup_custom_pattern() {
    // Pattern: [2024-11-20 10:30:45.123] [info] Message
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
    
    spdlog::info("Custom formatted message");
}
```

## Configuration

### ARM Build (Production)
- Uses the real spdlog library (header-only mode)
- Full functionality available
- Include: `#include <spdlog/spdlog.h>`
- Compile flag: `SPDLOG_HEADER_ONLY` (automatically set)

### Unit Tests
- Uses mock spdlog from `tests/spdlog/spdlog.h`
- All logging functions are no-ops
- No external dependencies required
- Compatible with the real spdlog API

## Log Levels

From lowest to highest severity:
1. `spdlog::trace()` - Very detailed debug info
2. `spdlog::debug()` - Debug information
3. `spdlog::info()` - Informational messages
4. `spdlog::warn()` - Warnings
5. `spdlog::error()` - Errors
6. `spdlog::critical()` - Critical errors

## Performance Tips

1. **Use header-only mode** (already configured)
2. **Set appropriate log level** in production:
   ```cpp
   spdlog::set_level(spdlog::level::info);  // Skip debug/trace in production
   ```

3. **Async logging** for high-performance scenarios:
   ```cpp
   #include <spdlog/async.h>
   auto async_logger = spdlog::create_async<spdlog::sinks::stdout_color_sink_mt>("async_logger");
   ```

## Documentation

For full documentation, visit: https://github.com/gabime/spdlog
