#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "FileCatch.h"
#include "Shader.h"
#include "Camera.h"
#include "Model.h"
//#include "LightDirectional.h"

#include "stb_image.h"

void renderSphere();
void renderCube();
void renderQuad();

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 15.0f));

float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;

bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = 10 * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
        camera.ProcessKeyboard(AUTO, deltaTime);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse == true)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float deltaX, deltaY;
    deltaX = xpos - lastX;
    deltaY = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(deltaX, deltaY);
    //printf("%f \n", deltaX);
}

// Set Model
void setModel(unsigned int Albedo, unsigned int Ao, unsigned int Metallic, unsigned int Normal, unsigned int Roughness, Shader* myShader, Model* modelpart, glm::mat4 model)
{
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, Albedo);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, Ao);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, Metallic);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, Normal);
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, Roughness);

    myShader->setMat4("model", model);
    modelpart->Draw(myShader);
};

unsigned int loadImageToGPU(char const* filename)
{
    unsigned int TexBuffer;
    glGenTextures(1, &TexBuffer);

    int width, height, nrChannel;
    //stbi_set_flip_vertically_on_load(true);

    unsigned char* data = stbi_load(filename, &width, &height, &nrChannel, 0);
    if (data)
    {
        GLint internalFormat = GL_RGBA;
        GLenum format = GL_RGBA;
        if (nrChannel == 1)
        {
            internalFormat = GL_RED;
            format = GL_RED;
        }

        else if (nrChannel == 3)
        {
            internalFormat = GL_RGB;
            format = GL_RGB;
        }

        else if (nrChannel == 4)
        {
            internalFormat = GL_RGBA;
            format = GL_RGBA;
        }


        glBindTexture(GL_TEXTURE_2D, TexBuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "load image failed" << filename << std::endl;
        stbi_image_free(data);
    }

    return TexBuffer;
}

int main()
{
    // glfw init
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(1280, 720, "learnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "OPEN WINDOW FAILED" << std::endl;
        glfwTerminate();

        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);

    //Init GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "INIT GLAD FAILED" << std::endl;

        return -1;
    }

    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_FRONT);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // Shader
    Shader* myPbrShader = new Shader("mypbr.vert", "mypbr.frag");
    Shader* lightShader = new Shader("mypbr.vert", "light.frag");
    Shader* equirectangularToCubemapShader = new Shader("cubemap.vert", "equirectangular_to_cubemap.frag");
    Shader* prefilterShader = new Shader("cubemap.vert", "prefilter.frag");
    Shader* irradianceShader = new Shader("cubemap.vert", "irradiance.frag");
    Shader* brdfShader = new Shader("brdf.vert", "brdf.frag");
    Shader* backgroundShader = new Shader("background.vert", "background.frag");

    myPbrShader->use();
    myPbrShader->setInt("TexIrradiance", 0);
    myPbrShader->setInt("TexPrefilter", 1);
    myPbrShader->setInt("brdfLUT", 2);
    myPbrShader->setInt("albedo", 3);
    myPbrShader->setInt("ao", 4);
    myPbrShader->setInt("metallic", 5);
    myPbrShader->setInt("normal", 6);
    myPbrShader->setInt("roughness", 7);

    backgroundShader->use();
    backgroundShader->setInt("environmentMap", 0);

    const int sphereNum = 9;
    std::string inputPath = "resources/textures/pbr";
    std::string twoInputPath[] = { "/floor/", "/slipperystonework/", "/ornate-celtic-gold/", "/metal/", "/wornpaintedcement/", "/stonewall/", "/Titanium-Scuffed/", "/wrinkled-paper/", "/rustymetal/"};
    std::string allTextureName[] = { "albedo.png", "ao.png", "metallic.png", "normal.png", "roughness.png" };
    unsigned int sphereMap[sphereNum][5];

    for (int i = 0; i < sphereNum; i++)
    {
        std::string tempInputPath = inputPath + twoInputPath[i];
        for (int j = 0; j < 5; j++)
        {
            std::string lastInputPath = tempInputPath + allTextureName[j];
            sphereMap[i][j] = loadImageToGPU(FileCatch::getPath(lastInputPath).c_str());
        }
    }

    stbi_set_flip_vertically_on_load(false);

    //Init model pbr texture
    
    const int modelNum = 3;
    std::string firPath = "resources/textures/pbr";
    std::string secPath[] = { "/gray/", "/red/", "/yellow/"};
    std::string TextureName[] = { "albedo.png", "ao.png", "metallic.png", "normal.png", "roughness.png" };
    unsigned int modelMap[modelNum][5];

    for (int i = 0; i < modelNum; i++)
    {
        std::string InputPath = firPath + secPath[i];
        for (int j = 0; j < 5; j++)
        {
            std::string lastPath = InputPath + TextureName[j];
            modelMap[i][j] = loadImageToGPU(FileCatch::getPath(lastPath).c_str());
        }
    }

    //Init model
    Model* gray = new Model(FileCatch::getPath("resources/objects/mk/gray.ply"));
    Model* red = new Model(FileCatch::getPath("resources/objects/mk/red.ply"));
    Model* yellow = new Model(FileCatch::getPath("resources/objects/mk/allyellow.ply"));
    Model* modelMum[] = { gray,red,yellow };

    int Albedo = loadImageToGPU(FileCatch::getPath("resources/textures/pbr/helmet/albedo.jpg").c_str());
    int Ao = loadImageToGPU(FileCatch::getPath("resources/textures/pbr/helmet/ao.jpg").c_str());
    int Metallic = loadImageToGPU(FileCatch::getPath("resources/textures/pbr/helmet/metallic.png").c_str());
    int Normal = loadImageToGPU(FileCatch::getPath("resources/textures/pbr/helmet/normal.jpg").c_str());
    int Roughness = loadImageToGPU(FileCatch::getPath("resources/textures/pbr/helmet/roughness.png").c_str());
    Model* helmet = new Model(FileCatch::getPath("resources/objects/mk/helmet.fbx"));

    int lightAlbedo = loadImageToGPU(FileCatch::getPath("resources/textures/pbr/light/albedo.png").c_str());
    int lightAo = loadImageToGPU(FileCatch::getPath("resources/textures/pbr/light/ao.png").c_str());
    int lightMetallic = loadImageToGPU(FileCatch::getPath("resources/textures/pbr/light/metallic.png").c_str());
    int lightNormal = loadImageToGPU(FileCatch::getPath("resources/textures/pbr/light/normal.png").c_str());
    int lightRoughness = loadImageToGPU(FileCatch::getPath("resources/textures/pbr/light/roughness.png").c_str());
    Model* alllight = new Model(FileCatch::getPath("resources/objects/mk/alllight.ply"));

    //lights
    glm::vec3 lightPositions[] = {
        glm::vec3(-10.0f, 10.0f, 10.0f),
        glm::vec3(10.0f,  10.0f, 10.0f),
        glm::vec3(-10.0f, -10.0f, 10.0f),
        glm::vec3(10.0f, -10.0f, 10.0f),
        glm::vec3(-10.0f, 10.0f, -10.0f),
        glm::vec3(10.0f,  10.0f, -10.0f),
        glm::vec3(-10.0f, -10.0f, -10.0f),
        glm::vec3(10.0f, -10.0f, -10.0f)
    };
    glm::vec3 lightColors[] = {
        glm::vec3(500.0f, 500.0f, 500.0f),
        glm::vec3(0.0f, 0.0f, 500.0f),
        glm::vec3(0.0f, 500.0f, 0.0f),
        glm::vec3(500.0f, 0.0f, 0.0f),
        glm::vec3(500.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 500.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 500.0f),
        glm::vec3(500.0f, 500.0f, 500.0f)
    };

    int nrRows = 7;
    int nrColumns = 7;
    float spacing = 2.5;

    // set framebuffer to cubemap
    unsigned int captureFBO;
    unsigned int captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

    // load pbr
    stbi_set_flip_vertically_on_load(true);
    int width, height, nrChannels;

    float* data = stbi_loadf(FileCatch::getPath("resources/textures/hdr/venice_sunset_4k.hdr").c_str(), &width, &height, &nrChannels, 0); 
    //float* data = stbi_loadf(FileCatch::getPath("resources/textures/hdr/LA_Downtown_Afternoon_Fishing_3k.hdr").c_str(), &width, &height, &nrChannels, 0); 
    //float* data = stbi_loadf(FileCatch::getPath("resources/textures/hdr/Barce_Rooftop_C_3k.hdr").c_str(), &width, &height, &nrChannels, 0);
    //float* data = stbi_loadf(FileCatch::getPath("resources/textures/hdr/Newport_Loft_Ref.hdr").c_str(), &width, &height, &nrChannels, 0);

    unsigned int hdrTexture;
    if (data)
    {
        glGenTextures(1, &hdrTexture);
        glBindTexture(GL_TEXTURE_2D, hdrTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else{
        std::cout << "Failed to load HDR image." << std::endl;
    }

    // pbr set cubemap
    unsigned int envCubemap;
    glGenTextures(1, &envCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // pbr framebuffer camera
    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] =
    {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    //equirectangular map to Cubemap 
    equirectangularToCubemapShader->use();
    equirectangularToCubemapShader->setInt("equirectangularMap", 0);
    equirectangularToCubemapShader->setMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);

    glViewport(0, 0, 512, 512);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for (unsigned int i = 0; i < 6; ++i)
    {
        equirectangularToCubemapShader->setMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderCube();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // irradiance cubemap
    unsigned int TexIrradiance;
    glGenTextures(1, &TexIrradiance);
    glBindTexture(GL_TEXTURE_CUBE_MAP, TexIrradiance);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

    irradianceShader->use();
    irradianceShader->setInt("environmentMap", 0);
    irradianceShader->setMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

    glViewport(0, 0, 32, 32);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for (unsigned int i = 0; i < 6; ++i)
    {
        irradianceShader->setMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, TexIrradiance, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderCube();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // pbr prefileter
    unsigned int TexPrefilter;
    glGenTextures(1, &TexPrefilter);
    glBindTexture(GL_TEXTURE_CUBE_MAP, TexPrefilter);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // prefileter cubemap
    prefilterShader->use();
    prefilterShader->setInt("environmentMap", 0);
    prefilterShader->setMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    unsigned int maxMipLevels = 5;
    for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
    {
        // reisze framebuffer according to mip-level size.
        unsigned int mipWidth = 128 * std::pow(0.5, mip);
        unsigned int mipHeight = 128 * std::pow(0.5, mip);
        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
        glViewport(0, 0, mipWidth, mipHeight);

        float roughness = (float)mip / (float)(maxMipLevels - 1);
        prefilterShader->setFloat("roughness", roughness);
        for (unsigned int i = 0; i < 6; ++i)
        {
            prefilterShader->setMat4("view", captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, TexPrefilter, mip);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            renderCube();
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // add brdf
    unsigned int TexbrdfLUT;
    glGenTextures(1, &TexbrdfLUT);

    // pre-allocate enough memory for the LUT texture.
    glBindTexture(GL_TEXTURE_2D, TexbrdfLUT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, TexbrdfLUT, 0);

    glViewport(0, 0, 512, 512);
    brdfShader->use();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderQuad();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // projection
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)1980 / (float)1080, 0.1f, 100.0f);
    myPbrShader->use();
    myPbrShader->setMat4("projection", projection);
    backgroundShader->use();
    backgroundShader->setMat4("projection", projection);
    lightShader->use();
    lightShader->setMat4("projection", projection);

    int scrWidth, scrHeight;
    glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
    glViewport(0, 0, scrWidth, scrHeight);

    const float PI = 3.14159265359;
    float theta[9] = { 0, PI / 4.5f, PI * 2 / 4.5f, PI * 3 / 4.5f, PI * 4 / 4.5f, PI * 5 / 4.5f, PI * 6 / 4.5f, PI * 7 / 4.5f, PI * 8 / 4.5f }; // rotate angle
    const float circleR = 4.5f;

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        //Process Input
        processInput(window);

        //Clear Screen
        //glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        myPbrShader->use();
        glm::mat4 view = camera.GetViewMatrix();
        myPbrShader->setMat4("view", view);
        myPbrShader->setVec3("camPos", camera.Position.x, camera.Position.y, camera.Position.z);

        //Set Material -> Textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, TexIrradiance);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, TexPrefilter);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, TexbrdfLUT);

        //Set Model
        for (int i = 0; i < modelNum; i++)
        {
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, modelMap[i][0]);
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, modelMap[i][1]);
            glActiveTexture(GL_TEXTURE5);
            glBindTexture(GL_TEXTURE_2D, modelMap[i][2]);
            glActiveTexture(GL_TEXTURE6);
            glBindTexture(GL_TEXTURE_2D, modelMap[i][3]);
            glActiveTexture(GL_TEXTURE7);
            glBindTexture(GL_TEXTURE_2D, modelMap[i][4]);

            glm::mat4 model = glm::mat4(4.0f);
            model = glm::translate(model, glm::vec3(0, -2, 0));
            model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
            model = glm::rotate(model, -(float)glfwGetTime(), glm::vec3(0, 0, 1));
            myPbrShader->setMat4("model", model);
            setModel(modelMap[i][0], modelMap[i][1], modelMap[i][2], modelMap[i][3], modelMap[i][4], myPbrShader, modelMum[i], model);
        }

        glm:: mat4 model = glm::mat4(1.5f);
        model = glm::translate(model, glm::vec3(0, 0, 3));
        model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1, 0, 0));
        model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0, 1, 0));
        setModel(Albedo, Ao, Metallic, Normal, Roughness, myPbrShader, helmet, model);

        lightShader->use();
        lightShader->setVec3("objectColor", 0.4f, 1.0f, 1.0f);
        lightShader->setVec3("lightColor", 1.0f, 1.0f, 1.0f);
        view = camera.GetViewMatrix();
        lightShader->setMat4("view", view);
        lightShader->setVec3("camPos", camera.Position.x, camera.Position.y, camera.Position.z);
        model = glm::mat4(4.0f);
        model = glm::translate(model, glm::vec3(0, -2, 0));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
        model = glm::rotate(model, -(float)glfwGetTime(), glm::vec3(0, 0, 1));
        setModel(lightAlbedo, lightAo, lightMetallic, lightNormal, lightRoughness, lightShader, alllight, model);

        //Set Sphere
        //point light
        const int pointLightNum = 8;
        const float lightNum = 4.0f;
        float lightTheta[8] = { 0, PI / lightNum, PI * 2 / lightNum, PI * 3 / lightNum, PI * 4 / lightNum, PI * 5 / lightNum, PI * 6 / lightNum, PI * 7 / lightNum };

        for (int i = 0; i < pointLightNum; i++)
        {
            lightShader->setVec3("objectColor", lightColors[i].x, lightColors[i].y, lightColors[i].z);
            lightShader->setVec3("lightColor", 1.0f, 1.0f, 1.0f);

            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(lightPositions[i]));

            lightShader->setMat4("model", model);
            renderSphere();
        }
 
        //loop
        float radian = -glfwGetTime() * 0.4f;

        myPbrShader->use();

        for (int i = 0; i < sphereNum; i++)
        {
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, sphereMap[i][0]);
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, sphereMap[i][1]);
            glActiveTexture(GL_TEXTURE5);
            glBindTexture(GL_TEXTURE_2D, sphereMap[i][2]);
            glActiveTexture(GL_TEXTURE6);
            glBindTexture(GL_TEXTURE_2D, sphereMap[i][3]);
            glActiveTexture(GL_TEXTURE7);
            glBindTexture(GL_TEXTURE_2D, sphereMap[i][4]);

            //float x = circleR * cos(theta[i]) * 2;
            //float y = circleR * sin(theta[i]) * 2;

            float x = circleR * cos(theta[i] + radian) * 2;
            float y = circleR * sin(theta[i] + radian) * 2;

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(x, 0, y));
            //model = glm::rotate(model, 0.0f, glm::vec3(0.0f, -1.0f, 0.0f));
            model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.0f, -1.0f, 0.0f));
            myPbrShader->setMat4("model", model);
            renderSphere();
        }

        for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
        {
            //std::cout << sizeof(lightPositions) / sizeof(lightPositions[0]) << endl;
            glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * 5.0) * 5.0, 0.0, 0.0);
            newPos = lightPositions[i];
            glm::vec3 newColor = lightColors[i];
            myPbrShader->setVec3("lightPositions[" + std::to_string(i) + "]", newPos.x, newPos.y, newPos.z);
            myPbrShader->setVec3("lightColors[" + std::to_string(i) + "]", newColor.x, newColor.y, newColor.z);


            model = glm::mat4(0.5f);
            model = glm::translate(model, newPos);
            model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(1, 0, 0));
            myPbrShader->setMat4("model", model);
        }

        backgroundShader->use();
        backgroundShader->setMat4("view", view);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
        renderCube();

        //Clean up, prepare for next render loop
        glfwSwapBuffers(window);
        glfwPollEvents();

        static int ticks = -1;
        ticks++;
        static double _lastTime = glfwGetTime();

        if (ticks == 60)
        {
            double deltaTime = glfwGetTime() - _lastTime;
            _lastTime = glfwGetTime();

            cout << "FPS:" << ticks / deltaTime << endl;
            ticks = 0;
        }
    }

    //Exit program
    glfwTerminate();
    return 0;
}

