#include <gtest/gtest.h>
#include <fstream>
#include <iostream>
#include "Integrations/IntegrationContainer.hpp"
#include "Integrations/HomeAssistantSwitch.hpp"

class IntegrationContainerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Code here will be called immediately after the constructor (right
        // before each test).
        std::remove("test_config.json");
        container = new IntegrationContainer();
    };

    void TearDown() override {
        // Code here will be called immediately after each test (right
        // before the destructor).
        std::remove("test_config.json");
        delete container;
    };

    IntegrationContainer* container;
};

TEST_F(IntegrationContainerTest, CanInstantiate) 
{
    EXPECT_NE(container, nullptr);
}

TEST_F(IntegrationContainerTest, GetSwitchByIdReturnsNullForUnknownId) 
{
    IntegrationSwitchBase* sw = container->GetSwitchById(999);
    EXPECT_EQ(sw, nullptr);
}

TEST_F(IntegrationContainerTest, LoadIntegrationsFromConfigLoadsHomeAssistantSwitches) 
{
    // create a test config file or mock the loading function as needed
    std::ofstream config("test_config.json");
    config << R"({
        "integrations": [
            {
                "id": 1,
                "name": "Test Switch 1",
                "type": "HomeAssistant",
                "entity_id": "switch.test_switch_1"
            },
            {
                "id": 2,
                "name": "Test Switch 2",
                "type": "HomeAssistant",
                "entity_id": "switch.test_switch_2"
            }
        ]
    })";
    config.close();

    container->LoadIntegrationsFromConfig("test_config.json");
    IntegrationSwitchBase* sw = container->GetSwitchById(1);
    ASSERT_NE(sw, nullptr);
    EXPECT_EQ(sw->GetId(), 1);
    EXPECT_EQ(sw->GetName(), "Test Switch 1");
    HomeAssistantSwitch *haSw = dynamic_cast<HomeAssistantSwitch*>(sw);
    EXPECT_NE(haSw, nullptr);
    EXPECT_EQ(haSw->GetEntityId(), "switch.test_switch_1");
}