#pragma once
#include <sys/time.h>

class IDateTimeProvider {
    public:
    virtual int gettimeofday (struct timeval &timeval) = 0;    
};