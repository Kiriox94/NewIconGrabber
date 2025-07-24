#pragma once
#include <borealis.hpp>
#include <switch.h>
#include <filesystem>
#include <nlohmann/json.hpp>

extern const std::string HomeDir;

namespace fs = std::filesystem;

namespace utils {
    std::string formatApplicationId(u64 ApplicationId);
    std::string formatStringsArray(std::vector<std::string> input, std::string separator);

    size_t write_to_string(void* ptr, size_t size, size_t nmemb, std::string stream);
    size_t write_to_file(void* ptr, size_t size, size_t nmemb, FILE* stream);
    size_t write_to_memory(void *contents, size_t size, size_t nmemb, void *userp);

    std::string getFileExtension(std::string const& path);
    std::string getIconPath(std::string tid);
    std::string clearSpecialCharacters(std::string input);
    std::optional<u64> extractTitleIDFromString(const std::string& input);

    void overwriteIcon(std::string outPath, std::string imagePath = "", std::vector<unsigned char> imageBuffer= {});

    std::string capitalizeWords(std::string input);
    std::string toUpperString(const std::string str);

    void setHeaderVisibility(bool visible);
}