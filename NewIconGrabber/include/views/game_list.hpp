#pragma once
#include "recycling_grid.hpp"
#include "auto_tab_frame.hpp"

#ifdef __SWITCH__
#include <nxtc.h>
#endif

class GameCell : public RecyclingGridItem
{
public:
  GameCell();
  void onFocusGained() override;
  void onFocusLost() override;

  BRLS_BIND(brls::Label, label, "title");
  BRLS_BIND(brls::Image, image, "image");
};

#ifdef __SWITCH__
class GameData : public RecyclingGridDataSource
{
public:
;
  GameData(std::function<void(std::string)> callback);
  size_t getItemCount() override;
  RecyclingGridItem *cellForRow(RecyclingGrid *recycler, size_t index) override;
  void onItemSelected(RecyclingGrid *recycler, size_t index) override;
  void clearData() override;

private:
  std::function<void(std::string)> selectCallback;
  std::vector<NxTitleCacheApplicationMetadata*> games;
};
#endif

class GameListView : public AttachedView
{
public:
  GameListView(std::function<void(std::string)> callback = nullptr);
  void onCreate() override;
  static brls::View *create();

private:
  std::function<void(std::string)> selectCallback;
  BRLS_BIND(RecyclingGrid, recycler, "recycler");
};
