#pragma once
#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>

class Shader {
public:
    unsigned int ID;

    Shader(const char* vertexPath, const char* fragmentPath) {
        std::string vertexCode, fragmentCode;
        try {
            vertexCode = readFile(vertexPath);
            fragmentCode = readFile(fragmentPath);
        } catch (const std::ifstream::failure& e) {
            throw std::runtime_error("ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " + std::string(e.what()));
        }

        compileAndLinkShaders(vertexCode.c_str(), fragmentCode.c_str());
    }

    void use() const {
        glUseProgram(ID);
    }

    void setBool(const std::string& name, bool value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), static_cast<int>(value));
    }

    void setInt(const std::string& name, int value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }

    void setFloat(const std::string& name, float value) const {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }

    void setDouble(const std::string& name, double value) const {
        glUniform1d(glGetUniformLocation(ID, name.c_str()), value);
    }

    void setVec2(const std::string& name, float x, float y) const {
        glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
    }

    void setVec3(const std::string& name, float x, float y, float z) const {
        glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
    }

    void setiVec3(const std::string& name, int x, int y, int z) const {
        glUniform3i(glGetUniformLocation(ID, name.c_str()), x, y, z);
    }

    void setVec4(const std::string& name, float x, float y, float z, float w) const {
        glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
    }

private:
    std::string readFile(const char* filePath) const {
        std::ifstream fileStream(filePath);
        if (!fileStream.is_open()) {
            throw std::runtime_error("ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " + std::string(filePath));
        }
        std::stringstream buffer;
        buffer << fileStream.rdbuf();
        return buffer.str();
    }

    void compileAndLinkShaders(const char* vertexCode, const char* fragmentCode) {
        unsigned int vertex = compileShader(vertexCode, GL_VERTEX_SHADER, "VERTEX");
        unsigned int fragment = compileShader(fragmentCode, GL_FRAGMENT_SHADER, "FRAGMENT");

        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");

        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    unsigned int compileShader(const char* shaderCode, GLenum shaderType, const std::string& type) const {
        unsigned int shader = glCreateShader(shaderType);
        glShaderSource(shader, 1, &shaderCode, nullptr);
        glCompileShader(shader);
        checkCompileErrors(shader, type);
        return shader;
    }

    void checkCompileErrors(GLuint shader, const std::string& type) const {
        GLint success;
        GLchar infoLog[1024];

        if (type != "PROGRAM") {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
                throw std::runtime_error("ERROR::SHADER_COMPILATION_ERROR of type: " + type + "\n" + infoLog);
            }
        } else {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
                throw std::runtime_error("ERROR::PROGRAM_LINKING_ERROR of type: " + type + "\n" + infoLog);
            }
        }
    }
};
