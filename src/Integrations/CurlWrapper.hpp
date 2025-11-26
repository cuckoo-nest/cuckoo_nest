#ifndef CURL_WRAPPER_HPP
#define CURL_WRAPPER_HPP

#include <dlfcn.h>
#include <spdlog/spdlog.h>

// Forward declarations to avoid including curl headers
typedef void CURL;
typedef int CURLcode;
typedef int CURLoption;
struct curl_slist;

// Define curl constants we need
#define CURLE_OK 0
#define CURLE_FAILED_INIT 2
#define CURLOPT_URL 10002
#define CURLOPT_HTTPHEADER 10023
#define CURLOPT_POSTFIELDS 10015

class CurlWrapper {
private:
    void* libcurl_handle = nullptr;
    
    // Function pointers for libcurl functions
    CURL* (*curl_easy_init_ptr)() = nullptr;
    void (*curl_easy_cleanup_ptr)(CURL*) = nullptr;
    CURLcode (*curl_easy_setopt_ptr)(CURL*, CURLoption, ...) = nullptr;
    CURLcode (*curl_easy_perform_ptr)(CURL*) = nullptr;
    const char* (*curl_easy_strerror_ptr)(CURLcode) = nullptr;
    struct curl_slist* (*curl_slist_append_ptr)(struct curl_slist*, const char*) = nullptr;
    void (*curl_slist_free_all_ptr)(struct curl_slist*) = nullptr;

public:
    bool initialize() {
        // Try to load libcurl dynamically
        libcurl_handle = dlopen("libcurl.so.4", RTLD_LAZY);
        if (!libcurl_handle) {
            libcurl_handle = dlopen("libcurl.so", RTLD_LAZY);
        }
        
        if (!libcurl_handle) {
            spdlog::error("Could not load libcurl: {}", dlerror());
            return false;
        }

        // Load function pointers
        curl_easy_init_ptr = (CURL*(*)())dlsym(libcurl_handle, "curl_easy_init");
        curl_easy_cleanup_ptr = (void(*)(CURL*))dlsym(libcurl_handle, "curl_easy_cleanup");
        curl_easy_setopt_ptr = (CURLcode(*)(CURL*, CURLoption, ...))dlsym(libcurl_handle, "curl_easy_setopt");
        curl_easy_perform_ptr = (CURLcode(*)(CURL*))dlsym(libcurl_handle, "curl_easy_perform");
        curl_easy_strerror_ptr = (const char*(*)(CURLcode))dlsym(libcurl_handle, "curl_easy_strerror");
        curl_slist_append_ptr = (struct curl_slist*(*)(struct curl_slist*, const char*))dlsym(libcurl_handle, "curl_slist_append");
        curl_slist_free_all_ptr = (void(*)(struct curl_slist*))dlsym(libcurl_handle, "curl_slist_free_all");

        if (!curl_easy_init_ptr || !curl_easy_cleanup_ptr || !curl_easy_setopt_ptr || 
            !curl_easy_perform_ptr || !curl_easy_strerror_ptr || !curl_slist_append_ptr || 
            !curl_slist_free_all_ptr) {
            spdlog::error("Could not load curl functions: {}", dlerror());
            return false;
        }

        return true;
    }

    ~CurlWrapper() {
        if (libcurl_handle) {
            dlclose(libcurl_handle);
        }
    }

    // Wrapper functions
    CURL* easy_init() {
        return curl_easy_init_ptr ? curl_easy_init_ptr() : nullptr;
    }

    void easy_cleanup(CURL* curl) {
        if (curl_easy_cleanup_ptr) curl_easy_cleanup_ptr(curl);
    }

    CURLcode easy_setopt(CURL* curl, CURLoption option, const char* parameter) {
        return curl_easy_setopt_ptr ? curl_easy_setopt_ptr(curl, option, parameter) : CURLE_FAILED_INIT;
    }

    CURLcode easy_setopt(CURL* curl, CURLoption option, struct curl_slist* parameter) {
        return curl_easy_setopt_ptr ? curl_easy_setopt_ptr(curl, option, parameter) : CURLE_FAILED_INIT;
    }

    CURLcode easy_perform(CURL* curl) {
        return curl_easy_perform_ptr ? curl_easy_perform_ptr(curl) : CURLE_FAILED_INIT;
    }

    const char* easy_strerror(CURLcode code) {
        return curl_easy_strerror_ptr ? curl_easy_strerror_ptr(code) : "Unknown error";
    }

    struct curl_slist* slist_append(struct curl_slist* list, const char* string) {
        return curl_slist_append_ptr ? curl_slist_append_ptr(list, string) : nullptr;
    }

    void slist_free_all(struct curl_slist* list) {
        if (curl_slist_free_all_ptr) curl_slist_free_all_ptr(list);
    }

    bool isLoaded() const {
        return libcurl_handle != nullptr;
    }
};

#endif // CURL_WRAPPER_HPP