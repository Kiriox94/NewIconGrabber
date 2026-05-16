#pragma once
#include <fstream>
#include <vector>

namespace iconsFilesHelper {
    void overwriteIcon(std::string titleId, std::string imagePath = "", std::vector<unsigned char> imageBuffer = {});
    inline std::string getIconDir(std::string tid) {
        return "sdmc:/atmosphere/contents/" + tid + "/";
    }

    struct IconScale {
        std::string name;
        std::string fileName;
        int width;
    };

    class OverwriteIconException : public std::exception {
        public:
            explicit OverwriteIconException(const std::string& message) : msg_(message) {}
            const char* what() const noexcept override {
                return msg_.c_str();
            }
        private:
            std::string msg_;
    };
}