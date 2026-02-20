#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stb_image.h>

#include <iostream>
#include <filesystem>
#include <algorithm>
#include <vector>
#include <cctype>
#include <string>

namespace fs = std::filesystem;

constexpr std::string FRAMES_FILE_TYPE = ".png";

std::string trim(const std::string& s) {
    auto start = s.begin();
    while (start != s.end() && std::isspace(*start)) start++;

    auto end = s.end();
    do { end--; } while (std::distance(start, end) > 0 && std::isspace(*end));

    return std::string(start, end + 1);
}

size_t getFileNum(const fs::path& path) {
    return std::distance(fs::directory_iterator{path}, fs::directory_iterator{});
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: Photogrammetry <FrameDir>" << std::endl;
        return EXIT_FAILURE;
    }

    fs::path inputPath = trim(argv[1]);

    if (!fs::exists(inputPath)) {
        std::cerr << "Error: Directory does not exist: " << inputPath << std::endl;
        return EXIT_FAILURE;
    }
    else {
        std::cout << "Directory exists: " << inputPath.relative_path() << std::endl;
    }

    const fs::path framesDir = fs::absolute(inputPath);
    std::cout << "Frames directory: " << framesDir << std::endl;

    const size_t fileNum = getFileNum(framesDir);
    std::vector<std::string> sortedFrames{ fileNum };

    size_t frameCount = 0;
    for (const fs::path& frame : fs::directory_iterator(framesDir)) {
        const std::string filePath = frame.string();
        const std::string fileName = frame.filename().string();

        if (fileName.ends_with(FRAMES_FILE_TYPE)) {
            ++frameCount;

            const std::string stem = frame.stem().string();
            uint32_t frameNum = std::stoul(stem);

            sortedFrames.at(frameNum - 1) = filePath;
        }
        else {
            std::cout << filePath << " is not of the valid file type: " << FRAMES_FILE_TYPE << std::endl;
        }
    }

    if (frameCount != fileNum) {
        std::cerr << "Number of frames found does NOT match number of files in path. This may cause errors and undefined behaviours" << std::endl;
    }
    sortedFrames.resize(frameCount);

    int width, height, channelNum;
    stbi_uc* image = stbi_load(sortedFrames.at(0).c_str(), &width, &height, &channelNum, 3);
    if (!image) {
        std::cerr << "Failed to load image" << std::endl;
        stbi_image_free(image);
        return EXIT_FAILURE;
    }

    std::cout << "Image resolution: " << width << "*" << height << "\n" << channelNum << " channels" << std::endl;

    return EXIT_SUCCESS;
}
