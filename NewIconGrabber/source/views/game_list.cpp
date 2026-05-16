#include "views/game_list.hpp"
#include "views/search_games.hpp"
#include <borealis/core/cache_helper.hpp>
#include "utils/app_metadata_helper.hpp"
#include "utils/icons_files_helper.hpp"
#include <filesystem>

GameCell::GameCell()
{
    this->inflateFromXMLRes("xml/cells/game_cell.xml");
}

void GameCell::onFocusGained() {
    Box::onFocusGained();
    label->setSingleLine(false);
}

void GameCell::onFocusLost() {
    Box::onFocusLost();
    label->setSingleLine(true);
}

GameData::GameData(std::function<void(std::string)> callback) {
    selectCallback = callback;
    games = appMetadataHelper::getInstalledGames();
    brls::Logger::debug("GameData Init: {} games found", games.size());
}

RecyclingGridItem* GameData::cellForRow(RecyclingGrid* recycler, size_t index)
{
    std::string tid = utils::formatApplicationId(games[index]->title_id);
    auto cell = (GameCell*)recycler->dequeueReusableCell("Cell");
    cell->label->setText(games[index]->name);
    cell->label->setTextColor(nvgRGB(255, 255, 255));

    cell->image->setFreeTexture(false);
    int tex = brls::TextureCache::instance().getCache(tid);
    if (tex > 0) {
        cell->image->innerSetImage(tex);
    } else {
        cell->image->setImageFromMem(static_cast<u8*>(games[index]->icon_data), games[index]->icon_size);
        brls::TextureCache::instance().addCache(tid, cell->image->getTexture());
    }

    std::string iconDir = iconsFilesHelper::getIconDir(tid);
    std::string iconPath = fmt::format("{}/icon.jpg", iconDir);
    if (std::filesystem::exists(iconPath) && !selectCallback) cell->registerAction("Remove custom icon", brls::ControllerButton::BUTTON_Y, [iconPath, iconDir](brls::View* view) {
        view->setActionAvailable(brls::ControllerButton::BUTTON_Y, false);
        if (!std::filesystem::exists(iconPath)) return false;

        if(!std::filesystem::remove(iconPath)) {
            brls::Application::notify("Failed to remove icon");
            return false;
        }

        brls::Application::notify("Custom icon removed");

        std::string smallIconPath = fmt::format("{}/icon174.jpg", iconDir);
        if (std::filesystem::exists(smallIconPath)) std::filesystem::remove(smallIconPath);

        if (std::filesystem::is_empty(iconDir)) std::filesystem::remove(iconDir);
        return true;
    });
    return cell;
}

size_t GameData::getItemCount() {
    return games.size();
}

void GameData::onItemSelected(RecyclingGrid* recycler, size_t index)
{
    std::string tid = utils::formatApplicationId(games[index]->title_id);
    if (!selectCallback) {
        recycler->present(new SearchGamesView(games[index]->name, tid));
    }else {
        selectCallback(tid);
    }
}

void GameData::clearData() {
    games.clear();
}

GameListView::GameListView(std::function<void(std::string)> callback) {
    selectCallback = callback;
    this->inflateFromXMLRes("xml/views/game_list.xml");
    if (selectCallback) {
        getAppletFrameItem()->title = "Select game to apply icon";
    }

    recycler->estimatedRowHeight = 150;
    recycler->spanCount = 7;
    recycler->registerCell("Cell", []() { return new GameCell();});
    recycler->setDataSource(new GameData(callback));
}

void GameListView::onCreate() {
    if (!selectCallback) this->registerTabAction("Search", brls::ControllerButton::BUTTON_X, [this](brls::View* view) {
        brls::Application::getImeManager()->openForText([&](std::string text) {
            recycler->present(new SearchGamesView(text));
        }, "Search for a game with title");
        return true;
    });
}

brls::View* GameListView::create() {
    return new GameListView();
}