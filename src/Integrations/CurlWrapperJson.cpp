
#include <string>
#include <algorithm> 
#include <functional> 

#include "CurlWrapperJson.hpp"
#include "logger.h"

#include <json11.hpp>


static std::size_t callbackString( const char* in, std::size_t size, std::size_t num, std::string* out)
{
    const std::size_t totalBytes(size * num);
    out->append(in, totalBytes);
    return totalBytes;
};

static std::string &rtrim(std::string &s)
{
    s.erase(
        std::find_if( s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace)) ).base()
        , s.end()
        );
    return s;
}

static std::vector<std::string> splitLlines(const std::string& input)
{
    std::vector<std::string> lines;
    std::stringstream ss(input);
    std::string line;
    while (std::getline(ss, line))
        lines.push_back(rtrim(line));
    return lines;
}


bool CurlWrapperJson::Startup()
{
    if(!curlInitialized_)
        curlInitialized_ = curlWrapper_.initialize();
    if(curlInitialized_)
    {
        if(curl_ == nullptr)
            curl_ = curlWrapper_.easy_init();
        if(curl_ == nullptr)
            LOG_ERROR_STREAM("Failed to initialize curlWrapper_");
    }
    else
        LOG_ERROR_STREAM("Failed to initialize libcurl");
    return curlInitialized_ && curl_ != nullptr;
}

void CurlWrapperJson::Shutdown()
{
    if(curl_)
    {
        curlWrapper_.easy_cleanup(curl_);
        curl_ = nullptr;
    }
}

json11::Json CurlWrapperJson::jsonGetOrPost(std::string url, std::string const &postData)
{
    json11::Json js;

    Startup();

    if(curl_)
    {
        std::string responseBody;
        std::string responseHeaders;
        struct curl_slist *headers = nullptr;

        if(headerAuthBearer_ != "")
            headers = curlWrapper_.slist_append(headers, headerAuthBearer_.c_str());
        headers = curlWrapper_.slist_append(headers, "Content-Type: application/json");

        LOG_INFO_STREAM("CurlWrapperJson: URL: " << url);
        curlWrapper_.easy_setopt(curl_, CURLOPT_URL, url.c_str());
        curlWrapper_.easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);
        curlWrapper_.easy_setopt(curl_, CURLOPT_WRITEFUNCTION, callbackString);
        curlWrapper_.easy_setopt(curl_, CURLOPT_WRITEDATA, &responseBody);
        curlWrapper_.easy_setopt(curl_, CURLOPT_HEADERFUNCTION, callbackString);
        curlWrapper_.easy_setopt(curl_, CURLOPT_HEADERDATA, &responseHeaders);
        if(postData != "")
        {
            // LOG_INFO_STREAM("CurlWrapperJson request post: " << postData);
            curlWrapper_.easy_setopt(curl_, CURLOPT_POSTFIELDS, postData.c_str());
        }

        CURLcode res = curlWrapper_.easy_perform(curl_);

        if(res == CURLE_OK)
        {
            // LOG_INFO_STREAM("CurlWrapperJson request completed headers = "<< responseHeaders);
            std::vector<std::string> headers = splitLlines(responseHeaders);
            bool responseIsJsonContent = false;
            for (auto& header : headers)
            {
                bool isJson = (header == "Content-Type: application/json");
                // LOG_INFO_STREAM("CurlWrapperJson request responseHeader " << header << " is Json " << isJson);
                responseIsJsonContent |= isJson;
            }
            // LOG_INFO_STREAM("CurlWrapperJson request completed body = "<< responseBody);
            if(responseIsJsonContent)
            {
                // LOG_INFO_STREAM("CurlWrapperJson request completed body = "<< responseBody);
                std::string parse_error;
                js = json11::Json::parse(responseBody, parse_error);
                if (!parse_error.empty())
                    LOG_ERROR_STREAM("CurlWrapperJson request JSON parse error: " << parse_error);
            }
            else
                LOG_INFO_STREAM("CurlWrapperJson request completed body is not JSON = " << responseBody);

        }
        else
            LOG_ERROR_STREAM("CurlWrapperJson request failed: " << curlWrapper_.easy_strerror(res));

        if(headers)
            curlWrapper_.slist_free_all(headers);
    }

    Shutdown();

    return js;
}
