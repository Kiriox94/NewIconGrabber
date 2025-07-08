#pragma once

#include <borealis.hpp>
#include "utils/utils.hpp"

class SettingsTab : public brls::Box
{
    public:
        SettingsTab();
        static brls::View* create();
    private:
        BRLS_BIND(brls::Box, container, "container");
        brls::DetailCell* topSysTweakButton = nullptr;
        brls::DetailCell* bottomSysTweakButton = nullptr;
        void updatesystweakStatus();
};
