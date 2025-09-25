#include "views/search_games.hpp"
#include "views/icon_list.hpp"
#include <ctime>
#include <borealis/core/cache_helper.hpp>
#include "utils/SGDB.hpp"
#include "utils/config.hpp"
#include "utils/image_helper.hpp"

std::vector<SGDBEntry> games;
std::string tid;

SGDBCell::SGDBCell()
{
    this->inflateFromXMLRes("xml/cells/cell.xml");
}

SGDBCell* SGDBCell::create()
{
    return new SGDBCell();
}

SGDBData::SGDBData() {
}

int SGDBData::numberOfSections(brls::RecyclerFrame* recycler)
{
    return 1;
}

int SGDBData::numberOfRows(brls::RecyclerFrame* recycler, int section)
{
    return games.size();
}
    
std::string SGDBData::titleForHeader(brls::RecyclerFrame* recycler, int section) 
{
    return "";
}

brls::RecyclerCell* SGDBData::cellForRow(brls::RecyclerFrame* recycler, brls::IndexPath indexPath)
{
    SGDBEntry game = games[indexPath.row];
    auto* cell = (SGDBCell*)recycler->dequeueReusableCell("Cell");
    cell->label->setText(!game.year.empty() ? fmt::format("{} ({})", game.name, game.year) : game.name);

    cell->image->setImageFromRes("img/borealis_96.png");
    if(config::settings.displaySearchResultsIcons && !game.iconUrl.empty()) ImageHelper::with(cell->image, game.iconUrl);
    return cell;
}

void SGDBData::didSelectRowAt(brls::RecyclerFrame* recycler, brls::IndexPath indexPath)
{
    SGDBEntry game = games[indexPath.row];
    if (game.id != 0) recycler->present(new IconListView(game.id, tid, game.iconUrl));
}

SearchGamesView::SearchGamesView(std::string gameName, std::string titleId) {
    this->inflateFromXMLRes("xml/views/search_games.xml");
    // getAppletFrameItem()->title = fmt::format("Search results for \"{}\"", gameName);
    // getAppletFrameItem()->setIconFromRes("img/search.png");
    tid = titleId;

    games.clear();
    try {
        std::vector<SGDB::SearchResult> searchResults = SGDB::searchGames(gameName);

        for (auto& result : searchResults) {
            SGDBEntry entry = {};

            entry.id = result.game.id;
            entry.name = result.game.name;

            if (result.game.releaseDate) {
                std::tm* timeinfo = std::localtime(&result.game.releaseDate);
                entry.year = std::to_string(timeinfo->tm_year + 1900);
            }

            if (result.assets.size() > 0) {
                entry.iconUrl = result.assets[0].thumb;
            }

            if (utils::toUpperString(entry.name) == utils::toUpperString(gameName) && foundGame.id == 0 && config::settings.autoSelectIfPerfectMatch && !tid.empty()) {
                brls::Logger::info("Perfect match found for {} with {}", entry.name, entry.id);
                foundGame = entry;
            }

            games.push_back(entry);
        }

        if (games.size() == 0) {
            games.push_back({0, "No games found."});
        }
    } catch (const SGDB::ApiException& e) {
        games.push_back({0, fmt::format("Error: {}.", e.what())});
    }

    recycler->estimatedRowHeight = 70;
    recycler->registerCell("Cell", []() { return SGDBCell::create(); });
    recycler->setDataSource(new SGDBData());
}

void SearchGamesView::onChildFocusGained(View* directChild, View* focusedView) {
    if (foundGame.id != 0) {
        brls::Logger::verbose("Auto selecting game {} with {}", foundGame.name, foundGame.id);
        recycler->present(new IconListView(foundGame.id, tid, foundGame.iconUrl));
        foundGame = {}; // Reset to prevent auto select on next event call
    }
    brls::Box::onChildFocusGained(directChild, focusedView);
}
