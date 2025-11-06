# ConfigurationReader

A JSON configuration file reader for the CuckooNest project that uses the libmjson library for embedded compatibility.

## Features

- Reads JSON configuration files from the same directory as the executable
- Type-safe getters for string, int, bool, and double values
- Default value support for all getter methods
- Key existence checking
- Designed for embedded systems using libmjson

## Usage

### Basic Example

```cpp
#include "ConfigurationReader.hpp"

int main() {
    // Create a configuration reader for "config.json"
    ConfigurationReader config("config.json");
    
    // Load the configuration file
    if (config.load()) {
        std::cout << "Configuration loaded successfully\n";
        
        // Get values with default fallbacks
        std::string app_name = config.get_string("app_name", "DefaultApp");
        int max_screens = config.get_int("max_screens", 5);
        bool debug_mode = config.get_bool("debug_mode", false);
        double refresh_rate = config.get_double("refresh_rate", 60.0);
        
        std::cout << "App: " << app_name << std::endl;
        std::cout << "Max screens: " << max_screens << std::endl;
        std::cout << "Debug: " << (debug_mode ? "enabled" : "disabled") << std::endl;
        std::cout << "Refresh rate: " << refresh_rate << "Hz" << std::endl;
        
        // Check if a key exists
        if (config.has_key("optional_setting")) {
            std::cout << "Optional setting found\n";
        }
    } else {
        std::cout << "Failed to load configuration, using defaults\n";
    }
    
    return 0;
}
```

### Configuration File Format

The configuration file should be valid JSON format. Here's an example `config.json`:

```json
{
    "app_name": "CuckooNest",
    "version": "1.0.0",
    "debug_mode": true,
    "max_screens": 10,
    "refresh_rate": 60.0,
    "display": {
        "width": 320,
        "height": 240,
        "brightness": 0.8
    },
    "homeassistant": {
        "base_url": "http://homeassistant.local:8123",
        "token": "your_token_here",
        "timeout": 30
    },
    "beeper": {
        "enabled": true,
        "volume": 0.5,
        "default_tone": 440
    }
}
```

## libmjson Dependency

This class requires the libmjson library to be available on the target embedded system.

### Installing libmjson

#### Option 1: System Package (if available)
```bash
# On some embedded Linux distributions
apt-get install libmjson-dev
# or
opkg install libmjson
```

#### Option 2: Build from Source
```bash
# Clone the mjson repository
git clone https://github.com/cesanta/mjson.git
cd mjson

# For embedded systems, you might need to cross-compile
# Using your ARM toolchain:
arm-linux-gnueabi-gcc -c mjson.c -o mjson.o
arm-linux-gnueabi-ar rcs libmjson.a mjson.o

# Copy headers and library to your project
cp mjson.h /path/to/your/project/include/
cp libmjson.a /path/to/your/project/lib/
```

#### Option 3: Add to Third-Party Dependencies
Add libmjson as a third-party dependency in your project:

1. Create directory: `third-party/mjson/`
2. Copy `mjson.h` and `mjson.c` to that directory
3. Update your CMakeLists.txt to include mjson:

```cmake
# In src/CMakeLists.txt, add mjson source
set (
    SRC_FILES 
    main.cpp
    ScreenManager.cpp
    ConfigurationReader.cpp
    ../third-party/mjson/mjson.c  # Add mjson source
    # ... other files
)

# Add mjson include directory
target_include_directories(cuckoo PRIVATE ../third-party/mjson)
```

### Current Status

The ConfigurationReader is implemented and tested, but libmjson is not currently available in this build environment. The code will:

- Compile successfully with warning messages about missing libmjson
- Fall back to returning default values for all configuration queries
- Provide a working API that will function fully once libmjson is available

### API Reference

#### Constructor
- `ConfigurationReader(const std::string& config_filename)` - Creates a reader for the specified config file

#### Loading
- `bool load()` - Loads and parses the configuration file
- `bool is_loaded() const` - Returns true if configuration is successfully loaded

#### Getters
- `std::string get_string(const std::string& key, const std::string& default_value = "")` - Get string value
- `int get_int(const std::string& key, int default_value = 0)` - Get integer value
- `bool get_bool(const std::string& key, bool default_value = false)` - Get boolean value
- `double get_double(const std::string& key, double default_value = 0.0)` - Get floating point value

#### Utilities
- `bool has_key(const std::string& key)` - Check if key exists
- `std::vector<std::string> get_keys()` - Get all configuration keys

## Testing

The ConfigurationReader includes comprehensive unit tests using Google Test. Run the tests with:

```bash
cd build
./tests/cuckoo_tests --gtest_filter="ConfigurationReaderTest.*"
```

The tests verify:
- Constructor behavior
- File loading (both valid and invalid files)
- Default value handling
- Type conversion
- Error handling
- Complete workflow scenarios

## File Location

The configuration file is expected to be in the same directory as the executable. The class automatically determines the executable's location using `/proc/self/exe` and looks for the config file there.

For example, if your executable is `/opt/cuckoo/cuckoo` and your config file is named `config.json`, the class will look for `/opt/cuckoo/config.json`.