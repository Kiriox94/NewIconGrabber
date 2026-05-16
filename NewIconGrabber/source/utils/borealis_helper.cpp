#include "utils/borealis_helper.hpp"
#include <borealis.hpp>

namespace utils {
    void setHeaderVisibility(bool visible) {
        brls::sync([visible]() {
            brls::Logger::debug("Setting activities header visibility to {}", visible ? "VISIBLE" : "GONE");
            auto stack = brls::Application::getActivitiesStack();
            for (auto& activity : stack) {
                auto* frame = dynamic_cast<brls::AppletFrame*>(activity->getContentView());
                if (!frame) continue;
                frame->setHeaderVisibility(visible ? brls::Visibility::VISIBLE : brls::Visibility::GONE);
            }
        });
    }
}
