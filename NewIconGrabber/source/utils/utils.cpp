#include "utils/utils.hpp"
#include <regex>
#include <optional>
#include <iostream>
#include <iomanip>
#include <numeric>

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

    std::string formatStringsArray(std::vector<std::string> input, std::string separator) {
        return std::accumulate(input.begin(), input.end(), std::string(),
        [separator](const std::string& a, const std::string& b) {
            return a.empty() ? b : a + separator + b;
        });
    }

    std::string clearSpecialCharacters(std::string input) {
        input.erase(std::remove_if(input.begin(), input.end(),
        [](unsigned char c) {
            return !std::isalnum(c); // remove non-alphanumeric characters
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
        bool newWord = true;
        
        for (char &ch : input) {
            if (std::isspace(ch)) {
                newWord = true; // If it's a space then the next character will be the start of a new word
            } else if (newWord && std::isalpha(ch)) {
                ch = std::toupper(ch); // Put the letter to uppercase
                newWord = false;
            } else {
                newWord = false;
            }
        }
        return input;
    }

    std::string toUpperString(std::string str) {
        std::transform(str.begin(), str.end(), str.begin(), ::toupper);
        return str;
    }

    int getFirmwareMajor() {
        static int cachedMajor = -1;

        if (cachedMajor == -1) {
            SetSysFirmwareVersion fw;
            int rc = setsysGetFirmwareVersion(&fw);
            
            if (R_SUCCEEDED(rc)) {
                cachedMajor = fw.major;
            } else {
                return 0; // Don't cache the result if the call fails, and return 0 as a safe default
            }
        }

        return cachedMajor;
    }
} // namespace utils
