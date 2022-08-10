#include "Shader.h"

Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
    // 1. retrieve the vertex/fragment source code from filePath

    std::ifstream vertexFile;
    std::ifstream fragmentFile;

    std::stringstream vertexStream;
    std::stringstream fragmentStream;

    vertexFile.open(vertexPath);
    fragmentFile.open(fragmentPath);
    vertexFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fragmentFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        if (!vertexFile.is_open() || !fragmentFile.is_open()) {
            throw std::exception("open file error");
        }

        vertexStream << vertexFile.rdbuf();
        fragmentStream << fragmentFile.rdbuf();

        vertexString = vertexStream.str();
        fragmentString = fragmentStream.str();

        vertexSource = vertexString.c_str();
        fragmentSource = fragmentString.c_str();

        unsigned int vertex, fragment;

        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vertexSource, NULL);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");

        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fragmentSource, NULL);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");

        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");

        glDeleteShader(vertex);
        glDeleteShader(fragment);

        //vertexFile.close();
        //fragmentFile.close();

    }
    catch (const std::exception& ex)
    {
        printf(ex.what());
    }

}

//Shader::~Shader(){
//
//}

void Shader::use()
{
    glUseProgram(ID);
}

void Shader::setBool(const char* paramNameString, bool param)
{
    glUniform1i(glGetUniformLocation(ID, paramNameString), (int)param);
}

void Shader::setInt(const char* paramNameString, int param)
{
    glUniform1i(glGetUniformLocation(ID, paramNameString), param);
}

void Shader::setFloat(const char* paramNameString, float param)
{
    glUniform1f(glGetUniformLocation(ID, paramNameString), param);
}

void Shader::setVec2(const char* paramNameString, glm::vec2 param)
{
    glUniform2f(glGetUniformLocation(ID, paramNameString), param.x, param.y);
}

void Shader::setVec3(const std::string& name, float x, float y, float z)
{
    glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
}

void Shader::setVec4(const char* paramNameString, glm::vec4 param)
{
    glUniform4f(glGetUniformLocation(ID, paramNameString), param.x, param.y, param.z, param.w);
}

void Shader::setMat2(const char* paramNameString, glm::mat2& mat)
{
    glUniformMatrix2fv(glGetUniformLocation(ID, paramNameString), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setMat3(const char* paramNameString, glm::mat3& mat)
{
    glUniformMatrix3fv(glGetUniformLocation(ID, paramNameString), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setMat4(const char* paramNameString, glm::mat4& mat)
{
    glUniformMatrix4fv(glGetUniformLocation(ID, paramNameString), 1, GL_FALSE, &mat[0][0]);
}

// utility function for checking shader compilation/linking errors.
// ------------------------------------------------------------------------
void Shader::checkCompileErrors(GLuint shader, std::string type)
{
    GLint success;
    GLchar infolog[512];
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 512, NULL, infolog);
            std::cout << "shader compile error: " << infolog << std::endl;
        }
    }
    else {
        glGetProgramiv(ID, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(ID, 512, NULL, infolog);
            std::cout << "program linking error:" << infolog << std::endl;
        }
    }
}