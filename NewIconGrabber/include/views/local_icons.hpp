#pragma once
#include <borealis.hpp>
#include "svg_image.hpp"
#include <borealis.hpp>
#include "views/auto_tab_frame.hpp"
#include <vector>
#include <filesystem>

#define INITIAL_DIRECTORY "sdmc:/config/NewIconGrabber/icons"

class LocalIconsView : public AttachedView {
  public:
    LocalIconsView();
    void onCreate() override;
    static brls::View* create();
    std::string currentDir = INITIAL_DIRECTORY;
  private:
    BRLS_BIND(brls::RecyclerFrame, recycler, "recycler");
};

class FileCell : public brls::RecyclerCell
{
  public:
  FileCell();

    BRLS_BIND(brls::Rectangle, accent, "brls/sidebar/item_accent");
    BRLS_BIND(brls::Label, title, "title");
    // BRLS_BIND(brls::Label, subtitle, "subtitle");
    BRLS_BIND(SVGImage, image, "image");

    static FileCell* create();
};

class FSData : public brls::RecyclerDataSource
{
  public:
    FSData(LocalIconsView* view);
    int numberOfSections(brls::RecyclerFrame* recycler) override;
    int numberOfRows(brls::RecyclerFrame* recycler, int section) override;
    brls::RecyclerCell* cellForRow(brls::RecyclerFrame* recycler, brls::IndexPath index) override;
    void didSelectRowAt(brls::RecyclerFrame* recycler, brls::IndexPath indexPath) override;
    std::string titleForHeader(brls::RecyclerFrame* recycler, int section) override;
  private:
    std::vector<std::filesystem::directory_entry> directoryEntries;
    std::vector<std::filesystem::directory_entry> fileEntries;
    bool hasDirectories = false;
};