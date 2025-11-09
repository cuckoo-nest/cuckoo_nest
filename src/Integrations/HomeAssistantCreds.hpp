#pragma once

class HomeAssistantCreds 
{
public:
    HomeAssistantCreds() = default;
    HomeAssistantCreds(std::string url, std::string token) : 
        url_(url), 
        token_(token) 
        {}

    const std::string& GetUrl() const { return url_; }
    const std::string& GetToken() const { return token_; }

private:
    std::string url_;
    std::string token_;
};