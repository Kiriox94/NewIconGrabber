#include "utils/config.hpp"
#include "utils/utils.hpp"
#include <fstream>
#include <borealis/core/logger.hpp>

const std::string configDirectory = utils::getRootDirectory() + "/config/NewIconGrabber";
const std::string configFilePath = configDirectory + "/config.json";

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
        try {
            if (std::filesystem::exists(configFilePath)) {
                brls::Logger::info("Loading config from file");

                std::ifstream i(configFilePath);

                nlohmann::json j;
                i >> j;

                settings = j.get<ParsedConfig>();
            } else {
                brls::Logger::info("Using default config");
            }
        }
        catch (const nlohmann::json::exception& e) {
            brls::Logger::error("Error loading config: {}", e.what());
        }
    }

    void save() {
        try {
            if (!std::filesystem::exists(configDirectory))
                if (!std::filesystem::create_directories(configDirectory))
                    brls::Logger::error("Could not create config directory");

            std::ofstream o(configFilePath);
            nlohmann::json j = settings;
            o << j.dump(4) << std::endl;
            brls::Logger::info("Saved config");
            o.close();
        }
        catch (const nlohmann::json::exception& e) {
            brls::Logger::error("Error saving config: {}", e.what());
        }
    }  
} // namespace config