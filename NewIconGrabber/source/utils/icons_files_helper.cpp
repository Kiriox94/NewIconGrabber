#include "utils/icons_files_helper.hpp"
#include "utils/utils.hpp"

#include <borealis/core/logger.hpp>
#include <filesystem>

#include <stb_image.h>
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "utils/stb_image_resize2.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "utils/stb_image_write.h"
#define STB_IMAGE_IMPLEMENTATION

namespace iconsFilesHelper {
    void writeToVector(void* context, void* data, int size) {
        auto* buffer = static_cast<std::vector<unsigned char>*>(context);
        unsigned char* bytes = static_cast<unsigned char*>(data);
        buffer->insert(buffer->end(), bytes, bytes + size);
    }
    
    bool writeJpegUnderSize(const std::string& outPath, int width, int height, int channels, std::vector<unsigned char> img, std::size_t maxSize) {
        int quality = 100;
        std::vector<unsigned char> buffer;

        while (quality >= 45) {
            buffer.clear();
            stbi_write_jpg_to_func(writeToVector, &buffer, width, height, channels, img.data(), quality);

            if (buffer.size() < maxSize) {
                std::ofstream out(outPath, std::ios::binary);
                brls::Logger::info("Writing icon with quality {}", quality);
                out.write(reinterpret_cast<char*>(buffer.data()), buffer.size());
                return true;
            }
            quality -= 2;
        }
        return false;
    }

    std::vector<unsigned char> resizeImage(const unsigned char* img, int inputWidth, int inputHeight, int outputWidth, int channels) {

        std::vector<unsigned char> output(outputWidth * outputWidth * channels);

        if (!stbir_resize_uint8_srgb(img, inputWidth, inputHeight, 0, 
                                    output.data(), outputWidth, outputWidth, outputWidth * channels, 
                                    (stbir_pixel_layout)channels)) {
            return {}; // Return an empty vector on failure
        }
        
        return output; 
    }

    std::size_t getIconMaxSize(int scale) {
        if (scale == 256) {
            int firmwareMajor = utils::getFirmwareMajor();
            if (firmwareMajor >= 19 || firmwareMajor == 0) { // Set max size for FW 19 and above, for unknown firmware we set it to the same to be safe
                return 102400; // Max size in B for big icons on newer firmware
            } else {
                return 131072; // Max size in B for big icons on older firmware
            }
        } else {
            return 65536; // Max size in B for small icons
        }
    }

    std::vector<IconScale> getIconScales() {
        int firmwareMajor = utils::getFirmwareMajor();
        std::vector<IconScale> iconScales = {
            {"Big Icon", "icon.jpg", 256}
        };

        if (firmwareMajor >= 20) { // Small icons are only used on FW 20 and above
            iconScales.push_back({"Small Icon", "icon174.jpg", 174});
        }
        return iconScales;
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
    void overwriteIcon(std::string titleId, std::string imagePath, std::vector<unsigned char> imageBuffer)
    {
        int width, height, channels;
        unsigned char* img;

        if (!imageBuffer.empty()) {
            img = stbi_load_from_memory(imageBuffer.data(), imageBuffer.size(), &width, &height, &channels, 0);
        }else if (!imagePath.empty()) {
            img = stbi_load(imagePath.c_str(), &width, &height, &channels, 0);
        } else {
            brls::Logger::error("Both imageBuffer and imagePath are empty, please provide one.");
            throw std::exception();
        }

        if (img == NULL)
        {
            throw OverwriteIconException("Image could not be loaded");
        }
        else
        {
            std::string outDir = getIconDir(titleId);
            if (!std::filesystem::exists(outDir) && !std::filesystem::create_directories(outDir)) {
                throw OverwriteIconException(fmt::format("Could not create directory: {}", outDir));
            }

            std::string errorMessage;
            for (const auto& iconScale : getIconScales()) {
                auto iconMaxSize = getIconMaxSize(iconScale.width);
                std::vector<unsigned char> data = resizeImage(img, width, height, iconScale.width, channels);

                if (data.empty()) {
                   errorMessage = fmt::format("Could not resize image to {}", iconScale.width);
                   break;
                }

                std::string outPath = outDir + iconScale.fileName;
                if (!writeJpegUnderSize(outPath, iconScale.width, iconScale.width, channels, data, iconMaxSize))
                {
                    errorMessage = "The selected image is too big";
                    break;
                }
            }

            if (!errorMessage.empty()) {
                throw OverwriteIconException(errorMessage);
            }
            
            stbi_image_free(img);
        }
    }
}
