#pragma once
#include "utils.hpp"
#include <curl/curl.h>

class Image {
    using Ref = std::shared_ptr<Image>;

public:
    Image();
    Image(const Image&) = delete;

    virtual ~Image();

    /// @brief Sets the image component to be loaded with content. This function needs to work in the main thread.
    static void fromUrl(brls::Image* view, const std::string& url, std::string path = "");

    /// @brief Cancel the request and clear the image. This function needs to work in the main thread.
    static void cancel(brls::Image* view);

private:
    void doRequest();
    static void clear(brls::Image* view);

private:
    std::string url;
    std::string path;
    brls::Image* image;
    // HTTP::Cancel isCancel;

    /// object pool
    inline static std::list<Ref> pool;
    inline static std::mutex requestMutex;
    inline static std::unordered_map<brls::Image*, Ref> requests;
};