unsigned int sphereVAO = 0;
unsigned int indexCount;
void renderSphere()
{
    if (sphereVAO == 0)
    {
        glGenVertexArrays(1, &sphereVAO);

        unsigned int vbo, ebo;
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uv;
        std::vector<glm::vec3> normals;
        std::vector<unsigned int> indices;

        const unsigned int X_SEGMENTS = 64;
        const unsigned int Y_SEGMENTS = 64;
        const float PI = 3.14159265359;
        for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
        {
            for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
            {
                float xSegment = (float)x / (float)X_SEGMENTS;
                float ySegment = (float)y / (float)Y_SEGMENTS;
                float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                float yPos = std::cos(ySegment * PI);
                float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

                positions.push_back(glm::vec3(xPos, yPos, zPos));
                uv.push_back(glm::vec2(xSegment, ySegment));
                normals.push_back(glm::vec3(xPos, yPos, zPos));
            }
        }

        bool oddRow = false;
        for (int y = 0; y < Y_SEGMENTS; ++y)
        {
            if (!oddRow) // even rows: y == 0, y == 2; and so on
            {
                for (int x = 0; x <= X_SEGMENTS; ++x)
                {
                    indices.push_back(y * (X_SEGMENTS + 1) + x);
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                }
            }
            else
            {
                for (int x = X_SEGMENTS; x >= 0; --x)
                {
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                    indices.push_back(y * (X_SEGMENTS + 1) + x);
                }
            }
            oddRow = !oddRow;
        }
        indexCount = indices.size();

        std::vector<float> data;
        for (int i = 0; i < positions.size(); ++i)
        {
            data.push_back(positions[i].x);
            data.push_back(positions[i].y);
            data.push_back(positions[i].z);
            if (uv.size() > 0)
            {
                data.push_back(uv[i].x);
                data.push_back(uv[i].y);
            }
            if (normals.size() > 0)
            {
                data.push_back(normals[i].x);
                data.push_back(normals[i].y);
                data.push_back(normals[i].z);
            }
        }
        glBindVertexArray(sphereVAO);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
        float stride = (3 + 2 + 3) * sizeof(float);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));
    }

    glBindVertexArray(sphereVAO);
    glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
}

unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube()
{
    // initialize (if necessary)
    if (cubeVAO == 0)
    {
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}
