#include "utils/config.hpp"
#include <fstream>


namespace config {
    AssetProfil getCurrentAssetProfil() {
        return allowedAssetProfiles[settings.assetProfile];
    }

    std::string getCurrentAssetStyle() {
        return allowedAssetStyles[settings.assetStyle];
    }

    std::string getCurrentSortOrder() {
        return allowedSortsOrders[settings.sortOrder].first;
    }

    void load() {
        if (std::filesystem::exists(CONFIG_PATH)) {
            brls::Logger::info("Loading config from file");
            std::ifstream i(CONFIG_PATH);
            nlohmann::json j;
            i >> j;
            settings = j.get<ParsedConfig>();
        } else {
            brls::Logger::info("Using default config");
        }
    }

    void save() {
        if (!std::filesystem::exists("sdmc:/config/NewIconGrabber/"))
            if (!std::filesystem::create_directories("sdmc:/config/NewIconGrabber/"))
                brls::Logger::error("Could not create config directory");

        std::ofstream o(CONFIG_PATH);
        nlohmann::json j = settings;
        o << j.dump(4) << std::endl;
        brls::Logger::info("Saved config");
    }  
} // namespace config