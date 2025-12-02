#pragma once
#include "IDateTimeProvider.hpp"

class SystemDateTimeProvider : public IDateTimeProvider {
public:
    int gettimeofday(struct timeval &tv) override {
        return ::gettimeofday(&tv, nullptr);
    }
};