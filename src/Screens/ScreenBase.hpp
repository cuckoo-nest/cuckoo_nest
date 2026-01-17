#pragma once
#include <string>
#include <json11.hpp>
#include "../HAL/InputDevices.hxx"
#include <iostream>

class ScreenManager;

enum screen_color
{
    SCREEN_COLOR_BLACK = 0x000000,
    SCREEN_COLOR_WHITE = 0xFFFFFF,
    SCREEN_COLOR_RED   = 0xFF0000,
    SCREEN_COLOR_GREEN = 0x00FF00,
    SCREEN_COLOR_BLUE  = 0x0000FF
};

class ScreenBase
{
public:
    ScreenBase() {}; // for testing harness
    ScreenBase(ScreenManager* screenManager, const json11::Json &jsonConfig)
    : screenManager_(screenManager)
    {
        std::vector<std::string> attribNames =
            { "name", "id", "nextScreen","integrationId" };
        // copy attribute value from json, and ensure there is entry in the attribs_
        for(const auto & attribName :attribNames)
        {
            std::string v;
            // this might be a number, convert it to string
            if(jsonConfig[attribName].is_number())
                v = std::to_string(jsonConfig[attribName].int_value());
            else if(jsonConfig[attribName].is_string())
                v = jsonConfig[attribName].string_value();
            attribs_[attribName] = v;
        }

        // if no json "id" attribute is provided, use our "name" attribute
        if(GetId() == "")
            SetId(GetName());
    }
    virtual ~ScreenBase() { OnChangeFocus(false); };

    virtual void Render() {};
    virtual void handle_input_event(const InputDeviceType device_type, const struct input_event& event) = 0;
    virtual void OnChangeFocus(bool focused) { if(focused) Render(); };

    // These getters are safe, because the ctor ensures that there is an entry in attribs_
    inline const std::string &GetId() const { return attribs_["id"]; }
    inline void SetId(std::string const &id) { attribs_["id"] = id; }

	inline void SetName(std::string const &str) { attribs_["name"] = str; }
	inline const std::string &GetName() const { return attribs_["name"]; }

    inline const std::string &GetIntegrationId() const { return attribs_["integrationId"]; }
    inline void SetIntegrationId(std::string const &id) { attribs_["integrationId"] = id; }

    inline const std::string &GetNextScreenId() const { return attribs_["nextScreen"]; }
    void SetNextScreenId(std::string const &id) { attribs_["nextScreen"] = id; }

protected:
    ScreenManager* screenManager_ = nullptr;
private:
    mutable std::map<std::string, std::string> attribs_;
};