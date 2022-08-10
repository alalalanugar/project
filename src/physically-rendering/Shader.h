#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

class Shader
{
public:
    Shader(const char* vertexPath, const char* fragmentPath);
    //~Shader();

    std::string vertexString;
    std::string fragmentString;
    const char* vertexSource;
    const char* fragmentSource;

    unsigned int ID;

    void use();

    void setBool(const char* paramNameString, bool param);
    void setInt(const char* paramNameString, int param);
    void setFloat(const char* paramNameString, float param);

    void setVec2(const char* paramNameString, glm::vec2 param);
    void setVec3(const std::string& name, float x, float y, float z);
    void setVec4(const char* paramNameString, glm::vec4 param);

    void setMat2(const char* paramNameString, glm::mat2& mat);
    void setMat3(const char* paramNameString, glm::mat3& mat);
    void setMat4(const char* paramNameString, glm::mat4& mat);

private:
    void checkCompileErrors(GLuint shader, std::string type);

};