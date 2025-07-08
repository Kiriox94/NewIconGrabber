#include "utils/SGDB.hpp"
#include <curl/curl.h>

const std::string BaseUrl = "https://www.steamgriddb.com/api/public/";

namespace SGDB {
    SearchResult getAssetsForGame(long gameId, std::string assetType, std::string sortOrder, std::vector<std::string> assetResolutions, std::vector<std::string> assetStyles, int assetsLimit, int page, bool nsfw) {
        nlohmann::json j;
        CURL* curl;
        CURLcode res;
        curl = curl_easy_init();
        if (curl)
        {
            struct curl_slist* headers = NULL;

            headers = curl_slist_append(headers, "Content-Type: application/json");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            std::string response;
            std::string url = BaseUrl + "search/assets";
            nlohmann::json jsonData = {
                {"order", sortOrder},
                {"animated", false},
                {"asset_type", assetType},
                {"game_id", {gameId}},
                {"page", page},
                {"limit", assetsLimit},
                {"nsfw", nsfw},
                {"styles", assetStyles},
                {"dimensions", assetResolutions},
            };
            std::string jsonString = jsonData.dump();

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonString.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, utils::write_to_string);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

            res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);
            if (res != CURLE_OK)
            {
                throw ApiException(curl_easy_strerror(res));
            }
            else
            {
                try {
                    j = nlohmann::json::parse(response);
                    if (!j["success"]) {
                        throw ApiException(j["errors"][0]);
                    }
                    return j["data"].get<SearchResult>();
                } catch (const std::exception& e) {
                    throw ApiException(fmt::format("Failed to parse response: {}", e.what()));
                }
            }

        }else throw ApiException("Failed to initialize cURL.");

        return j;
    }

    std::vector<SearchResult> searchGames(std::string searchTerm) {
        CURL* curl;
        CURLcode res;
        curl = curl_easy_init();
        if (curl)
        {
            struct curl_slist* headers = NULL;
    
            headers = curl_slist_append(headers, "Content-Type: application/json");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
            std::string url = BaseUrl + "search/main/games";
            std::string response;
            nlohmann::json jsonData = {
                {"asset_type", "icon"},
                {"filters", {{"order", "score_desc"}}},

                {"term", searchTerm},
            };
            std::string jsonString = jsonData.dump();
    
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonString.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, utils::write_to_string);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);
            if (res != CURLE_OK)
            {
                throw ApiException(curl_easy_strerror(res));
            }
            else
            {
                try {
                    nlohmann::json j = nlohmann::json::parse(response);
                    if (!j["success"]) {
                        throw ApiException(j["errors"][0]);
                    }
                    return j["data"]["games"].get<std::vector<SearchResult>>();
                }catch (const std::exception& e) {
                    throw ApiException(fmt::format("Failed to parse response: {}", e.what()));
                }
            }
    
        }else throw ApiException("Failed to initialize cURL.");
    }
}