#include "utils/image.hpp"
#include <borealis/core/cache_helper.hpp>
#include <curl/curl.h>
#include <iostream>
#include <fstream>
#include <thread>
#include "utils/threads_manager.hpp"

Image::Image() : image(nullptr) {
    brls::Logger::verbose("new Image {}", fmt::ptr(this));
}

Image::~Image() { brls::Logger::verbose("delete Image {}", fmt::ptr(this)); }

void Image::fromUrl(brls::Image* view, const std::string& url, std::string path) {
    if (!path.empty() && fs::exists(path) && fs::file_size(path) > 0) {
        view->setImageFromFile(path);
        return;
    }
    
    int tex = brls::TextureCache::instance().getCache(url);
    if (tex > 0) {
        
        view->innerSetImage(tex);
        return;
    }else brls::Logger::info("Image {} not found in cache", url);

    view->setImageFromRes("img/placeholder.png");

    Ref item;
    std::lock_guard<std::mutex> lock(requestMutex);

    if (pool.empty()) {
        item = std::make_shared<Image>();
    } else {
        item = pool.front();
        pool.pop_front();
    }

    auto it = requests.insert(std::make_pair(view, item));
    if (!it.second) {
        brls::Logger::warning("insert Image {} failed", fmt::ptr(view));
        return;
    }

    item->image = view;
    item->url = url;
    item->path = path;
    view->ptrLock();
   
    view->setFreeTexture(false); // Set the image component not to handle texture destruction, the cache manages texture destruction uniformly
    ThreadsManager::getImagesPool().detach_task([item]() { item->doRequest(); });
}

void Image::cancel(brls::Image* view) {
    brls::TextureCache::instance().removeCache(view->getTexture());
    view->clear();

    clear(view);
}

void Image::doRequest() {
    brls::Logger::info("Downloading image: {}", url);

    try {
        CURL* curl;
        std::vector<char> imageBuffer;
        CURLcode res;
        curl = curl_easy_init();
        if (curl)
        {
            curl_easy_reset(curl);
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, utils::write_to_memory);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &imageBuffer);
            res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);
        }

        brls::sync([this, imageBuffer] {
            if (!image) return;
            image->setImageFromMem(reinterpret_cast<const unsigned char*>(imageBuffer.data()), imageBuffer.size());

            // if (!path.empty()) {
            //     std::ofstream file(path, std::ios::out | std::ios::binary);
            //     file.write(imageBuffer.data(), imageBuffer.size());
            // } else {
            // }

            brls::TextureCache::instance().addCache(url, image->getTexture());
            Image::clear(image);
        });
    } catch (const std::exception& ex) {
        brls::Logger::warning("request image {} {}", url, ex.what());
        if (image)
            Image::clear(image);
    }
}


void Image::clear(brls::Image* view) {
    std::lock_guard<std::mutex> lock(requestMutex);

    auto it = requests.find(view);
    if (it == requests.end()) return;

    it->second->image->ptrUnlock();
    it->second->image = nullptr;
    it->second->url.clear();
    it->second->path.clear();
    pool.push_back(it->second);
    requests.erase(it);
}