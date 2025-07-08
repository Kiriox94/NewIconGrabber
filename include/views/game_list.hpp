#pragma once
#include "utils/utils.hpp"
#include "recycling_grid.hpp"
#include "auto_tab_frame.hpp"
#include <nxtc.h>

class GameCell : public RecyclingGridItem
{
public:
  GameCell();
  void onFocusGained() override;
  void onFocusLost() override;

  BRLS_BIND(brls::Label, label, "title");
  BRLS_BIND(brls::Image, image, "image");
};

class GameData : public RecyclingGridDataSource
{
public:
  GameData();
  size_t getItemCount() override;
  RecyclingGridItem *cellForRow(RecyclingGrid *recycler, size_t index) override;
  void onItemSelected(RecyclingGrid *recycler, size_t index) override;
  void clearData() override;

private:
  std::vector<NxTitleCacheApplicationMetadata*> games;
};

class GameListView : public AttachedView
{
public:
  GameListView(std::function<void(std::string)> callback = nullptr);
  void onCreate() override;
  static brls::View *create();

private:
  BRLS_BIND(RecyclingGrid, recycler, "recycler");
};
