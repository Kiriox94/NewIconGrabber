#include "views/icon_list.hpp"
#include <nlohmann/json.hpp>
#include <borealis/core/cache_helper.hpp>
#include "utils/SGDB.hpp"
#include "views/game_list.hpp"
#include "utils/config.hpp"
#include "utils/threads_manager.hpp"
#include "activity/main_activity.hpp"
#include "curl/curl.h"

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
    int tex = brls::TextureCache::instance().getCache(url);
    cell->image->setFreeTexture(false);
    if (tex > 0) {
        cell->image->innerSetImage(tex);
    } else {
        cell->image->setImageFromRes("img/placeholder.png");
        cell->ptrLock();
        ThreadsManager::getImagesPool().detach_task([cell, url]() {
            try {
                CURL* curl;
                std::vector<char> imageBuffer;
                CURLcode res;
                curl = curl_easy_init();
                if (curl)
                {
                    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, utils::write_to_memory);
                    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &imageBuffer);
                    res = curl_easy_perform(curl);
                    curl_easy_cleanup(curl);

                    brls::sync([cell, url, imageBuffer] {
                        cell->image->setImageFromMem(reinterpret_cast<const unsigned char*>(imageBuffer.data()), imageBuffer.size());
                        brls::TextureCache::instance().addCache(url, cell->image->getTexture());
                        cell->ptrUnlock();
                    });
                }
            } catch (const std::exception& ex) {
                // cell->image->setImageFromRes("img/error.png");
                brls::Logger::error("request image {} {}", url, ex.what());
            }
        });
    }
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
                CURL* curl;
                std::vector<unsigned char> imageBuffer;
                CURLcode res;
                curl = curl_easy_init();
                if (curl)
                {
                    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, utils::write_to_memory);
                    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &imageBuffer);
                    res = curl_easy_perform(curl);
                    curl_easy_cleanup(curl);
                    utils::overwriteIcon(outPath, "", imageBuffer);
                    brls::Application::notify("Icon applied");
                    brls::Application::popActivity();
                    brls::Application::pushActivity(new MainActivity());
                }
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
    icons.reserve(icons.size() + data.size()); // optimisation mémoire
    icons.insert(icons.end(), data.begin(), data.end());
}

void IconData::clearData() {
    icons.clear();
}

IconListView::IconListView(long SGDBGameId, std::string tid) {
    gameId = SGDBGameId;
    titleId = tid;
    sortOrder = config::settings.sortOrder;
    this->inflateFromXMLRes("xml/views/game_list.xml");

    getAppletFrameItem()->title = "Loading…";
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
            ThreadsManager::getRequestsPool().detach_task([this]() { requestAssets(); });
        }
    });
    // ThreadsManager::getRequestsPool().detach_task([this]() { requestAssets(); });
    requestAssets();
}

IconListView::~IconListView() {
    utils::setHeaderVisibility(false);
}

void IconListView::requestAssets() {
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

    std::string title = "";
    std::vector<std::string> icons;
    
    try{
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
    }catch (const SGDB::ApiException& e) {
        title = "Failed to fetch icons.";
        recycler->setError(e.what());
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
