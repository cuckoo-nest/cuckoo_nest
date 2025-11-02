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

static HomeScreen home_screen (&screen, &beeper);

int main() 
{
   printf("Cuckoo Hello\n");

   if (!screen.initialize()) {
       fprintf(stderr, "Failed to initialize screen\n");
       return 1;
   }

   // Set up input event callback
   inputs.set_callback(handle_input_event);
   
   if (!inputs.start_polling()) {
       fprintf(stderr, "Failed to start input polling\n");
       return 1;
   }
   
   printf("Input polling started in background thread...\n");

   home_screen.Render();
   
   // Main thread can now do other work or just wait
   while (1) {
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

   home_screen.handle_input_event(device_type, event);
   home_screen.Render();
}
