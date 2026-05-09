#include <borealis.hpp>
#include <switch.h>
#include <filesystem>
#include <iostream>
#include "utils/config.hpp"
#include <nxtc.h>
#include <borealis/core/cache_helper.hpp>
#include "utils/thread.hpp"

#include "activity/main_activity.hpp"
#include "activity/appletmode_activity.hpp"
#include "views/settings_tab.hpp"
#include "views/game_list.hpp"
#include "views/auto_tab_frame.hpp"
#include "views/svg_image.hpp"
#include "views/local_icons.hpp"
#include "views/gallery_view.hpp"

int main(int argc, char* argv[]) {     
    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "-d") == 0) {
            brls::Logger::setLogLevel(brls::LogLevel::LOG_DEBUG);
        } else if (std::strcmp(argv[i], "-v") == 0) {
            brls::Logger::setLogLevel(brls::LogLevel::LOG_VERBOSE);
        } else if (std::strcmp(argv[i], "-dv") == 0) {
            brls::Application::enableDebuggingView(true);
        } else if (std::strcmp(argv[i], "-o") == 0) {
            const char* path = (i + 1 < argc) ? argv[++i] : "newicongrabber.log";
            brls::Logger::setLogOutput(std::fopen(path, "w+"));
        }
    }

    // Init services
#if __SWITCH__
    nsInitialize();
    nxtcInitialize();
    if (brls::Logger::getLogLevel() >= brls::LogLevel::LOG_DEBUG) {
        socketInitializeDefault();
        nxlinkStdio();
    }
#endif

    if(!brls::Application::init()) {
        brls::Logger::error("Unable to init Borealis application");
        return EXIT_FAILURE;
    }

    config::load();
    brls::TextureCache::instance().cache.setCapacity(600);

    brls::Application::createWindow("NewIconGrabber");
    brls::Application::setGlobalQuit(true);

    //XML View
    brls::Application::registerXMLView("SettingsTab", SettingsTab::create);
    brls::Application::registerXMLView("GameListView", GameListView::create);
    brls::Application::registerXMLView("LocalIconsView", LocalIconsView::create);
    brls::Application::registerXMLView("RecyclingGrid", RecyclingGrid::create);
    brls::Application::registerXMLView("AutoTabFrame", AutoTabFrame::create);
    brls::Application::registerXMLView("SVGImage", SVGImage::create);
    brls::Application::registerXMLView("GalleryView", GalleryView::create);

    // Add custom values to the theme
    brls::Theme::getLightTheme().addColor("captioned_image/caption", nvgRGB(2, 176, 183));
    brls::Theme::getDarkTheme().addColor("captioned_image/caption", nvgRGB(51, 186, 227));
    brls::Theme::getLightTheme().addColor("color/grey_1", nvgRGB(245, 246, 247));
    brls::Theme::getDarkTheme().addColor("color/grey_1", nvgRGB(51, 52, 53));
    brls::Theme::getLightTheme().addColor("color/grey_3", nvgRGBA(200, 200, 200, 16));
    brls::Theme::getDarkTheme().addColor("color/grey_3", nvgRGBA(160, 160, 160, 160));
    brls::Theme::getLightTheme().addColor("font/grey", nvgRGB(148, 153, 160));
    brls::Theme::getDarkTheme().addColor("font/grey", nvgRGB(148, 153, 160));

    if (brls::Application::getPlatform()->isApplicationMode()) {
        brls::Application::pushActivity(new MainActivity());
    } else {
        brls::Application::pushActivity(new AppletModeActivity());
    }

    while (brls::Application::mainLoop());

    // Exit services
    ThreadPool::instance().stop();
#ifdef __SWITCH__
    nxtcExit();
    nsExit();
    if (brls::Logger::getLogLevel() >= brls::LogLevel::LOG_DEBUG) {
        socketExit();
    }
#endif

    return EXIT_SUCCESS;
}