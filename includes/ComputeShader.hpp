#pragma once
#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

std::string reformErrorString(const std::string& fileName, int lineID, const std::string& thisString);

class ComputeShader {
public:
    unsigned int ID;

    template <typename... Paths>
    ComputeShader(const char* computePath, Paths... definitionPaths) {
        std::string computeCode = loadShaderCode(computePath);
        (appendDefinitionFile(computeCode, definitionPaths), ...);
        compileShader(computeCode.c_str());
    }

    void use() const { 
        glUseProgram(ID); 
    }

    void setBool(const std::string &name, bool value) const {         
        glUniform1i(glGetUniformLocation(ID, name.c_str()), static_cast<int>(value)); 
    }

    void setInt(const std::string &name, int value) const { 
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
    }

    void setFloat(const std::string &name, float value) const { 
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
    }

    void setDouble(const std::string &name, double value) const { 
        glUniform1d(glGetUniformLocation(ID, name.c_str()), value); 
    }
    
    void setVec2(const std::string &name, float x, float y) const { 
        glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y); 
    }

    void setVec2(const std::string &name, double x, double y) const { 
        glUniform2d(glGetUniformLocation(ID, name.c_str()), x, y); 
    }

    template <typename T>
    void setVec2(const std::string &name, const std::vector<T>& c) const { 
        setVec2(name, c[0], c[1]);
    }

    void setVec3(const std::string &name, float x, float y, float z) const { 
        glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z); 
    }

    void setiVec3(const std::string &name, int x, int y, int z) const { 
        glUniform3i(glGetUniformLocation(ID, name.c_str()), x, y, z); 
    }

    void setVec4(const std::string &name, float x, float y, float z, float w) const { 
        glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w); 
    }

private:
    std::string loadShaderCode(const char* path) const {
        std::ifstream shaderFile;
        shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try {
            shaderFile.open(path);
            std::stringstream shaderStream;
            shaderStream << shaderFile.rdbuf();
            shaderFile.close();
            return shaderStream.str();
        } catch (const std::ifstream::failure& e) {
            throw std::runtime_error(reformErrorString(__FILE__, __LINE__, "SHADER::UNSUCCESSFULLY_READ: Failed to open file \"" + std::string(path) + "\" : " + e.what()));
        }
    }

    void appendDefinitionFile(std::string& computeCode, const char* path) const {
        computeCode += "\n" + loadShaderCode(path);
    }

    void compileShader(const char* shaderCode) {
        unsigned int computeShader = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(computeShader, 1, &shaderCode, nullptr);
        glCompileShader(computeShader);
        checkCompileErrors(computeShader, "COMPUTE");

        ID = glCreateProgram();
        glAttachShader(ID, computeShader);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");

        glDeleteShader(computeShader);
    }

    void checkCompileErrors(GLuint shader, const std::string& type) const {
        GLint success;
        GLchar infoLog[1024];
        if (type != "PROGRAM") {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
                throw std::runtime_error(reformErrorString(__FILE__, __LINE__, "SHADER::COMPILATION_ERROR of type: " + type + "\n" + infoLog));
            }
        } else {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
                throw std::runtime_error(reformErrorString(__FILE__, __LINE__, "SHADER::PROGRAM_LINKING_ERROR of type: " + type + "\n" + infoLog));
            }
        }
    }
};
