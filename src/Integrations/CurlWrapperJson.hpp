#pragma once

#include <string>

#include "CurlWrapper.hpp"
#include <json11.hpp>

class CurlWrapperJson
{
public:
    CurlWrapperJson() { Startup(); };
    virtual ~CurlWrapperJson() { Shutdown(); };

    bool Startup();
    void Shutdown();

    inline CurlWrapperJson *Bearer(std::string token)
    {
        headerAuthBearer_ = (token != "" ? "Authorization: Bearer " + token : "");
        return this;
    }

    json11::Json jsonGetOrPost(std::string url, std::string const &postData = "");

protected:
    CurlWrapper curlWrapper_;
    bool curlInitialized_ = false;
    CURL *curl_ = nullptr;
    std::string headerAuthBearer_;
};
