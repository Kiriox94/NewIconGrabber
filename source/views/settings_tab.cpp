#include "views/settings_tab.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include "utils/config.hpp"

using namespace brls::literals;  // for _i18n

template <typename T>
T* addCell(brls::Box* container) {
    T* cell = new T();
    container->addView(cell);
    return cell;
}

// brls::BooleanCell* addBooleanCell(brls::Box* container, std::string title, bool* setting) {
//     auto* cell = addCell<brls::BooleanCell>(container);
//     if (setting) {
//         cell->init(title, *setting, [setting](bool enabled) {
//             *setting = enabled;
//             config::save();
//         });
//     } else {
//         cell->setText(title);
//     }
//     return cell;
// }

#define ADD_BOOLEAN_SETTING(container, title, setting) addBooleanCell(container, title, *setting, [](bool e) {*setting = e;config::save();})
#define ADD_SELECTOR_SETTING(container, title, setting, options) addSelectorCell(container, title, *setting, options, [](int s) {*setting = s;config::save();})

template <typename Func>
brls::BooleanCell* addBooleanCell(brls::Box* container, std::string title, bool defaultValue, Func callback) {
    auto* cell = addCell<brls::BooleanCell>(container);
    cell->init(title, defaultValue, callback);
    return cell;
}

template <typename Func>
brls::SelectorCell* addSelectorCell(brls::Box* container, std::string title, int defaultValue, std::vector<std::string> options, Func callback) {
    auto* cell = addCell<brls::SelectorCell>(container);
    if (options.size() > 0) {
        cell->init(title, options, defaultValue, [](int s) {}, callback);
    }else {
        cell->setText(title);
    }
    return cell;
}

brls::Header* addHeader(brls::Box* container, std::string title, std::string subtitle = "") {
    auto* header = addCell<brls::Header>(container);
    header->setTitle(title);
    if (!subtitle.empty()) header->setSubtitle(subtitle);
    return header;     
}

SettingsTab::SettingsTab()
{
    brls::Logger::debug("SettingsTab init");
    this->inflateFromXMLRes("xml/tabs/settings.xml");

    std::vector<std::string> assetProfileNames;
    for (const auto& p : config::allowedAssetProfiles) {
        assetProfileNames.push_back(p.name);
    }

    std::vector<std::string> assetStyleNames;
    for (auto& p : config::allowedAssetStyles) {
        std::replace(p.begin(), p.end(), '_', ' ');
        assetStyleNames.push_back(utils::capitalizeWords(p));
    }

    std::vector<std::string> sortOrderNames;
    for (const auto& p : config::allowedSortsOrders) {
        sortOrderNames.push_back(p.second);
    }

    // topSysTweakButton = addCell<brls::DetailCell>(container);
    addHeader(container, "General");
    ADD_BOOLEAN_SETTING(container, "Display search results icons", &config::settings.displaySearchResultsIcons);
    ADD_BOOLEAN_SETTING(container, "Auto-select if perfect match", &config::settings.autoSelectIfPerfectMatch);
    ADD_BOOLEAN_SETTING(container, "Use English game titles", &config::settings.useEnglishGamesTitle);

    addHeader(container, "SteamGridDB");
    ADD_SELECTOR_SETTING(container, "Asset style", &config::settings.assetStyle, assetStyleNames);
    ADD_SELECTOR_SETTING(container, "Asset format", &config::settings.assetProfile, assetProfileNames);
    ADD_SELECTOR_SETTING(container, "Default sort order", &config::settings.sortOrder, sortOrderNames);
    ADD_BOOLEAN_SETTING(container, "Display mature assets", &config::settings.nsfw);

    // addHeader(container, "Update");
    // bottomSysTweakButton = addCell<brls::DetailCell>(container);
    // addBooleanCell(container, "Check for app updates at startup", &config::settings.ckeckForHomebrewUpdates);
    // addBooleanCell(container, "Check for Sys-tweak updates at startup", &config::settings.checkForSysTweakUpdates);

    addHeader(container, "Advanced");
    addBooleanCell(container, "Show debugging view", brls::Application::isDebuggingViewEnabled(), [](bool enabled) {
        brls::Application::enableDebuggingView(enabled);
    });
    addBooleanCell(container, "Show FPS", brls::Application::getFPSStatus(), [](bool enabled) {
        brls::Application::setFPSStatus(enabled);
    });
    addSelectorCell(container, "Log Level", (int)brls::Logger::getLogLevel(), {"Error", "Warning", "Info", "Debug", "Verbose"}, [](int level) {
        brls::Logger::setLogLevel((brls::LogLevel)level);
    });

    updatesystweakStatus();
}

