#pragma once

#include <string>
#include <glm/glm.hpp>

uint64_t GetMaxThreadsPerDispatch(int localSizeX, int localSizeY, int localSizeZ);

class Shader {
public:
    unsigned int ID = 0;

private:
    void checkCompileErrors(unsigned int shader, std::string type);

public:
    Shader() = default;
    Shader(std::string vertexSrc, std::string fragmentSrc, std::string geometrySrc = "", bool isFromFile = true);

    void use() const;
public:
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setUint(const std::string& name, unsigned int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec2(const std::string& name, const glm::vec2& value) const;
    void setVec2(const std::string& name, float x, float y) const;
    void setiVec2(const std::string& name, const glm::ivec2& value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setiVec3(const std::string& name, const glm::ivec3& value) const;
    void setVec3(const std::string& name, float x, float y, float z) const;
    void setVec4(const std::string& name, const glm::vec4& value) const;
    void setVec4(const std::string& name, float x, float y, float z, float w) const;
    void setMat2(const std::string& name, const glm::mat2& mat) const;
    void setMat3(const std::string& name, const glm::mat3& mat) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;
};
