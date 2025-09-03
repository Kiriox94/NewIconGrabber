#include "views/local_icons.hpp"
#include "views/game_list.hpp"
#include "utils/app_metadata_helper.hpp"

FileCell::FileCell()
{
    this->inflateFromXMLRes("xml/cells/cell.xml");
}

FileCell* FileCell::create()
{
    return new FileCell();
}

FSData::FSData(LocalIconsView* view) {
    view->setTabActionAvailable(brls::ControllerButton::BUTTON_X, view->currentDir != INITIAL_DIRECTORY);
    fileEntries.clear();
    directoryEntries.clear();

    for (const auto& entry : fs::directory_iterator(view->currentDir)) {
        std::vector<std::string> validExtensions = { ".jpg", ".jpeg", ".png" };
        if (entry.is_regular_file() && std::find(validExtensions.begin(), validExtensions.end(), entry.path().extension()) != validExtensions.end()) fileEntries.push_back(entry);
        if (entry.is_directory()) {
            for (auto& e : std::filesystem::directory_iterator(entry)) // Ignore empty directories
            {
                if (e.is_regular_file() && std::find(validExtensions.begin(), validExtensions.end(), e.path().extension()) == validExtensions.end()) continue;
                directoryEntries.push_back(entry);
                break;
            }
        }
    }

    hasDirectories = directoryEntries.size() > 0;

    if (fileEntries.size() > 0) std::sort(fileEntries.begin(), fileEntries.end(), [](const fs::directory_entry& a, const fs::directory_entry& b) {
        return a.path().filename().string() < b.path().filename().string();
    });

    if (hasDirectories) std::sort(directoryEntries.begin(), directoryEntries.end(), [](const fs::directory_entry& a, const fs::directory_entry& b) {
        return a.path().filename().string() < b.path().filename().string();
    });
}

int FSData::numberOfSections(brls::RecyclerFrame* recycler)
{
    return hasDirectories && fileEntries.size() > 0 ? 2 : 1;
}

int FSData::numberOfRows(brls::RecyclerFrame* recycler, int section)
{
    if (hasDirectories && section == 0) {
        return directoryEntries.size();
    } else {
        return fileEntries.size();
    }
}
    
std::string FSData::titleForHeader(brls::RecyclerFrame* recycler, int section) 
{
    if (hasDirectories && section == 0) {
        return "Directories";
    } else {
        return "Files";
    }
}

brls::RecyclerCell* FSData::cellForRow(brls::RecyclerFrame* recycler, brls::IndexPath indexPath)
{
    brls::Logger::info("cellForRow");
    FileCell* item = (FileCell*)recycler->dequeueReusableCell("Cell");
    if (hasDirectories && indexPath.section == 0) {
        item->title->setText(directoryEntries[indexPath.row].path().filename().string());
        item->image->setImageFromSVGRes("img/folder.svg");
    } else {
        item->title->setText(fileEntries[indexPath.row].path().filename().string());
        item->image->setImageFromFile(fileEntries[indexPath.row].path().string());
    }
    return item;
}

void FSData::didSelectRowAt(brls::RecyclerFrame* recycler, brls::IndexPath indexPath)
{
    if (hasDirectories && indexPath.section == 0) { // Directory selected
        AutoTabFrame::focus2Sidebar(recycler->getParent());
        auto* view = dynamic_cast<LocalIconsView*>(recycler->getParent());
        view->currentDir = directoryEntries[indexPath.row].path().string();
        recycler->setDataSource(new FSData(view));
    } else { // File selected
        std::string assetPath = fileEntries[indexPath.row].path().string();
        auto callback = [assetPath](std::string tid) {
            if (!tid.empty()) {
                std::string outPath = utils::getIconPath(tid);
                try
                {
                    utils::overwriteIcon(outPath, assetPath);
                    brls::Application::notify("Icon applied");
                }
                catch(const std::exception& e)
                {
                    brls::Application::notify("Error: Icon not applied");
                }
            }
        };

        auto foundTID = utils::extractTitleIDFromString(fileEntries[indexPath.row].path().filename().string());
        if (foundTID) {
            std::string gameTitle = appMetadataHelper::getMetadataFromTitleId(*foundTID)->name;
            // brls::Logger::info("Found title ID {} in filename, corresponding to game: {}", utils::formatApplicationId(*foundTID), gameTitle ? *gameTitle : "Unknown");
            auto box   = new brls::Box();
            auto img   = new brls::Image();
            auto label = new brls::Label();
            label->setText(fmt::format("Apply this icon to {}?", gameTitle));
            label->setHorizontalAlign(brls::HorizontalAlign::CENTER);
            label->setMargins(20, 0, 10, 0);
            img->setMaxHeight(400);
            img->setImageFromFile(assetPath);
            box->addView(img);
            box->addView(label);
            box->setAxis(brls::Axis::COLUMN);
            box->setAlignItems(brls::AlignItems::CENTER);
            box->setMargins(20, 20, 20, 20);
            auto dialog = new brls::Dialog(box);
            dialog->addButton("Yes", [foundTID, callback]() {
                callback(utils::formatApplicationId(*foundTID));
            });
            dialog->addButton("No", []() {});
            dialog->addButton("To another game", [this, recycler, callback]() {
                auto* frame = new brls::AppletFrame(static_cast<brls::Box*>(new GameListView(callback)));
                brls::Application::pushActivity(new brls::Activity(frame));
            });
            dialog->open();
            return;
        }

        auto* frame = new brls::AppletFrame(static_cast<brls::Box*>(new GameListView(callback)));
        brls::Application::pushActivity(new brls::Activity(frame));
    }
}

LocalIconsView::LocalIconsView() {
    this->inflateFromXMLRes("xml/views/search_games.xml");

    recycler->estimatedRowHeight = 70;
    recycler->registerCell("Cell", []() { return FileCell::create(); });
    recycler->setDataSource(new FSData(this));
}

void LocalIconsView::onCreate() {
    this->registerTabAction("Previous directory", brls::ControllerButton::BUTTON_X, [this](brls::View* view) {
        if (this->currentDir != INITIAL_DIRECTORY) {
            AutoTabFrame::focus2Sidebar(recycler->getParent());
            this->currentDir = fs::path(currentDir).parent_path().string();
            recycler->setDataSource(new FSData(this));
            return true;
        }
        return false;
    });
    this->setTabActionAvailable(brls::ControllerButton::BUTTON_X, this->currentDir != INITIAL_DIRECTORY);
}

brls::View* LocalIconsView::create() {
    return new LocalIconsView();
}