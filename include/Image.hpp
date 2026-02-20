#pragma once

#include <stb_image.h>
#include <glm/glm.hpp>

#include <filesystem>
#include <string>

#include <iostream>

enum class ImageFormat : int {
    GRAY_SCALE = 1,
    RGB = 3,
    RGBA = 4
};

enum class LoadResult {
    SUCCESS,
    FILE_NOT_FOUND,
    STB_FAIL
};

template<ImageFormat Format, typename ComponentType = unsigned char>
requires std::same_as<float, ComponentType> || std::same_as<unsigned char, ComponentType>
class Image {
    private:
        ComponentType* m_data = nullptr;
        uint32_t m_width, m_height;
        uint32_t m_channelNum;

    public:
        Image() = default;

        ~Image() {
            if (m_data) stbi_image_free(m_data);
        }

        Image(const Image&) = delete;
        Image& operator=(const Image&) = delete;

        Image(Image&& other) noexcept {
            m_data = other.m_data;
            m_width = other.m_width;
            m_height = other.m_height;
            m_channelNum = other.m_channelNum;
        
            other.m_data = nullptr;
        }
        
        Image& operator=(Image&& other) noexcept {
            if (this != &other) {
                if (m_data) {
                    stbi_image_free(m_data);
                }
        
                m_data = other.m_data;
                m_width = other.m_width;
                m_height = other.m_height;
                m_channelNum = other.m_channelNum;
        
                other.m_data = nullptr;
            }
            return *this;
        }

    public:
        LoadResult loadFromFile(const std::filesystem::path& absolutePath) {
            if (!std::filesystem::exists(absolutePath)) {
                return LoadResult::FILE_NOT_FOUND;
            }

            const std::string pathStr = absolutePath.string();

            int width, height, channelNum;

            stbi_set_flip_vertically_on_load(true);
            if constexpr (std::is_same_v<float, ComponentType>) {
                float* data = stbi_loadf(pathStr.c_str(), &width, &height, &channelNum, static_cast<int>(Format));
                if (!data) {
                    std::cerr << stbi_failure_reason() << std::endl;
                    return LoadResult::STB_FAIL;
                }
            
                m_data = data;
            }
            else {
                stbi_uc* data = stbi_load(pathStr.c_str(), &width, &height, &channelNum, static_cast<int>(Format));
                if (!data) {
                    std::cerr << stbi_failure_reason() << std::endl;
                    return LoadResult::STB_FAIL;
                }
            
                m_data = data;
            }

            m_width = width;
            m_height = height;
            m_channelNum = static_cast<uint32_t>(Format);

            return LoadResult::SUCCESS;
        }

    public:
        bool isGrayScale() const { return m_channelNum == 1; }
        bool isRGB() const { return m_channelNum == 3; }
        bool isRGBA() const { return m_channelNum == 4; }

    public:
        using Resolution = glm::uvec2;
        Resolution getResolution() const { return { m_width, m_height }; }

    public:
        bool isLoaded() const { return m_data; }
        
    public:
        using Pixel = glm::vec<static_cast<uint32_t>(Format), float>; 
        Pixel getPixel(uint32_t x, uint32_t y) const { 
            if (!isLoaded()) return {};
            if (x >= m_width || y >= m_height) return {};

            size_t index = (x + y * m_width) * m_channelNum;
            Pixel pixel;

            for (uint32_t i = 0; i < m_channelNum; ++i) {
                if constexpr (std::is_same_v<unsigned char, ComponentType>) {
                    pixel[i] = static_cast<float>(m_data[index + i]) / 255.0f;
                }
                else {
                    pixel[i] = m_data[index + i];
                }
            }

            return pixel;
        }

        Pixel getPixel(glm::uvec2 index) const {
            return getPixel(index.x, index.y);
        }

    public:
        void setPixel(Pixel val, uint32_t x, uint32_t y) {
            if (!isLoaded()) return;
            if (x >= m_width || y >= m_height) return;

            size_t index = (x + y * m_width) * m_channelNum;
            Pixel pixel;

            for (uint32_t i = 0; i < m_channelNum; ++i) {
                if constexpr (std::is_same_v<unsigned char, ComponentType>) {
                    float clamped = std::clamp(val[i], 0.0f, 1.0f);
                    unsigned char byteVal = static_cast<unsigned char>(clamped * 255.0f);
                    m_data[index + i] = byteVal;
                }
                else {
                    m_data[index + i] = val[i];
                }
            }
        }

        void setPixel(Pixel val, glm::uvec2 index) {
            setPixel(val, index.x, index.y);
        }

    public:
        const ComponentType* data() const {
            return m_data;
        }
};
