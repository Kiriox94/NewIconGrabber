#include "views/search_games.hpp"
#include "views/icon_list.hpp"
#include <ctime>
#include <borealis/core/cache_helper.hpp>
#include "utils/SGDB.hpp"
#include "utils/config.hpp"
#include <curl/curl.h>
#include "utils/threads_manager.hpp"

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
    SGDBCell* cell = (SGDBCell*)recycler->dequeueReusableCell("Cell");
    cell->label->setText(!game.year.empty() ? fmt::format("{} ({})", game.name, game.year) : game.name);

    if(config::settings.displaySearchResultsIcons && !game.iconUrl.empty()) {
        cell->image->setImageFromRes("img/borealis_96.png");
        ThreadsManager::getImagesPool().detach_task([cell, game]() {
            try {
                CURL* curl;
                std::vector<char> imageBuffer;
                CURLcode res;
                curl = curl_easy_init();
                if (curl)
                {
                    curl_easy_setopt(curl, CURLOPT_URL, game.iconUrl.c_str());
                    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, utils::write_to_memory);
                    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &imageBuffer);
                    res = curl_easy_perform(curl);
                    curl_easy_cleanup(curl);

                    brls::sync([cell, game, imageBuffer] {
                        cell->image->setImageFromMem(reinterpret_cast<const unsigned char*>(imageBuffer.data()), imageBuffer.size());
                        brls::TextureCache::instance().addCache(game.iconUrl, cell->image->getTexture());
                    });
                }
            } catch (const std::exception& ex) {
                cell->image->setImageFromRes("img/error.png");
                brls::Logger::error("request image {} {}", game.iconUrl, ex.what());
            }
        });
    }else cell->image->setImageFromRes("img/borealis_96.png");
    return cell;
}

void SGDBData::didSelectRowAt(brls::RecyclerFrame* recycler, brls::IndexPath indexPath)
{
   if (games[indexPath.row].id != 0) recycler->present(new IconListView(games[indexPath.row].id, tid));
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

            if (utils::toUpperString(entry.name) == utils::toUpperString(gameName) && foundGameId == 0 && config::settings.autoSelectIfPerfectMatch && !tid.empty()) {
                brls::Logger::info("Perfect match found for {} with {}", entry.name, entry.id);
                foundGameId = entry.id;
            }

            if (result.game.releaseDate) {
                std::tm* timeinfo = std::localtime(&result.game.releaseDate);
                entry.year = std::to_string(timeinfo->tm_year + 1900);
            }

            if (result.assets.size() > 0) {
                entry.iconUrl = result.assets[0].thumb;
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
    brls::Box::onShowAnimationEnd();
    if (foundGameId != 0) {
        recycler->present(new IconListView(foundGameId, tid));
        foundGameId = 0; // Reset to prevent auto select on next event call
    }
}
