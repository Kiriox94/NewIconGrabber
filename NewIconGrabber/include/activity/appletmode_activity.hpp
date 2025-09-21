//
// Created by fang on 2022/8/21.
//

#pragma once

#include <borealis/core/activity.hpp>
#include <borealis/core/bind.hpp>

class GalleryView;

class AppletModeActivity : public brls::Activity {
public:
    // Declare that the content of this activity is the given XML file
    CONTENT_FROM_XML_STR(R"xml(
        <brls:AppletFrame
        iconInterpolation="linear"
        backgroundColor="@theme/brls/background"
        headerHidden="true"
        footerHidden="true">
            <brls:Box
                width="100%"
                height="100%">

                    <brls:Label
                        positionType="absolute"
                        positionTop="40"
                        positionLeft="92"
                        fontSize="28"
                        text="This homebrew need to be run in applet mod, please relaunch it from an app"/>
                <GalleryView
                        wireframe="false"
                        width="100%"
                        height="95%"
                        paddingLeft="100"
                        paddingRight="100"
                        hideHighlight="true"
                        id="hint/gallery"/>
                <brls:Label
                        positionType="absolute"
                        positionBottom="10"
                        positionRight="10"
                        fontSize="12"
                        textColor="@theme/font/grey"
                        text="The pictures are from the Internet, and the copyright belongs to Nintendo Co., Ltd"/>
            </brls:Box>
        </brls:AppletFrame>
    )xml");

    AppletModeActivity();

    void onContentAvailable() override;

    ~AppletModeActivity();

private:
    BRLS_BIND(GalleryView, gallery, "hint/gallery");
};