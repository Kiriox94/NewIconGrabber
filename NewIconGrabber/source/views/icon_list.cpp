#include "views/icon_list.hpp"
#include <nlohmann/json.hpp>
#include <borealis/core/cache_helper.hpp>
#include "utils/SGDB.hpp"
#include "views/game_list.hpp"
#include "utils/config.hpp"
#include "activity/main_activity.hpp"
#include "utils/image_helper.hpp"
#include "utils/thread.hpp"

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

        <brls:Image
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
    cell->image->setImageFromRes("img/placeholder.png");
    ImageHelper::with(cell->image, url);
    return cell;
}

size_t IconData::getItemCount() {
    return icons.size();
}

void IconData::onItemSelected(RecyclingGrid* recycler, size_t index)
{
    std::string url = icons[index];
    auto callback = [url](std::string tid) {
        if (!tid.empty()) {
            std::string outPath = utils::getIconPath(tid);
            try
            {
                std::string response = HTTP::get(url);
                std::vector<uint8_t> imageBuffer(response.begin(), response.end());
                utils::overwriteIcon(outPath, "", imageBuffer);
                brls::Application::notify("Icon applied");
                brls::Application::popActivity();
                brls::Application::pushActivity(new MainActivity());
            }
            catch(const utils::OverwriteIconException e)
            {
                brls::Application::notify(fmt::format("Icon Error: {}", e.what()));
            }
            catch(const std::exception& e)
            {
                brls::Application::notify("Error: Icon not applied");
            }
        }
    };

    if (!titleId.empty()) {
        callback(titleId);
    }else {
        auto* frame = new brls::AppletFrame(static_cast<brls::Box*>(new GameListView(callback)));
        brls::Application::pushActivity(new brls::Activity(frame));
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
    this->getAppletFrameItem()->setIconFromTexture(brls::TextureCache::instance().getCache(iconUrl));
    utils::setHeaderVisibility(true);

    std::vector<std::string> sortOrderNames;
    for (const auto& p : config::allowedSortsOrders) {
        sortOrderNames.push_back(p.second);
    }
    recycler->registerAction("Sort Order", brls::BUTTON_X, [this, sortOrderNames](brls::View* view) {
        auto* dropdown = new brls::Dropdown("Set Sort Order", sortOrderNames, [this](int index) {;
            sortOrder = index;
            recycler->clearData();
            recycler->showSkeleton();
            currentPage = 0;
            requestAssets();
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
            ThreadPool::instance().submit([this](HTTP& s) { this->requestAssets(); });
        }
    });
    requestAssets();
}

IconListView::~IconListView() {
    utils::setHeaderVisibility(false);
}

void IconListView::requestAssets() {
    std::string title = "";
    std::vector<std::string> icons;
    
    try{
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
            title = "No icons found.";
            recycler->setEmpty();
        }else {
            pagesCount = std::ceil(result.total / config::getCurrentAssetProfil().pageSize);
            title = fmt::format("Icons for {} ({})", result.game.name, result.total);

            for (auto& asset : result.assets) {
                std::string url = asset.thumb;
                icons.push_back(url);
            }
        }
    }catch (const std::exception& e) {
        title = "Failed to fetch icons.";
        recycler->setError(fmt::format("Request error: {}", e.what()));
    }
    
    brls::sync([this, title, icons]() {
        IconData* dataSource = dynamic_cast<IconData*>(recycler->getDataSource());
        recycler->getParent()->getAppletFrame()->setTitle(title);
        
        if (dataSource && dataSource->getItemCount() > 0) {
            dataSource->appendData(icons);
            recycler->notifyDataChanged();
        }else {
            recycler->setDataSource(new IconData(icons));
        }

        brls::Application::giveFocus(recycler);
    });
}
