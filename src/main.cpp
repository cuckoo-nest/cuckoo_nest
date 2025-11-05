#include <stdio.h>
#include <unistd.h>
#include "linux/input.h"

#include "HAL/Beeper.hpp"
#include "HAL/Display.hpp"
#include "HAL/Inputs.hpp"

#include "Screens/HomeScreen.hpp"

// Function declarations
void change_screen_color();
void handle_input_event(const InputDeviceType device_type, const struct input_event& event);

static Beeper beeper("/dev/input/event0");
static Display screen("/dev/fb0");
static Inputs inputs("/dev/input/event2", "/dev/input/event1");
static ScreenManager screen_manager;

static HomeScreen home_screen (
   &screen_manager,
   &screen, 
   &beeper);

int main() 
{
   printf("Cuckoo Hello\n");

   if (!screen.initialize()) {
       fprintf(stderr, "Failed to initialize screen\n");
       return 1;
   }

   screen_manager.GoToNextScreen(new HomeScreen(
       &screen_manager,
       &screen,
       &beeper));

   // Set up input event callback
   inputs.set_callback(handle_input_event);
   
   if (!inputs.start_polling()) {
       fprintf(stderr, "Failed to start input polling\n");
       return 1;
   }
   
   printf("Input polling started in background thread...\n");

   
   // Main thread can now do other work or just wait
   while (1) {
      screen_manager.RenderCurrentScreen();
      sleep(1); // Sleep for 1 second - background thread handles input polling
   }

   return 0;
}

// Input event handler callback
void handle_input_event(const InputDeviceType device_type, const struct input_event &event)
{
   printf("%d: time_sec=%lu, time_usec=%lu, type=%hu, code=%hu, value=%d | \n",
          static_cast<int>(device_type),
          event.time.tv_sec,
          event.time.tv_usec,
          event.type,
          event.code,
          event.value);

   if (device_type == InputDeviceType::ROTARY
   && event.type == 0 && event.code == 0) {
       return; // Ignore "end of event" markers from rotary encoder
   }

   screen_manager.ProcessInputEvent(device_type, event);
   screen_manager.RenderCurrentScreen();
}
