#include <gtest/gtest.h>
#include <memory>
#include "ScreenManager.hpp"
#include "Screens/ScreenBase.hpp"

// Mock screen class for testing
class MockScreen : public ScreenBase {
public:
    MockScreen() {}
    virtual ~MockScreen() {}
    
    void Render() {
        renderCallCount++;
    }

    int GetRenderCallCount() const {
        return renderCallCount;
    }
    
    void handle_input_event(const InputDeviceType device_type, const struct input_event& event) {
        // Mock implementation - do nothing for tests
    }

private:
    int renderCallCount = 0;
};

// Test fixture for ScreenManager tests
class ScreenManagerTest : public ::testing::Test {
protected:
    void SetUp() {
        screenManager = new ScreenManager(nullptr, nullptr);
        
        mockScreen1 = new MockScreen();
        mockScreen1->SetId(1);
        screenManager->AddScreen(std::unique_ptr<ScreenBase>(mockScreen1));
        
        mockScreen2 = new MockScreen();
        mockScreen2->SetId(2);
        screenManager->AddScreen(std::unique_ptr<ScreenBase>(mockScreen2));
    }
    
    void TearDown() {
        delete screenManager;
        screenManager = nullptr;
    }
    
    ScreenManager* screenManager;
    MockScreen* mockScreen1;
    MockScreen* mockScreen2;
};

TEST_F(ScreenManagerTest, CanInstantiate) 
{
    EXPECT_NE(screenManager, nullptr);
}

TEST_F(ScreenManagerTest, FirstScreenRenderCalled) 
{
    screenManager->GoToNextScreen(mockScreen1->GetId());
    EXPECT_EQ(mockScreen1->GetRenderCallCount(), 1);
}

TEST_F(ScreenManagerTest, GoToNextScreen) 
{   
    screenManager->GoToNextScreen(mockScreen1->GetId());
    screenManager->GoToNextScreen(mockScreen2->GetId());
    EXPECT_EQ(mockScreen2->GetRenderCallCount(),1);
}

TEST_F(ScreenManagerTest, GoToPreviousScreen) 
{
    screenManager->GoToNextScreen(mockScreen1->GetId());
    screenManager->GoToNextScreen(mockScreen2->GetId());
    screenManager->GoToPreviousScreen();
    
    EXPECT_EQ(mockScreen1->GetRenderCallCount(), 2);
}

TEST_F(ScreenManagerTest, MultipleScreenTransitions) 
{
    // Test a sequence of screen transitions
    screenManager->GoToNextScreen(mockScreen1->GetId());
    screenManager->GoToNextScreen(mockScreen2->GetId());
    screenManager->GoToPreviousScreen();
    screenManager->GoToNextScreen(mockScreen1->GetId());

    EXPECT_EQ(mockScreen1->GetRenderCallCount(), 3);
}

TEST_F(ScreenManagerTest, Destructor) 
{
    // Set up some screens
    screenManager->GoToNextScreen(mockScreen1->GetId());
    screenManager->GoToNextScreen(mockScreen2->GetId());
    
    // Create a new ScreenManager and delete it to test destructor
    ScreenManager* test_manager = new ScreenManager(nullptr, nullptr);
    delete test_manager;
    
    SUCCEED();
}

TEST_F(ScreenManagerTest, ThreeLevelScreenHistory) 
{
    MockScreen* mock_screen3 = new MockScreen();
    mock_screen3->SetId(3);
    screenManager->AddScreen(std::unique_ptr<ScreenBase>(mock_screen3));

    screenManager->GoToNextScreen(mockScreen1->GetId());
    screenManager->GoToNextScreen(mockScreen2->GetId());
    screenManager->GoToNextScreen(mock_screen3->GetId());

    // go back to screen 1
    screenManager->GoToPreviousScreen(); // should go to screen2
    screenManager->GoToPreviousScreen(); // should go to screen1
    
    EXPECT_EQ(mockScreen1->GetRenderCallCount(), 2);
}