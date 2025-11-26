#include "ConfigurationReader.hpp"
#include <spdlog/spdlog.h>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <limits.h>
#include "json11.hpp"

ConfigurationReader::ConfigurationReader(const std::string& config_filename)
    : config_filename_(config_filename)
    , json_root_(nullptr)
    , loaded_(false)
{
    config_filepath_ = config_filename_;
}

ConfigurationReader::~ConfigurationReader()
{
    if (json_root_ != nullptr) {
        delete json_root_;
        json_root_ = nullptr;
    }
}

bool ConfigurationReader::load()
{
    loaded_ = false;
    
    // Clean up any existing json object
    if (json_root_ != nullptr) {
        delete json_root_;
        json_root_ = nullptr;
    }
    
    // Read file content
    std::string content = read_file_content(config_filepath_);
    if (content.empty()) {
        spdlog::error("ConfigurationReader: Failed to read file: {}", config_filepath_);
        return false;
    }

    // Parse JSON using json11
    std::string parse_error;
    json11::Json parsed_json = json11::Json::parse(content, parse_error);
    
    if (!parse_error.empty()) {
        spdlog::error("ConfigurationReader: JSON parse error: {}", parse_error);
        return false;
    }
    
    if (!parsed_json.is_object()) {
        spdlog::error("ConfigurationReader: Root JSON element must be an object");
        return false;
    }
    
    // Store the parsed JSON
    json_root_ = new json11::Json(parsed_json);
    loaded_ = true;
    
    spdlog::info("ConfigurationReader: Successfully loaded config from {}", config_filepath_);
    return true;
}

bool ConfigurationReader::is_loaded() const
{
    return loaded_;
}

std::string ConfigurationReader::get_string(const std::string& key, const std::string& default_value) const
{
    if (!loaded_ || json_root_ == nullptr) {
        return default_value;
    }

    const json11::Json& value = (*json_root_)[key];
    if (value.is_string()) {
        return value.string_value();
    }

    return default_value;
}

int ConfigurationReader::get_int(const std::string& key, int default_value) const
{
    if (!loaded_ || json_root_ == nullptr) {
        return default_value;
    }

    const json11::Json& value = (*json_root_)[key];
    if (value.is_number()) {
        return value.int_value();
    }

    return default_value;
}

bool ConfigurationReader::get_bool(const std::string& key, bool default_value) const
{
    if (!loaded_ || json_root_ == nullptr) {
        return default_value;
    }

    const json11::Json& value = (*json_root_)[key];
    if (value.is_bool()) {
        return value.bool_value();
    }

    return default_value;
}

double ConfigurationReader::get_double(const std::string& key, double default_value) const
{
    if (!loaded_ || json_root_ == nullptr) {
        return default_value;
    }

    const json11::Json& value = (*json_root_)[key];
    if (value.is_number()) {
        return value.number_value();
    }

    return default_value;
}

bool ConfigurationReader::has_key(const std::string& key) const
{
    if (!loaded_ || json_root_ == nullptr) {
        return false;
    }

    const json11::Json& value = (*json_root_)[key];
    return !value.is_null();
}

std::vector<std::string> ConfigurationReader::get_keys() const
{
    std::vector<std::string> keys;
    
    if (!loaded_ || json_root_ == nullptr) {
        return keys;
    }

    if (json_root_->is_object()) {
        const auto& object_items = json_root_->object_items();
        for (const auto& pair : object_items) {
            keys.push_back(pair.first);
        }
    }

    return keys;
}

std::string ConfigurationReader::get_executable_directory() const
{
    char path[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
    
    if (count == -1) {
        spdlog::error("ConfigurationReader: Failed to get executable path");
        return ".";  // fallback to current directory
    }
    
    std::string exe_path(path, count);
    size_t last_slash = exe_path.find_last_of('/');
    
    if (last_slash != std::string::npos) {
        return exe_path.substr(0, last_slash);
    }
    
    return ".";  // fallback to current directory
}

std::string ConfigurationReader::get_home_assistant_base_url(const std::string& default_value) const
{
    if (!loaded_ || json_root_ == nullptr) {
        return default_value;
    }

    const json11::Json& ha_control = (*json_root_)["homeAssistant"];
    if (ha_control.is_object()) {
        const json11::Json& base_url = ha_control["baseURL"];
        if (base_url.is_string()) {
            return base_url.string_value();
        }
    }

    return default_value;
}

std::string ConfigurationReader::get_home_assistant_token(const std::string& default_value) const
{
    if (!loaded_ || json_root_ == nullptr) {
        return default_value;
    }

    const json11::Json& ha_control = (*json_root_)["homeAssistant"];
    if (ha_control.is_object()) {
        const json11::Json& token = ha_control["token"];
        if (token.is_string()) {
            return token.string_value();
        }
    }

    return default_value;
}

std::string ConfigurationReader::get_home_assistant_entity_id(const std::string& default_value) const
{
    if (!loaded_ || json_root_ == nullptr) {
        return default_value;
    }

    const json11::Json& ha_control = (*json_root_)["homeAssistant"];
    if (ha_control.is_object()) {
        const json11::Json& entity_id = ha_control["entityId"];
        if (entity_id.is_string()) {
            return entity_id.string_value();
        }
    }

    return default_value;
}

bool ConfigurationReader::has_home_assistant_config() const
{
    if (!loaded_ || json_root_ == nullptr) {
        return false;
    }

    const json11::Json& ha_control = (*json_root_)["homeAssistant"];
    if (ha_control.is_object()) {
        const json11::Json& base_url = ha_control["baseURL"];
        const json11::Json& token = ha_control["token"];
        const json11::Json& entity_id = ha_control["entityId"];
        
        return base_url.is_string() && !base_url.string_value().empty() &&
               token.is_string() && !token.string_value().empty() &&
               entity_id.is_string() && !entity_id.string_value().empty();
    }

    return false;
}

std::string ConfigurationReader::read_file_content(const std::string& filepath) const
{
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}