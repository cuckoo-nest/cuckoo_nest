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
        render_call_count_++;
    }

    int GetRenderCallCount() const {
        return render_call_count_;
    }
    
    void handle_input_event(const InputDeviceType device_type, const struct input_event& event) {
        // Mock implementation - do nothing for tests
    }

private:
    int render_call_count_ = 0;
};

// Test fixture for ScreenManager tests
class ScreenManagerTest : public ::testing::Test {
protected:
    void SetUp() {
        screen_manager = new ScreenManager();
        mock_screen1 = new MockScreen();
        mock_screen2 = new MockScreen();
    }
    
    void TearDown() {
        // dont clean these up - they are deleted by ScreenManager
        // delete screen_manager;
        // delete mock_screen1;
        // delete mock_screen2;
        // screen_manager = nullptr;
        // mock_screen1 = nullptr;
        // mock_screen2 = nullptr;
    }
    
    ScreenManager* screen_manager;
    MockScreen* mock_screen1;
    MockScreen* mock_screen2;
};

TEST_F(ScreenManagerTest, CanInstantiate) 
{
    EXPECT_NE(screen_manager, nullptr);
}

TEST_F(ScreenManagerTest, FirstScreenRenderCalled) 
{
    screen_manager->GoToNextScreen(mock_screen1);
    EXPECT_EQ(mock_screen1->GetRenderCallCount(), 1);
}

TEST_F(ScreenManagerTest, GoToNextScreen) 
{   
    screen_manager->GoToNextScreen(mock_screen1);
    screen_manager->GoToNextScreen(mock_screen2);
    EXPECT_EQ(mock_screen2->GetRenderCallCount(),1);
}

TEST_F(ScreenManagerTest, GoToPreviousScreen) 
{
    screen_manager->GoToNextScreen(mock_screen1);
    screen_manager->GoToNextScreen(mock_screen2);
    screen_manager->GoToPreviousScreen();
    
    EXPECT_EQ(mock_screen1->GetRenderCallCount(), 2);
}

TEST_F(ScreenManagerTest, MultipleScreenTransitions) 
{
    // Test a sequence of screen transitions
    screen_manager->GoToNextScreen(mock_screen1);
    screen_manager->GoToNextScreen(mock_screen2);
    screen_manager->GoToPreviousScreen();
    screen_manager->GoToNextScreen(mock_screen1);

    EXPECT_EQ(mock_screen1->GetRenderCallCount(), 3);
}

TEST_F(ScreenManagerTest, Destructor) 
{
    // Set up some screens
    screen_manager->GoToNextScreen(mock_screen1);
    screen_manager->GoToNextScreen(mock_screen2);
    
    // Create a new ScreenManager and delete it to test destructor
    ScreenManager* test_manager = new ScreenManager();
    delete test_manager;
    
    SUCCEED();
}

// Test screen history of three levels
TEST_F(ScreenManagerTest, ThreeLevelScreenHistory) 
{
    MockScreen* mock_screen3 = new MockScreen();
    screen_manager->GoToNextScreen(mock_screen1);
    screen_manager->GoToNextScreen(mock_screen2);
    screen_manager->GoToNextScreen(mock_screen3);

    // go back to screen 1
    screen_manager->GoToPreviousScreen(); // should go to screen2
    screen_manager->GoToPreviousScreen(); // should go to screen1
    
    EXPECT_EQ(mock_screen1->GetRenderCallCount(), 2);
}