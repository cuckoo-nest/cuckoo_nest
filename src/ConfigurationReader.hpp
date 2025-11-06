#ifndef CONFIGURATION_READER_HPP
#define CONFIGURATION_READER_HPP

#include <string>
#include <map>
#include <vector>

// Forward declaration to avoid including json11.hpp in header
namespace json11 {
    class Json;
}

/**
 * @brief A configuration file reader that parses JSON files using libmjson
 * 
 * This class reads JSON configuration files from the same directory as the executable
 * and provides methods to access configuration values by key.
 */
class ConfigurationReader
{
public:
    /**
     * @brief Construct a new Configuration Reader object
     * 
     * @param config_filename The name of the configuration file (without path)
     */
    explicit ConfigurationReader(const std::string& config_filename);
    
    /**
     * @brief Destroy the Configuration Reader object
     */
    ~ConfigurationReader();

    /**
     * @brief Load and parse the configuration file
     * 
     * @return true if the file was successfully loaded and parsed
     * @return false if there was an error loading or parsing the file
     */
    bool load();

    /**
     * @brief Check if the configuration was successfully loaded
     * 
     * @return true if configuration is loaded and valid
     * @return false if configuration is not loaded or invalid
     */
    bool is_loaded() const;

    /**
     * @brief Get a string value from the configuration
     * 
     * @param key The key to look up
     * @param default_value The default value to return if key is not found
     * @return std::string The configuration value or default_value
     */
    std::string get_string(const std::string& key, const std::string& default_value = "") const;

    /**
     * @brief Get an integer value from the configuration
     * 
     * @param key The key to look up
     * @param default_value The default value to return if key is not found
     * @return int The configuration value or default_value
     */
    int get_int(const std::string& key, int default_value = 0) const;

    /**
     * @brief Get a boolean value from the configuration
     * 
     * @param key The key to look up
     * @param default_value The default value to return if key is not found
     * @return bool The configuration value or default_value
     */
    bool get_bool(const std::string& key, bool default_value = false) const;

    /**
     * @brief Get a floating point value from the configuration
     * 
     * @param key The key to look up
     * @param default_value The default value to return if key is not found
     * @return double The configuration value or default_value
     */
    double get_double(const std::string& key, double default_value = 0.0) const;

    /**
     * @brief Check if a key exists in the configuration
     * 
     * @param key The key to check
     * @return true if the key exists
     * @return false if the key does not exist
     */
    bool has_key(const std::string& key) const;

    /**
     * @brief Get all keys in the configuration
     * 
     * @return std::vector<std::string> A vector of all configuration keys
     */
    std::vector<std::string> get_keys() const;

    /**
     * @brief Get the Home Assistant base URL from the homeAssistant section
     * 
     * @param default_value The default value to return if not found
     * @return std::string The base URL or default_value
     */
    std::string get_home_assistant_base_url(const std::string& default_value = "") const;

    /**
     * @brief Get the Home Assistant token from the homeAssistant section
     * 
     * @param default_value The default value to return if not found
     * @return std::string The token or default_value
     */
    std::string get_home_assistant_token(const std::string& default_value = "") const;

    /**
     * @brief Get the Home Assistant entity ID from the homeAssistant section
     * 
     * @param default_value The default value to return if not found
     * @return std::string The entity ID or default_value
     */
    std::string get_home_assistant_entity_id(const std::string& default_value = "") const;

    /**
     * @brief Check if the homeAssistant section exists and has all required fields
     * 
     * @return bool True if baseURL, token, and entityId are present
     */
    bool has_home_assistant_config() const;

private:
    std::string config_filename_;
    std::string config_filepath_;
    json11::Json* json_root_;  // json11 object pointer
    bool loaded_;

    /**
     * @brief Get the directory where the executable is located
     * 
     * @return std::string The executable directory path
     */
    std::string get_executable_directory() const;

    /**
     * @brief Read the entire file content into a string
     * 
     * @param filepath The path to the file to read
     * @return std::string The file content, empty string on error
     */
    std::string read_file_content(const std::string& filepath) const;
};

#endif // CONFIGURATION_READER_HPP