brls::View* SettingsTab::create()
{
    return new SettingsTab();
}

void SettingsTab::updatesystweakStatus() {
    int sysTweakStatus = 0;
    long currentVersion = 0;
    brls::DetailCell* sysTweakButton;

    std::string sysTweakActionStr = "Set up Sys-tweak";
    std::string sysTweakStatusStr = "Sys-tweak is not installed.";

    if (fs::exists("sdmc:/atmosphere/contents/00FF747765616BFF/exefs.nsp")) {
        if (!fs::exists("sdmc:/atmosphere/contents/00FF747765616BFF/flags/boot2.flag")) {
            sysTweakStatusStr = "Sys-tweak is installed but not active.";
            sysTweakActionStr = "Activate Sys-tweak";
            sysTweakStatus = 1;
        } else {
            sysTweakActionStr = "Re-install Sys-tweak";
            sysTweakStatusStr = "Sys-tweak is installed and active.";
            sysTweakStatus = 2;
            if (fs::exists("sdmc:/atmosphere/contents/00FF747765616BFF/toolbox.json")) {
                std::ifstream i("sdmc:/atmosphere/contents/00FF747765616BFF/toolbox.json");
                if (i.is_open()) {
                    nlohmann::json j;
                    i >> j;
                    if(!j["version"].is_null()) {
                        currentVersion = j["version"].get<long>();
                        sysTweakActionStr = "Check for updates of Sys-tweak";
                        sysTweakStatusStr += fmt::format(" (v{})", currentVersion);
                    }
                }
            }
        }
    }

    if (topSysTweakButton == nullptr) sysTweakButton = bottomSysTweakButton;
    else if (bottomSysTweakButton == nullptr) sysTweakButton = topSysTweakButton;
    else if (sysTweakStatus != 2) {
        sysTweakButton = topSysTweakButton;
        bottomSysTweakButton->setVisibility(brls::Visibility::GONE);
    } else {
        sysTweakButton = bottomSysTweakButton;
        topSysTweakButton->setVisibility(brls::Visibility::GONE);
        bottomSysTweakButton->setActionAvailable(brls::ControllerButton::BUTTON_A, false);
    }

    if (sysTweakButton == nullptr) {
        brls::Logger::error("[Settings] Failed to find Sys-tweak button");
        return;
    }

    sysTweakButton->setVisibility(brls::Visibility::VISIBLE);
    sysTweakButton->setText(sysTweakActionStr);
    sysTweakButton->setDetailText(sysTweakStatusStr);
    sysTweakButton->registerAction("Refresh", brls::BUTTON_X, [this](brls::View*){
        updatesystweakStatus();
        return true;
    });

    sysTweakButton->registerClickAction([this, sysTweakStatus](...){
        switch (sysTweakStatus) {
            case 0:

                break;
            case 1: {
                std::ofstream file("sdmc:/atmosphere/contents/00FF747765616BFF/flags/boot2.flag");
                if (!file.is_open()) {
                    brls::Application::notify("Failed to enable Sys-Tweak");
                    brls::Logger::error("Failed to create boot2.flag");
                    return false;
                }

                file.close();
                brls::Application::notify("Sys-Tweak enabled");
                break;
            }
            case 2:

                break;
        }
        updatesystweakStatus();
        return true;
    });
}