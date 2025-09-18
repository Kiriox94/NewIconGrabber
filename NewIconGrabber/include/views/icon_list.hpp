#pragma once
#include "utils/utils.hpp"
#include "views/recycling_grid.hpp"

class IconCell : public RecyclingGridItem
{
  public:
    IconCell();
    // BRLS_BIND(brls::Label, label, "title");
    BRLS_BIND(brls::Image, image, "image");
};

class IconData : public RecyclingGridDataSource
{
  public:
    IconData(std::vector<std::string> data);
    size_t getItemCount() override;
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override;
    void onItemSelected(RecyclingGrid* recycler, size_t index) override;
    void clearData() override;
    void appendData(std::vector<std::string> data);
  private:
    std::vector<std::string> icons;
};

class IconListView : public brls::Box {
  public:
    IconListView(long gameId, std::string tid = "", int iconTexture = 0);
    ~IconListView();
    void requestAssets();
  private:
    long gameId;
    int pagesCount;
    int currentPage = 0;
    int sortOrder;
    BRLS_BIND(RecyclingGrid, recycler, "recycler"); 
};
