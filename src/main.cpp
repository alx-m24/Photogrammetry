#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stb_image.h>

#include <future>
#include <iostream>
#include <filesystem>
#include <execution>
#include <algorithm>
#include <vector>
#include <cctype>
#include <string>

#include "Image.hpp"
#include "Shader.hpp"

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

using PhotogrammetryImage = Image<ImageFormat::RGB, unsigned char>;

std::vector<PhotogrammetryImage> getSortedFrames(const fs::path& framesDir, const uint32_t stride = 5) {
    const size_t fileNum = getFileNum(framesDir);

    std::vector<fs::path> files;
    for (auto& entry : fs::directory_iterator(framesDir)) {
        files.push_back(entry.path());
    }
    
    std::sort(files.begin(), files.end(), [](const fs::path& a, const fs::path& b) {
        return std::stoul(a.stem().string()) < std::stoul(b.stem().string());
    });

    std::vector<PhotogrammetryImage> images;
    images.reserve(fileNum);

    size_t frameCount = 0;
    bool discardedPrevious = false;
    for (const fs::path& frame : files) {
        ++frameCount;

        // Only keep every 'stride'-th frame
        if ((frameCount - 1) % stride != 0 || discardedPrevious) continue;

        const std::string extension = frame.extension().string();
        if (extension != FRAMES_FILE_TYPE) {
            discardedPrevious = true;
            std::cout << frame << " is not of the valid file type: " << FRAMES_FILE_TYPE << std::endl;
            continue;
        }

        const std::string filePath = frame.string();
        const uint32_t frameNum = std::stoul(frame.stem().string());

        std::cout << "Processing frame: " << frameNum << " | kept frame count: " << images.size() + 1 << std::endl;

        // push_back only the kept frames
        PhotogrammetryImage img;
        img.loadFromFile(filePath);
        images.push_back(std::move(img));
        discardedPrevious = false;
    }

    return images;
}

void frameBufferResize(GLFWwindow*, int w, int h) {
    glViewport(0, 0, w, h);
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

    const fs::path framesDir = fs::absolute(inputPath);
    std::cout << "Frames directory: " << framesDir << std::endl;

    auto futureImages = std::async(std::launch::async, getSortedFrames, framesDir, 1);
    std::vector<PhotogrammetryImage> images;

    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwSwapInterval(1);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Photogrammetry", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to load glad" << std::endl;
        return EXIT_FAILURE;
    }

    frameBufferResize(window, 1280, 720);
    glfwSetFramebufferSizeCallback(window, frameBufferResize);

    glEnable(GL_FRAMEBUFFER_SRGB); // converting to srgb at output

    Shader fullScreen("Shaders/FullScreen/FullScreen.vert", "Shaders/FullScreen/FullScreen.frag");

    GLuint vao;
    glCreateVertexArrays(1, &vao);
    glBindVertexArray(vao);

    while (futureImages.wait_for(std::chrono::milliseconds(10)) != std::future_status::ready) {
        glfwPollEvents();

        if (glfwWindowShouldClose(window)) {
            return EXIT_SUCCESS;
        }

        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
    }

    images = futureImages.get();

    if (images.empty()) {
        std::cerr << "ERROR: No images loaded!" << std::endl;
        return EXIT_FAILURE;
    }
    
    GLuint imageTex;
    glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &imageTex);

    GLenum internalFormat;
    GLenum format = GL_RGB;
    GLenum type;
    if constexpr (std::is_same_v<const unsigned char*, typeof(images.front().data())>) {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        internalFormat = GL_SRGB8;
        type = GL_UNSIGNED_BYTE;
    }
    else {
        internalFormat = GL_RGB32F;
        type = GL_FLOAT;
    }
    glTextureStorage3D(imageTex, 1, internalFormat, images.front().getResolution().x, images.front().getResolution().y, images.size());
    for (size_t offset = 0; offset < images.size(); ++offset) {
        auto& img = images[offset];
        glTextureSubImage3D(imageTex, 0, 0, 0, offset, img.getResolution().x, img.getResolution().y, 1, format, type, img.data());    
    }

    glTextureParameteri(imageTex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTextureParameteri(imageTex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTextureParameteri(imageTex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(imageTex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    const uint32_t targetFPS = 24;
    float lastFrameTime = 0.0f;
    float frameNum = 0;

    while (!glfwWindowShouldClose(window)) {
        float time = glfwGetTime();
        float dt = lastFrameTime - time;
        lastFrameTime = time;

        frameNum += dt * targetFPS;

        glfwPollEvents();

        glClear(GL_COLOR_BUFFER_BIT);
        
        fullScreen.use();
        fullScreen.setInt("image", 0);
        fullScreen.setInt("frameNum", static_cast<int>(frameNum) % images.size());
        glBindTextureUnit(0, imageTex);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
    }

    glDeleteTextures(1, &imageTex);
    glDeleteVertexArrays(1, &vao);

    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
