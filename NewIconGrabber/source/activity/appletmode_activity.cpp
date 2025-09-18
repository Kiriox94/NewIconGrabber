//
// Created by fang on 2022/8/21.
//

#include <borealis/core/i18n.hpp>
#include <borealis/core/touch/tap_gesture.hpp>
#include <borealis/views/label.hpp>
#include <borealis/views/dialog.hpp>

#include "activity/appletmode_activity.hpp"
#include "views/gallery_view.hpp"

#ifdef BUILTIN_NSP
#include "nspmini.hpp"
#endif

using namespace brls::literals;

AppletModeActivity::AppletModeActivity() {
    brls::Logger::debug("AppletModeActivity: create");
}

void AppletModeActivity::onContentAvailable() {
    brls::Logger::debug("AppletModeActivity: onContentAvailable");

    gallery->setData({
        {"img/hint_game_1.png", "Select a game from home screen"},
        {"img/hint_game_2.png", "Hold the \uE0E5 key and click \uE0E0 to enter hbmenu"},
        {"img/hint_hbmenu.png", "In the hbmenu, select wiliwili"},
    });
}

AppletModeActivity::~AppletModeActivity() { brls::Logger::debug("AppletModeActivity: delete"); }