#pragma once

#include <borealis.hpp>
#include "views/svg_image.hpp"

class SkeletonImage : public SVGImage {
public:
    static SVGImage* create() { return new SkeletonImage(); }
    void draw(NVGcontext* vg, float x, float y, float width, float height,
              brls::Style style, brls::FrameContext* ctx) override;

private:
    NVGcolor background = brls::Application::getTheme()["color/grey_3"];
};
