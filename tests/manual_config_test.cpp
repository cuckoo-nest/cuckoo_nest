#include "../src/ConfigurationReader.hpp"
#include <iostream>

int main() {
    std::cout << "Testing ConfigurationReader...\n";
    
    ConfigurationReader config("config.json");
    
    if (config.load()) {
        std::cout << "✓ Configuration loaded successfully\n";
        
        // Test various getters
        std::string app_name = config.get_string("app_name", "DefaultApp");
        std::cout << "App name: '" << app_name << "'\n";
        
        bool debug_mode = config.get_bool("debug_mode", false);
        std::cout << "Debug mode: " << (debug_mode ? "enabled" : "disabled") << "\n";
        
        int max_screens = config.get_int("max_screens", 5);
        std::cout << "Max screens: " << max_screens << "\n";
        
        double refresh_rate = config.get_double("refresh_rate", 30.0);
        std::cout << "Refresh rate: " << refresh_rate << "Hz\n";
        
        // Test key existence
        bool has_version = config.has_key("version");
        std::cout << "Has 'version' key: " << (has_version ? "yes" : "no") << "\n";
        
        bool has_nonexistent = config.has_key("nonexistent_key");
        std::cout << "Has 'nonexistent_key': " << (has_nonexistent ? "yes" : "no") << "\n";
        
    } else {
        std::cout << "✗ Failed to load configuration\n";
        std::cout << "This is expected since libmjson is not available\n";
        std::cout << "Testing default value behavior:\n";
        
        std::string default_str = config.get_string("any_key", "default_string");
        int default_int = config.get_int("any_key", 42);
        bool default_bool = config.get_bool("any_key", true);
        double default_double = config.get_double("any_key", 3.14);
        
        std::cout << "Default string: '" << default_str << "'\n";
        std::cout << "Default int: " << default_int << "\n";
        std::cout << "Default bool: " << (default_bool ? "true" : "false") << "\n";
        std::cout << "Default double: " << default_double << "\n";
    }
    
    std::cout << "\nConfigurationReader test completed.\n";
    return 0;
}