#pragma once
#include <vector>
#include <string>

#ifdef __SWITCH__
#include <switch.h>
#endif

extern const std::string HomeDir;

namespace utils {
    std::string formatStringsArray(std::vector<std::string> input, std::string separator);

    size_t write_to_string(void* ptr, size_t size, size_t nmemb, std::string stream);
    size_t write_to_file(void* ptr, size_t size, size_t nmemb, FILE* stream);
    size_t write_to_memory(void *contents, size_t size, size_t nmemb, void *userp);

    std::string getFileExtension(std::string const& path);
    std::string clearSpecialCharacters(std::string input);

    std::string capitalizeWords(std::string input);
    std::string toUpperString(const std::string str);

    int getFirmwareMajor();
    std::string getRootDirectory();
}