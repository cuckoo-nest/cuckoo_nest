#ifndef _INPUTEVENT_HPP_
#define _INPUTEVENT_HPP_

#ifdef BUILD_TARGET_LINUX
#include <linux/input.h>
#else
#define EV_KEY 1
#define EV_SND 2
#define SND_BELL 1
extern "C" struct input_event
{
	int type;
	int code;
	int value;
};
#endif
#endif