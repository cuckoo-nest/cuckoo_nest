#pragma once

class ScreenBase
{
public:
    virtual ~ScreenBase() = default;

    virtual void Render() = 0;
};