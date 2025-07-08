#include "utils/utils.hpp"
#include <regex>
#include <optional>

#include "stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_IMPLEMENTATION

const std::string HomeDir = "sdmc:/switch/NewIconGrabber/";

namespace utils
{
    std::string formatApplicationId(u64 ApplicationId)
    {
        std::stringstream strm;
        strm << std::uppercase << std::setfill('0') << std::setw(16) << std::hex << ApplicationId;
        return strm.str();
    }

    size_t write_to_string(void* ptr, size_t size, size_t nmemb, std::string stream)
    {
        size_t realsize = size * nmemb;
        std::string temp(static_cast<const char*>(ptr), realsize);
        stream.append(temp);
        return realsize;
    }

    size_t write_to_file(void* ptr, size_t size, size_t nmemb, FILE* stream)
    {
        size_t written;
        written = fwrite(ptr, size, nmemb, stream);
        return written;
    }

    size_t write_to_memory(void *contents, size_t size, size_t nmemb, void *userp) {
        size_t totalSize = size * nmemb;
        std::vector<char> *buffer = static_cast<std::vector<char> *>(userp);
        buffer->insert(buffer->end(), (char *)contents, (char *)contents + totalSize);
        return totalSize;
    }

    std::string getFileExtension(std::string const& path)
    {
        return clearSpecialCharacters(path.substr(path.find_last_of(".\\") + 1));
    }

    std::string getIconPath(std::string tid) {
        return fmt::format("sdmc:/atmosphere/contents/{}/icon.jpg", tid);
    }

    /**
     * @brief Overwrites the icon for a specific title ID with a new image.
     * 
     * This function replaces the existing icon associated with the given title ID
     * (tid) with the image located at the specified file path (imagePath).
     * 
     * @param tid The title ID as a string for which the icon will be replaced.
     * @param imagePath The file path to the new image that will be used as the icon.
     */
    void overwriteIcon(std::string outPath, std::string imagePath, std::vector<unsigned char> imageBuffer)
    {
        int width, height, channels;
        unsigned char* img;

        if (!imageBuffer.empty()) {
            img = stbi_load_from_memory(imageBuffer.data(), imageBuffer.size(), &width, &height, &channels, 0);
        }else if (!imagePath.empty()) {
            img = stbi_load(imagePath.c_str(), &width, &height, &channels, 0);
        } else {
            brls::Logger::error("[overwriteIcon] Both imageBuffer and imagePath are empty, please provide one.");
            throw std::exception();
        }

        if (img == NULL)
        {
            brls::Logger::error("[overwriteIcon] Image could not be loaded");
        }
        else
        {
            unsigned char* data = NULL;
            data = (unsigned char*)malloc(256 * 256 * channels * sizeof(unsigned char));

            if (!stbir_resize_uint8_srgb(img, width, height, 0, data, 256, 256, 256 * channels, (stbir_pixel_layout)channels)){
                brls::Logger::error("[overwriteIcon] Could not resize image");
                throw std::exception();
            }

            std::string outDir = fs::path(outPath).parent_path().string();
            if (!fs::exists(outDir) && !std::filesystem::create_directories(outDir)) {
                brls::Logger::error("[overwriteIcon] Could not create directory: {}", outDir);
                throw std::exception();
            }

            if (!stbi_write_jpg(outPath.c_str(), 256, 256, channels, data, 100))
            {
                brls::Logger::error("[overwriteIcon] Could not write image");
                throw std::exception();
            }
            stbi_image_free(img);
            free(data);
        }
    }

    std::string formatStringsArray(std::vector<std::string> input, std::string separator) {
        return std::accumulate(input.begin(), input.end(), std::string(),
        [separator](const std::string& a, const std::string& b) {
            return a.empty() ? b : a + separator + b;
        });
    }

    std::string clearSpecialCharacters(std::string input) {
        input.erase(std::remove_if(input.begin(), input.end(),
        [](unsigned char c) {
            return !std::isalnum(c); // supprime tout ce qui n'est pas lettre ou chiffre
        }), input.end());

        return input;
    }

    std::optional<u64> extractTitleIDFromString(const std::string& input) {
        std::regex titleIdRegex(R"(0100[a-fA-F0-9]{12})");
        std::smatch match;
        u64 tid;

        if (std::regex_search(input, match, titleIdRegex)) {
            std::istringstream buffer(match.str());
            buffer >> std::hex >> tid;
            return tid; // Return first match
        }

        return std::nullopt; // No Title ID found
    }

    std::string capitalizeWords(std::string input) {
        bool newWord = true; // Indique si nous sommes au début d'un mot
        
        for (char &ch : input) {
            if (std::isspace(ch)) {
                newWord = true; // Si c'est un espace, on est prêt à commencer un nouveau mot
            } else if (newWord && std::isalpha(ch)) {
                ch = std::toupper(ch); // Mettre la première lettre du mot en majuscule
                newWord = false; // Nous avons déjà capitalisé le premier caractère du mot
            } else {
                newWord = false; // Si on n'est pas au début d'un mot, on continue
            }
        }
        return input;
    }

    std::string toUpperString(std::string str) {
        std::transform(str.begin(), str.end(), str.begin(), ::toupper);
        return str;
    }

    void setHeaderVisibility(bool visible) {
        brls::sync([visible]() {
            brls::Logger::debug("Setting activities header visibility to {}", visible ? "VISIBLE" : "GONE");
            auto stack = brls::Application::getActivitiesStack();
            for (auto& activity : stack) {
                auto* frame = dynamic_cast<brls::AppletFrame*>(activity->getContentView());
                if (!frame) continue;
                frame->setHeaderVisibility(visible ? brls::Visibility::VISIBLE : brls::Visibility::GONE);
            }
        });
    }
} // namespace utils