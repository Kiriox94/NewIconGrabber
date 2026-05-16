#pragma once

#include <borealis.hpp>
#include "utils/utils.hpp"

template <typename T>
T* addCell(brls::Box* container) {
    T* cell = new T();
    container->addView(cell);
    return cell;
}

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
