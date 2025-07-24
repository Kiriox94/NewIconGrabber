#include <borealis.hpp>
#include <switch.h>
#include <filesystem>
#include <iostream>
#include "utils/config.hpp"
#include <nxtc.h>
#include <borealis/core/cache_helper.hpp>
#include "utils/thread.hpp"

#include "activity/main_activity.hpp"
#include "views/settings_tab.hpp"
#include "views/game_list.hpp"
#include "views/auto_tab_frame.hpp"
#include "views/svg_image.hpp"
#include "views/local_icons.hpp"

void init();
void exit();

int main(int argc, char* argv[]) {     
    init();
    brls::Logger::setLogLevel(brls::LogLevel::LOG_DEBUG);
    // brls::Application::enableDebuggingView(true);

    #ifdef NDEBUG //release
        // Using FILE* because brls::Logger::setLogOutput only takes FILE*, not std::ofstream
        FILE* logFile = fopen("sdmc:/config/NewIconGrabber/log.log", "w");
        brls::Logger::setLogOutput(logFile);
    #endif

    if(!brls::Application::init()) {
        brls::Logger::error("Unable to init Borealis application");
    }

    config::load();
    brls::loadTranslations();
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

    // Add custom values to the theme
    brls::Theme::getLightTheme().addColor("captioned_image/caption", nvgRGB(2, 176, 183));
    brls::Theme::getDarkTheme().addColor("captioned_image/caption", nvgRGB(51, 186, 227));
    brls::Theme::getLightTheme().addColor("color/grey_1", nvgRGB(245, 246, 247));
    brls::Theme::getDarkTheme().addColor("color/grey_1", nvgRGB(51, 52, 53));
    brls::Theme::getLightTheme().addColor("color/grey_3", nvgRGBA(200, 200, 200, 16));
    brls::Theme::getDarkTheme().addColor("color/grey_3", nvgRGBA(160, 160, 160, 160));

    brls::Application::pushActivity(new MainActivity());

    while (brls::Application::mainLoop());
    ThreadPool::instance().stop();

    exit();
    return -1;
}

void init() {
    setsysInitialize();
    socketInitializeDefault();
    nxlinkStdio();
    plInitialize(PlServiceType_User);
    nsInitialize();
    nxtcInitialize();
    pmdmntInitialize();
    pminfoInitialize();
    splInitialize();
    fsInitialize();
    romfsInit();
    setInitialize();
    psmInitialize();
    nifmInitialize(NifmServiceType_User);
    lblInitialize();
}

void exit() {
    lblExit();
    nifmExit();
    psmExit();
    setExit();
    romfsExit();
    splExit();
    pminfoExit();
    pmdmntExit();
    nxtcExit();
    nsExit();
    setsysExit();
    fsExit();
    plExit();
    socketExit();
}