#pragma once
#include "utils.hpp"

#define CONFIG_PATH "sdmc:/config/NewIconGrabber/config.json"

struct AssetProfil{
    std::string name;
    int spanCount;
    int rowHeight;
    int pageSize;
    std::vector<std::string> assetResolutions;
};

struct ParsedConfig {
    int assetProfile = 0;
    int assetStyle = 0;
    int sortOrder = 0;
    bool nsfw = false;

    bool displaySearchResultsIcons = true;
    bool autoSelectIfPerfectMatch = true;
    bool useEnglishGamesTitle = true;

    bool ckeckForHomebrewUpdates = true;
    bool checkForSysTweakUpdates = true;

    bool showDebuggingView = false;
    bool showFPS = false;
    int logLevel = 0; 

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ParsedConfig, assetProfile, assetStyle, sortOrder, nsfw, displaySearchResultsIcons, autoSelectIfPerfectMatch, useEnglishGamesTitle, ckeckForHomebrewUpdates, checkForSysTweakUpdates, showDebuggingView, showFPS, logLevel);
};

namespace config {
    inline ParsedConfig settings;

    inline std::vector<AssetProfil> allowedAssetProfiles = {
        {"Square", 7, 150, 42, {"512x512", "1024x1024"}},
        {"Vertical", 7, 230, 42, {"600x900", "660x930", "342x482"}},
        {"Horizontal", 4, 130, 48, {"460x215", "920x430"}}
    };
    inline std::vector<std::string> allowedAssetStyles = {
        "all",
        "alternate",
        "blurred",
        "white_logo",
        "material",
        "no_logo"
    };

    inline std::vector<std::pair<std::string, std::string>> allowedSortsOrders = {
        {"score_desc", "Highest Score"},
        {"score_asc", "Lowest Score"},
        {"score_old_desc", "Highest Score (Old)"},
        {"score_old_asc", "Lowest Score (Old)"},
        {"age_desc", "Newest First"},
        {"age_asc", "Oldest First"}
    };
        

    AssetProfil getCurrentAssetProfil();
    std::string getCurrentAssetStyle();
    std::string getCurrentSortOrder();

    void load();
    void save();
};