#include <gtest/gtest.h>
#include <fstream>
#include <iostream>

#include "ScreenManager.hpp"
#include "Screens/HomeScreen.hpp"
#include "Screens/MenuScreen.hpp"
#include "Screens/SwitchScreen.hpp"
#include "Screens/DimmerScreen.hpp"

class ScreenManagerConfigLoadTest : public ::testing::Test {
protected:

    void SetUp() override {
        screen_manager = new ScreenManager(&hal, nullptr);
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
                "type": "Home",
                "nextScreen": 2
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

TEST_F(ScreenManagerConfigLoadTest, HomeScreenLinksToMenuScreen) {
    std::ofstream config("test_config.json");
    config << R"({
        "screens": [
            {
                "id": 1,
                "name": "HomeScreen",
                "type": "Home",
                "nextScreen": 2
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

    // Navigate to HomeScreen
    ScreenBase* home_screen = screen_manager->GetScreenById(1);
    ASSERT_NE(nullptr, home_screen);

    // Check that HomeScreen's next screen is MenuScreen
    HomeScreen* hs = dynamic_cast<HomeScreen*>(home_screen);
    ASSERT_NE(hs, nullptr);

    EXPECT_EQ(2, hs->GetNextScreenId());
}

TEST_F(ScreenManagerConfigLoadTest, ScreenTypeCaseInsensitive) {
    std::ofstream config("test_config.json");
    config << R"({
        "screens": [
            {
                "id": 1,
                "name": "HomeScreen",
                "type": "hOmE",
                "nextScreen": 2
            },
            {
                "id": 2,
                "name": "MainMenu",
                "type": "MeNu"
            }
        ]
    })";
    config.close();

    screen_manager->LoadScreensFromConfig("test_config.json");
    EXPECT_EQ(2, screen_manager->CountScreens());

    ScreenBase* home_screen = screen_manager->GetScreenById(1);
    ASSERT_NE(nullptr, home_screen);
    EXPECT_NE(dynamic_cast<HomeScreen*>(home_screen), nullptr);

    ScreenBase* menu_screen = screen_manager->GetScreenById(2);
    ASSERT_NE(nullptr, menu_screen);
    EXPECT_NE(dynamic_cast<MenuScreen*>(menu_screen), nullptr);
}

TEST_F(ScreenManagerConfigLoadTest, InvalidScreenIdSkipped) {
    std::ofstream config("test_config.json");
    config << R"({
        "screens": [
            {
                "id": "one",
                "name": "HomeScreen",
                "type": "Home",
                "nextScreen": 2
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
    EXPECT_EQ(1, screen_manager->CountScreens());

    ScreenBase* menu_screen = screen_manager->GetScreenById(2);
    ASSERT_NE(nullptr, menu_screen);
    EXPECT_NE(dynamic_cast<MenuScreen*>(menu_screen), nullptr);
}

// menu screen is populated with a list of menu items from config
TEST_F(ScreenManagerConfigLoadTest, MenuScreenItemsLoaded) {
    std::ofstream config("test_config.json");
    config << R"({
        "screens": [
            {
                "id": 1,
                "name": "MainMenu",
                "type": "Menu",
                "menuItems": [
                    {
                        "name": "Item1",
                        "nextScreen": 2
                    },
                    {
                        "name": "Item2",
                        "nextScreen": 3
                    }
                ]
            },
            {
                "id": 2,
                "name": "SubMenu1",
                "type": "Menu"
            },
            {
                "id": 3,
                "name": "SubMenu2",
                "type": "Menu"
            }
        ]
    })";
    config.close();

    screen_manager->LoadScreensFromConfig("test_config.json");
    EXPECT_EQ(3, screen_manager->CountScreens());

    ScreenBase* menu_screen = screen_manager->GetScreenById(1);
    ASSERT_NE(nullptr, menu_screen);
    MenuScreen* ms = dynamic_cast<MenuScreen*>(menu_screen);
    ASSERT_NE(ms, nullptr);
    EXPECT_EQ(2, ms->CountMenuItems());

}

TEST_F(ScreenManagerConfigLoadTest, SwitchScreenIntegrationLoaded) {
    std::ofstream config("test_config.json");
    config << R"({
        "screens": [
            {
                "id": 1,
                "name": "SwitchScreen1",
                "type": "Switch",
                "integrationId": 42
            }
        ]
    })";
    config.close();

    screen_manager->LoadScreensFromConfig("test_config.json");
    EXPECT_EQ(1, screen_manager->CountScreens());

    ScreenBase* switch_screen = screen_manager->GetScreenById(1);
    ASSERT_NE(nullptr, switch_screen);
    SwitchScreen* ss = dynamic_cast<SwitchScreen*>(switch_screen);
    ASSERT_NE(ss, nullptr);
    EXPECT_EQ(42, ss->GetIntegrationId());
}

TEST_F(ScreenManagerConfigLoadTest, DimmerScreenIntegrationLoaded) {
    std::ofstream config("test_config.json");
    config << R"({
        "screens": [
            {
                "id": 1,
                "name": "DimmerScreen1",
                "type": "Dimmer",
                "integrationId": 55
            }
        ]
    })";
    config.close();

    screen_manager->LoadScreensFromConfig("test_config.json");
    EXPECT_EQ(1, screen_manager->CountScreens());

    ScreenBase* dimmer_screen = screen_manager->GetScreenById(1);
    ASSERT_NE(nullptr, dimmer_screen);
    DimmerScreen* ds = dynamic_cast<DimmerScreen*>(dimmer_screen);
    ASSERT_NE(ds, nullptr);
    EXPECT_EQ(55, ds->GetIntegrationId());
}
