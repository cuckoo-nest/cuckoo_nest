#include "HomeScreen.hpp"
#include <string>
#include "logger.h"
#include "DimmerScreen.hpp"

static enum screen_color colors[] = {
    SCREEN_COLOR_BLACK,
    SCREEN_COLOR_RED,
    SCREEN_COLOR_GREEN,
    SCREEN_COLOR_BLUE,
    SCREEN_COLOR_WHITE,
};
// static int color_count = sizeof(colors) / sizeof(colors[0]);

HomeScreen::HomeScreen(ScreenManager *screenManager, const json11::Json &jsonConfig)
    : ScreenBase(screenManager, jsonConfig)
{
    if(GetName() == "")
        SetName("Home");
    beeper_ = screenManager_->HalBeeper();
    display_ = screenManager_->HalDisplay();
    backplateComms_ = screenManager_->GetBackplaceComms();

    temperatureUnits_ = 'c';
    if(!jsonConfig["showTempAs"].is_null())
    {
        std::string temperatureUnits = jsonConfig["showTempAs"].string_value();
        if(temperatureUnits.length() > 0)
            temperatureUnits_ = temperatureUnits[0];
    }

    timeFormat_ = (
        !jsonConfig["timeFormat"].is_null()
        ? jsonConfig["timeFormat"].int_value()
        : 24
    );
}

void HomeScreen::Render()
{
    if (display_ == nullptr)
        return;

    display_->SetBackgroundColor(colors[currentColorIndex]);
    
    // Display text using the bitmap font
    uint32_t text_color = SCREEN_COLOR_WHITE;
    if (colors[currentColorIndex] == SCREEN_COLOR_WHITE)
        text_color = SCREEN_COLOR_BLACK; // Use black text on white background

    // get current time
    time_t now = time(0);
	display_->DrawText(60, -40, GetName(), SCREEN_COLOR_WHITE, Font::FONT_H1);
    display_->DrawText(40, 0, TimeToString(now), text_color, Font::FONT_H1);

    display_->DrawText(0, 60, GetTemperatureString(), SCREEN_COLOR_RED, Font::FONT_H2);
    display_->DrawText(0, 90, GetHumidityString(), SCREEN_COLOR_BLUE, Font::FONT_H2);

}

void HomeScreen::OnChangeFocus(bool focused)
{
    if(focused)
    {
        Render();
        if (!lvObjTimerCb)
            lvObjTimerCb = lv_timer_create(timer_cb, 250, this);
    }
    else
    {
        if(lvObjTimerCb)
        {
            lv_timer_delete(lvObjTimerCb);
            lvObjTimerCb = nullptr;
        }
    }
}

void HomeScreen::timer_cb(lv_timer_t * timer)
{
    HomeScreen * screen = (HomeScreen *)lv_timer_get_user_data(timer);
    screen->Render();
}

void HomeScreen::handle_input_event(const InputDeviceType device_type, const struct input_event& event)
{
    if (device_type == InputDeviceType::BUTTON && event.type == EV_KEY && event.code == 't' && event.value == 1)
    {
        if(beeper_ != nullptr)
            beeper_->click();

        if (GetNextScreenId() != "")
            screenManager_->GoToNextScreen(GetNextScreenId());
        else
            screenManager_->GoToPreviousScreen();
    }
}

std::string HomeScreen::TimeToString(time_t time)
{
    char buffer[100];
    struct tm *timeinfo = localtime(&time);
    std::string strFmt;

    switch(timeFormat_)
    {
        default:
        case 24:
            strFmt = "%H:%M:%S";
            break;
        case 12:
            strFmt = "%-I:%02M:%02S %p";
            break;
    }

    strftime(buffer, sizeof(buffer), strFmt.c_str(), timeinfo);
    return std::string(buffer);
}

std::string HomeScreen::GetTemperatureString()
{
    if (backplateComms_ == nullptr)
        return "N/A";

    float temperature = backplateComms_->GetCurrentTemperatureC();
    char buffer[16];
    switch(temperatureUnits_)
    {
        default:
        case 'c':
            snprintf(buffer, sizeof(buffer), "%.2f C", temperature);
            break;
        case 'f':
            snprintf(buffer, sizeof(buffer), "%d F", (int)(temperature * 1.8) + 32);
            break;
        case 'k':
            snprintf(buffer, sizeof(buffer), "%.2f K", temperature + 237.15);
            break;
    }

    return std::string(buffer);
}

std::string HomeScreen::GetHumidityString()
{
    if (backplateComms_ == nullptr)
        return "N/A";
    
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%.2f %%", backplateComms_->GetCurrentHumidityPercent());
    return std::string(buffer);
}
