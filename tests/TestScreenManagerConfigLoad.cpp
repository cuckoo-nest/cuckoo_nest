#include <gtest/gtest.h>
#include <fstream>
#include <iostream>

#include "ScreenManager.hpp"

class ScreenManagerConfigLoadTest : public ::testing::Test {
protected:

    void SetUp() override {
        screen_manager = new ScreenManager(&hal);
    }

    void TearDown() override {
        delete screen_manager;
        screen_manager = nullptr;
    }

    ScreenManager* screen_manager;
    HAL hal;
};

TEST_F(ScreenManagerConfigLoadTest, InitialState) {
    EXPECT_EQ(0, screen_manager->CountScreens());
}

TEST_F(ScreenManagerConfigLoadTest, LoadScreensFromConfig) {
    std::ofstream config("test_config.json");
    config << R"({
        "screens": [
            {
                "id": 1,
                "name": "HomeScreen",
                "type": "Home"
            },
            {
                "id": 2,
                "name": "MainMenu",
                "type": "Menu"
            }
        ]
    })";
    config.close();

    screen_manager->LoadScreensFromConfig("test_config.json");
    EXPECT_EQ(2, screen_manager->CountScreens());
}


// screen types should be case insensitive
// if id is a string then the screen should be skipped
