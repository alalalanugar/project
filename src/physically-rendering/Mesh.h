#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

using namespace std;

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
};

struct Texture {
    unsigned int id;
    string type;
    string path;
};

class Mesh {
public:
    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures);
    
    //~Mesh();
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;

    void Draw(Shader* shader);

private:

    unsigned int VAO, VBO, EBO;
    void setupMesh();
};