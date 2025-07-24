#pragma once

#include <borealis.hpp>
#include "utils/http.hpp"

class ImageHelper {
    using Ref = std::shared_ptr<ImageHelper>;

public:
    ImageHelper();
    ImageHelper(const ImageHelper&) = delete;

    virtual ~ImageHelper();

    /// @brief Set the image component to load the content. This function needs to work in the main thread.
    static void with(brls::Image* view, const std::string& url);

    /// @brief Cancel the request and clear the image. This function needs to work in the main thread.
    static void cancel(brls::Image* view);

private:
    void doRequest(HTTP& s);

    static void clear(brls::Image* view);

private:
    std::string url;
    brls::Image* image;
    HTTP::Cancel isCancel;

    /// Object Pool
    inline static std::list<Ref> pool;
    inline static std::mutex requestMutex;
    inline static std::unordered_map<brls::Image*, Ref> requests;
};