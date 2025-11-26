#include "IntegrationActionBase.hpp"
#include "CurlWrapper.hpp"
#include <string>
#include <spdlog/spdlog.h>

class ActionHomeAssistantService : public IntegrationActionBase 
{
public:
    ActionHomeAssistantService(
        const std::string& authKey,
        const std::string& domain,
        const std::string& baseUrl,
        const std::string& serviceUrl,
        const std::string& entityId
    ) : 
        authKey_(authKey),
        domain_(domain),
        baseUrl_(baseUrl),
        serviceUrl_(serviceUrl),
        entityId_(entityId)
    {}

    virtual ~ActionHomeAssistantService() = default;

    
    void execute() override 
    {
        // Implementation to call Home Assistant service
        spdlog::info("Calling Home Assistant service: {}", domain_);

        static CurlWrapper curl_wrapper;
        static bool curl_initialized = false;
        
        if (!curl_initialized) {
            curl_initialized = curl_wrapper.initialize();
            if (!curl_initialized) {
                spdlog::error("Failed to initialize libcurl - functionality disabled");
                return;
            }
        }

        // Prepare JSON data
        std::string jsonData = "{\"entity_id\": \"" + entityId_ + "\"}";
        
        // Prepare headers
        struct curl_slist* headers = nullptr;
        std::string authHeader = "Authorization: Bearer " + authKey_;
        headers = curl_wrapper.slist_append(headers, authHeader.c_str());
        headers = curl_wrapper.slist_append(headers, "Content-Type: application/json");

        CURL* curl = curl_wrapper.easy_init();
        if (curl) {
            std::string url = baseUrl_ + "/api/services/" + serviceUrl_;
            curl_wrapper.easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_wrapper.easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_wrapper.easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());

            CURLcode res = curl_wrapper.easy_perform(curl);
            if (res != CURLE_OK) {
                spdlog::error("curl_easy_perform() failed: {}", curl_wrapper.easy_strerror(res));
            }

            curl_wrapper.easy_cleanup(curl);
            curl_wrapper.slist_free_all(headers);
        }
    }

private:
    std::string authKey_;
    std::string domain_;
    std::string baseUrl_;
    std::string serviceUrl_;
    std::string entityId_;
    
};