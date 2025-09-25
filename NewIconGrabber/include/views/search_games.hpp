#pragma once
#include "utils/utils.hpp"
#include "svg_image.hpp"

struct SGDBEntry{
  long id = 0;
  std::string name;
  std::string year;
  std::string iconUrl;
};

class SGDBCell : public brls::RecyclerCell
{
  public:
    SGDBCell();

    BRLS_BIND(brls::Rectangle, accent, "brls/sidebar/item_accent");
    BRLS_BIND(brls::Label, label, "title");
    BRLS_BIND(SVGImage, image, "image");

    static SGDBCell* create();
};

class SGDBData : public brls::RecyclerDataSource
{
  public:
    SGDBData();
    int numberOfSections(brls::RecyclerFrame* recycler) override;
    int numberOfRows(brls::RecyclerFrame* recycler, int section) override;
    brls::RecyclerCell* cellForRow(brls::RecyclerFrame* recycler, brls::IndexPath index) override;
    void didSelectRowAt(brls::RecyclerFrame* recycler, brls::IndexPath indexPath) override;
    std::string titleForHeader(brls::RecyclerFrame* recycler, int section) override;
};

class SearchGamesView : public brls::Box {
    public:
      SearchGamesView(std::string gameName, std::string titleId = "");
      void onChildFocusGained(View* directChild, View* focusedView) override;
    private:
      SGDBEntry foundGame;
      BRLS_BIND(brls::RecyclerFrame, recycler, "recycler");
};