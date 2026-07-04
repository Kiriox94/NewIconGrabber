#include "views/icons_grid.hpp"
#include <nlohmann/json.hpp>
#include <borealis/core/cache_helper.hpp>
#include "utils/SGDB.hpp"
#include "views/game_list.hpp"
#include "utils/config.hpp"
#include "activity/main_activity.hpp"
#include "utils/image_helper.hpp"
#include "utils/thread.hpp"
#include "utils/icons_files_helper.hpp"
#include "utils/borealis_helper.hpp"

std::string titleId;

IconCell::IconCell()
{
    // this->inflateFromXMLRes("xml/cells/game_cell.xml");
    this->inflateFromXMLString(R"(
    <brls:Box
    axis="column"
    alignItems="stretch"
    focusable="true"
    cornerRadius="12"
    highlightCornerRadius="15">

        <SkeletonImage
            id="image"
            grow="1"
            scalingType="fill"
            cornerRadius="12"
            backgroundColor="#00000030"/>
    </brls:Box>
    )");
}

IconData::IconData(std::vector<std::string> data) {
    icons = data;
}

RecyclingGridItem* IconData::cellForRow(RecyclingGrid* recycler, size_t index)
{
    auto cell = (IconCell*)recycler->dequeueReusableCell("Cell");
    // brls::Logger::info("Cell width: {}", cell->getWidth());

    std::string url = icons[index];
    ImageHelper::with(cell->image, url);
    return cell;
}

size_t IconData::getItemCount() {
    return icons.size();
}

void IconData::onItemSelected(RecyclingGrid* recycler, size_t index)
{
    std::string url = icons[index];
    auto callback = [recycler, url](std::string tid) {
        if (!tid.empty()) {
            try {
                std::string response = HTTP::get(url);
                std::vector<uint8_t> imageBuffer(response.begin(), response.end());
                iconsFilesHelper::overwriteIcon(tid, "", imageBuffer);
                brls::Application::notify("Icon applied");
                recycler->getAppletFrame()->popToRootContentView();
            } catch(const iconsFilesHelper::OverwriteIconException& e) {
                brls::Application::notify(fmt::format("Icon Error: {}", e.what()));
            } catch(const std::exception& e) {
                brls::Application::notify("Error: Icon not applied");
            }
        }
    };

    if (!titleId.empty()) {
        callback(titleId);
    }else {
        recycler->present(new GameListView(callback));
    }
}

void IconData::appendData(std::vector<std::string> data) {
    icons.reserve(icons.size() + data.size()); // memory optimisation
    icons.insert(icons.end(), data.begin(), data.end());
}

void IconData::clearData() {
    icons.clear();
}

IconListView::IconListView(long SGDBGameId, std::string tid, std::string iconUrl) {
    gameId = SGDBGameId;
    titleId = tid;
    sortOrder = config::settings.sortOrder;
    this->inflateFromXMLRes("xml/views/game_list.xml");

    this->getAppletFrameItem()->title = "Loading…";
    int iconTex = brls::TextureCache::instance().getCache(iconUrl);

    // If icon is already cached, set it immediately.
    // Otherwise, wait for it to be cached, by checking every 0.5s up to 5 times and set it there.
    if (iconTex != 0) {
        this->getAppletFrameItem()->setIconFromTexture(iconTex);
    } else {
        brls::async([this, iconUrl]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            for (int i = 0; i < 5; i++) {
                int iconTex = brls::TextureCache::instance().getCache(iconUrl);
                if (iconTex != 0) {
                    brls::sync([this, iconTex]() {
                        this->getAppletFrameItem()->setIconFromTexture(iconTex);
                        this->updateAppletFrameItem();
                    });
                    return;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
        });
    }
    utils::setHeaderVisibility(true);

    std::vector<std::string> sortOrderNames;
    for (const auto& p : config::allowedSortsOrders) {
        sortOrderNames.push_back(p.second);
    }

    recycler->registerAction("Sort Order", brls::BUTTON_X, [this, sortOrderNames](brls::View* view) {
        auto* dropdown = new brls::Dropdown("Set Sort Order", sortOrderNames, [this](int index) {
            sortOrder = index;
            currentPage = 0;

            recycler->clearData();
            recycler->showSkeleton();
            
            ThreadPool::instance().submit([this](HTTP& s) {
                this->requestAssets();
            });
        }, sortOrder);
        brls::Application::pushActivity(new brls::Activity(dropdown), brls::TransitionAnimation::FADE);
        return true;
    });

    recycler->estimatedRowHeight = config::getCurrentAssetProfil().rowHeight;
    recycler->spanCount = config::getCurrentAssetProfil().spanCount;
    recycler->registerCell("Cell", []() { return new IconCell();});
    recycler->onNextPage([this] {
        if (currentPage < pagesCount) {
            currentPage++;
            ThreadPool::instance().submit([this](HTTP& s) {
                this->requestAssets();
            });
        }
    });

    ThreadPool::instance().submit([this](HTTP& s) {
        this->requestAssets();
    });
}

IconListView::~IconListView() {
    utils::setHeaderVisibility(false);
}

void IconListView::requestAssets() {
    std::string title = "";
    std::vector<std::string> icons;
    
    try {
        SGDB::SearchResult result = SGDB::getAssetsForGame(
            gameId, 
            "grid", 
            config::allowedSortsOrders[sortOrder].first, 
            config::getCurrentAssetProfil().assetResolutions, 
            {config::getCurrentAssetStyle()}, 
            config::getCurrentAssetProfil().pageSize, 
            currentPage, 
            config::settings.nsfw
        );

        if (result.total <= 0) {
            title = "No icons found";
            recycler->setEmpty();
        }else {
            pagesCount = std::ceil(result.total / config::getCurrentAssetProfil().pageSize);
            title = fmt::format("{} ({} icons)", result.game.name, result.total);

            for (auto& asset : result.assets) {
                std::string url = asset.thumb;
                icons.push_back(url);
            }
        }
    } catch (const std::exception& e) {
        title = "Failed to fetch icons";
        recycler->setError(fmt::format("Request error: {}", e.what()));
    }
    
    brls::sync([this, title, icons]() {
        IconData* dataSource = dynamic_cast<IconData*>(recycler->getDataSource());
        this->getAppletFrameItem()->title = title;
        this->updateAppletFrameItem();
        
        if (dataSource && dataSource->getItemCount() > 0) {
            dataSource->appendData(icons);
            recycler->notifyDataChanged();
        }else {
            recycler->setDataSource(new IconData(icons));
            brls::Application::giveFocus(recycler);
        }
    });
}
