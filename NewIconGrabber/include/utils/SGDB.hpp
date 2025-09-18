#include <nlohmann/json.hpp>

namespace SGDB {
    struct Asset {
        long id;
        std::string style;
        bool nsfw = false;
        bool humor = false;
        std::string language;
        std::string thumb;
        long date;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(Asset, id, style, nsfw, humor, language, thumb, date)
    };

    struct Game {
        long id;
        std::string name;
        std::vector<std::string> types;
        time_t releaseDate;
    };

    inline void from_json(const nlohmann::json& j, Game& g) {
        j.at("id").get_to(g.id);
        j.at("name").get_to(g.name);
        if(!j.at("release_date").is_null()) j.at("release_date").get_to(g.releaseDate);
    }

    inline void to_json(nlohmann::json& j, const Game& g) {
        j["id"] = g.id;
        j["name"] = g.name;
        j["release_date"] = g.releaseDate;
    }

    struct SearchResult {
        Game game;
        int total = 0;
        std::vector<Asset> assets = {};
        NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(SearchResult, game, total, assets)
    };

    class ApiException : public std::exception {
        public:
            explicit ApiException(const std::string& message) : msg_(message) {}
            const char* what() const noexcept override {
                return msg_.c_str();
            }
        private:
            std::string msg_;
    };

    SearchResult getAssetsForGame(long gameId, std::string assetType, std::string sortOrder, std::vector<std::string> assetResolutions = {}, std::vector<std::string> assetStyles = {}, int maxAssetsPerPage = 48, int page = 0, bool nsfw = false);
    std::vector<SearchResult> searchGames(std::string searchTerm);
}