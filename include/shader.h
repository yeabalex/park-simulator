#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <string>
#include <glm/glm.hpp>

class Shader {
public:
    GLuint ID;

    Shader(const char* vertexPath, const char* fragmentPath);
    void use();

    // Utility uniform functions
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void setMat4(const std::string &name, const glm::mat4 &mat) const;
    void setVec3(const std::string &name, const glm::vec3 &vec) const;
    void setVec4(const std::string &name, const glm::vec4 &vec) const;
    void setVec2(const std::string &name, const glm::vec2 &vec) const;

private:
    void checkCompileErrors(GLuint shader, std::string type);
};

#endif
