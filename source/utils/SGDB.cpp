#include "utils/SGDB.hpp"
#include "utils/http.hpp"

const std::string BaseUrl = "https://www.steamgriddb.com/api/public/";

namespace SGDB {
    SearchResult getAssetsForGame(long gameId, std::string assetType, std::string sortOrder, std::vector<std::string> assetResolutions, std::vector<std::string> assetStyles, int assetsLimit, int page, bool nsfw) {
        std::string url = BaseUrl + "search/assets";
        HTTP::Header header = {"Content-Type: application/json"};
        nlohmann::json body = {
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
        std::string response = HTTP::post(url, body.dump(), header);

        try {
            nlohmann::json j = nlohmann::json::parse(response);
            if (!j["success"]) {
                throw ApiException(j["errors"][0]);
            }
            return j["data"].get<SearchResult>();
        } catch (const std::exception& e) {
            throw ApiException(fmt::format("Failed to parse response: {}", e.what()));
        }
    }

    std::vector<SearchResult> searchGames(std::string searchTerm) {
        std::string url = BaseUrl + "search/main/games";
        HTTP::Header header = {"Content-Type: application/json"};
        nlohmann::json body = {
            {"asset_type", "icon"},
            {"filters", {{"order", "score_desc"}}},
            {"term", searchTerm},
        };
        std::string response = HTTP::post(url, body.dump(), header);

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
}