#include "views/skeleton_image.hpp"

// brls::Image* SkeletonImage::create() { return new SkeletonImage(); }

void SkeletonImage::draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style style,
                        brls::FrameContext* ctx) {
    if (this->getTexture() == 0) {
        brls::Time curTime = brls::getCPUTimeUsec() / 1000;
        float p            = (curTime % 1000) * 1.0 / 1000;
        p                  = fabs(0.5 - p) + 0.25;

        NVGcolor end = background;
        end.a        = p;

        NVGpaint paint = nvgLinearGradient(vg, x, y, x + width, y + height, a(background), a(end));
        nvgBeginPath(vg);
        nvgFillPaint(vg, paint);
        nvgRoundedRect(vg, x, y, width, height, 6);
        nvgFill(vg);
    }else {
        SVGImage::draw(vg, x, y, width, height, style, ctx);
    }
}
