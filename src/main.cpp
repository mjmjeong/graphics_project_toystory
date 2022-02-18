#define GLM_ENABLE_EXPERIMENTAL
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.h"
#include "opengl_utils.h"
#include "geometry_primitives.h"
#include <iostream>
#include <vector>
#include<string>
#include "camera.h"
#include "texture.h"
#include "texture_cube.h"
#include "math_utils.h"
#include "light.h"
#include "FreeImage/FreeImage.h"
#include <time.h>
#include <cmath>

#include "model_multi.h"
#include "mesh_multi.h"

#include "model_ani.h"
#include "mesh_ani.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow* window, DirectionalLight* sun);
void saveImage(const char* filename);
glm::vec3 cal_ani_location(Camera camera, bool jump, float starting_jump, float currentTime);
void renderscreen();

bool isWindowed = true;
bool isKeyboardDone[1024] = { 0 };
// setting
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
const float planeSize = 70.f;
//const float planeSize = 1.f;
const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;

// camera
Camera camera(glm::vec3(0.0f, 3.3f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;
float score = 0.0f;

// Save Image to png file. press V key.
// file name : date.png (created in bin folder)
void saveImage(const char* filename) {
    // Make the BYTE array, factor of 3 because it's RBG.
    BYTE* pixels = new BYTE[3 * SCR_WIDTH * SCR_HEIGHT];
    glReadPixels(0, 0, SCR_WIDTH, SCR_HEIGHT, GL_BGR, GL_UNSIGNED_BYTE, pixels);

    // Convert to FreeImage format & save to file
    FIBITMAP* image = FreeImage_ConvertFromRawBits(pixels, SCR_WIDTH, SCR_HEIGHT, 3 * SCR_WIDTH, 24, 0xFF0000, 0x00FF00, 0x0000FF, false);
    FreeImage_Save(FIF_PNG, image, filename, 0);

    // Free resources
    FreeImage_Unload(image);
    delete[] pixels;
}

bool useNormalMap = true;
bool useSpecular = false;
bool useShadow = true;
bool useLighting = true;
bool bloom = true;
bool bloomKeyPressed = false;
bool jump = false;
float starting_jump = 0.0f;
float exposure = 1.0f;
float ani_direction = 0.0f;
int main()
{
    //stbi_set_flip_vertically_on_load(true);
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader program
    // ------------------------------------
    Shader lightingShader("../shaders/shader_lighting.vs", "../shaders/shader_lighting.fs"); 
    Shader lighting_sourceShader("../shaders/shader_lighting.vs", "../shaders/lighting_source.fs"); 
    Shader blurShader("../shaders/blur.vs", "../shaders/blur.fs"); 
    Shader bloomShader("../shaders/bloom.vs", "../shaders/bloom.fs"); 
    Shader shadowShader("../shaders/shadow.vs", "../shaders/shadow.fs");
    Shader skyboxShader("../shaders/shader_skybox.vs", "../shaders/shader_skybox.fs");
    Shader aniShader("../shaders/shader_ani.vs", "../shaders/shader_ani.fs"); 
    
    //animation
    
    Model_ani testModel("../resources/animation/test.dae");
    Texture Texture_ani("../resources/animation/CharacterTexture.png");
    unsigned int ani_texture = Texture_ani.ID;
    
    vector <glm::vec3> Coins_location;
    vector <glm::mat4> Coins_world;
    Model_multi Coins_model("../resources/goldcoin/gold_coin.obj");
    
    for (unsigned int i= 0; i<20; i++){
        Coins_location.push_back(glm::vec3(0.10f * planeSize*cos(i*3.14f*20.0f/180.0f), 0.0f, 0.10f * planeSize * sin(i*3.14f*20.0f/180.f)));
    }
    for (unsigned int i = 0; i < 30; i++) {
        Coins_location.push_back(glm::vec3(-14.0f,7.5f,25.3f)+glm::vec3(7* cos(i*3.14f*12.0f/180.0f), 0.0f, 7* sin(i*3.14f*12.0f/180.0f)));
    }
    for (unsigned int i = 0; i < 10; i++) {
        Coins_location.push_back(glm::vec3(-25.0f, 0.0f, 15.0f-2.5*i));
    }
    for (unsigned int i = 0; i < 10; i++) {
        Coins_location.push_back(glm::vec3(-25.0f+2.5*i, 0.0f, -10.f));
    }


    for (unsigned int i = 0; i < Coins_location.size(); i++) {
        float scale_coin = 0.05 * planeSize;
        glm::mat4 coin_buffer = glm::rotate(glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)) * glm::scale(glm::vec3(scale_coin));
        coin_buffer = glm::translate(Coins_location[i]) * glm::translate(glm::vec3(0.0f, 0.25*scale_coin,0.0f))*coin_buffer;
        Coins_world.push_back(coin_buffer);
    }
    
    vector <Model_multi> Models;
    vector <glm::mat4> Models_world;
    //bed
    Models.push_back(Model_multi("../resources/andysroom/bed.obj"));
    Models_world.push_back(glm::translate(glm::vec3(-0.27 * planeSize, 0.0f, 0.37 * planeSize)) * glm::scale(glm::vec3(0.28 * planeSize)));
    
    //bed table
    Models.push_back(Model_multi("../resources/andysroom/bedtable.obj"));
    Models_world.push_back(glm::translate(glm::vec3(-0.43 * planeSize, 0.0f, 0.12 * planeSize)) * glm::scale(glm::vec3(0.28 * planeSize)));
    //desk    
    Models.push_back(Model_multi("../resources/andysroom/desk.obj"));
    Models_world.push_back(glm::translate(glm::vec3(0.0f, 0.0f, -0.38 * planeSize)) * glm::scale(glm::vec3(0.24 * planeSize)));
    
    //shelf
    Models.push_back(Model_multi("../resources/andysroom/shelf.obj"));
    Models_world.push_back(glm::translate(glm::vec3(0.45 * planeSize, 0.0f,0.0f)) * glm::scale(glm::vec3(0.24 * planeSize)));
    
    
    //Hamms
    Models.push_back(Model_multi("../resources/Hamm/hamm.obj"));
    Models_world.push_back(glm::translate(glm::vec3(planeSize*0.2f, 2.0f, planeSize * 0.1f))
        * glm::rotate(glm::radians(-60.0f), glm::vec3(0.0f, 1.0f, 0.0f))
        * glm::rotate(glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f))
        *glm::scale(glm::vec3(0.50f)));

    //Dogs
    Models.push_back(Model_multi("../resources/SlinkyDog/dog.obj"));
    Models_world.push_back(glm::translate(glm::vec3(planeSize * 0.2f, 1.0f, -planeSize * 0.1f))
        * glm::rotate(glm::radians(60.0f), glm::vec3(0.0f, 1.0f, 0.0f))
        * glm::rotate(glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f))
        * glm::scale(glm::vec3(0.50f)));
    //Losto
    Models.push_back(Model_multi("../resources/Lotso/Lotso.obj"));
    Models_world.push_back(
        glm::translate(glm::vec3(-planeSize * 0.45f, 0.0f, -0.07f*planeSize))
        * glm::rotate(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f))
        *glm::scale(glm::vec3(4.0f)));

    //boxes
    Models.push_back(Model_multi("../resources/box/box1.obj"));
    Models_world.push_back(glm::translate(planeSize * glm::vec3(0.3f, 0.0f, 0.2f))
        * glm::rotate(glm::radians(-20.0f), glm::vec3(0.0f, 1.0f, 0.0f))
        * glm::scale(glm::vec3(4 * planeSize)));
    
    Models.push_back(Model_multi("../resources/box/box2.obj"));
    glm::mat4 box_world2 = glm::rotate(glm::radians(30.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    box_world2 = glm::scale(glm::vec3(4 * planeSize)) * box_world2;
    box_world2 = glm::translate(planeSize * glm::vec3(0.1f, 0.0f, 0.43)) * box_world2;
    Models_world.push_back(box_world2);

    Models.push_back(Model_multi("../resources/box/box3.obj"));
    glm::mat4 box_world3 = glm::scale(glm::vec3(4 * planeSize));
    box_world3 = glm::rotate(glm::radians(20.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * box_world3;
    box_world3 = glm::translate(planeSize * glm::vec3(0.1f, 0.0f, 0.3f)) * box_world3;
    Models_world.push_back(box_world3);
    

    // simple cube - VAO
    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_positions_textures), &cube_positions_textures, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    //source    
    vector <glm::mat4> lighting_source_world;
    //source1
    glm::mat4 source1 = glm::translate(glm::vec3(0.0, 0.5 * planeSize - 4, 0.0)) * glm::scale(glm::vec3(0.2 * planeSize, 0.01, 0.2 * planeSize));
    lighting_source_world.push_back(source1);
    //source2
    glm::mat4 source2 = glm::translate(glm::vec3(-0.093 * planeSize, 0.275 * planeSize, -0.37 * planeSize)) * glm::scale(glm::vec3(0.3, 0.3, 0.3));
    lighting_source_world.push_back(source2);

    //simple quad - VAO
    unsigned int EBO, quadVAO, quadVBO;
    glGenBuffers(1, &EBO);
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_positions_textures_normal), quad_positions_textures_normal, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_indices), quad_indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    //Texture
    vector <Texture> planes_texture;
    vector <glm::mat4> planes_world;
    
    planes_texture.push_back("../resources/andysroom/Textures/Posters/Potato.png");
    planes_world.push_back(
                    glm::rotate(glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f))
                    *glm::translate(glm::vec3(0.1f*planeSize, 0.25*planeSize, -0.5 * planeSize + 0.01f))
                    *glm::scale(glm::vec3(0.2 * planeSize)));
    
    planes_texture.push_back("../resources/andysroom/Textures/Floor/Floor_paint.png");
    planes_world.push_back(glm::rotate(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f))* glm::scale(glm::vec3(planeSize)));
    
    for (int i = 0; i < 4; i++)
    {
        planes_texture.push_back("../resources/andysroom/Textures/Walls/East_Wall_paint.png");
        glm::mat4 wall_world = glm::scale(glm::vec3(planeSize)) * glm::translate(glm::vec3(0.f, 0.0f, -0.5f));
        planes_world.push_back(glm::rotate(glm::radians(-90.0f * i), glm::vec3(0.0f, 1.0f, 0.0f)) * wall_world);
    }
    
    planes_texture.push_back("../resources/andysroom/Textures/Walls/East_Wall_paint.png");
    glm::mat4 ceil_world = glm::rotate(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::scale(glm::vec3(planeSize));
    ceil_world = glm::translate(glm::vec3(0.0f, 0.5f * planeSize-2, 0.0f)) * ceil_world;
    planes_world.push_back(ceil_world);
    
    // skybox
    std::vector<std::string> faces
    {
        "../resources/skybox/right.jpg",
        "../resources/skybox/left.jpg",
        "../resources/skybox/top.jpg",
        "../resources/skybox/bottom.jpg",
        "../resources/skybox/front.jpg",
        "../resources/skybox/back.jpg"
    };
    CubemapTexture skyboxTexture = CubemapTexture(faces);
    unsigned int VAOskybox, VBOskybox;
    getPositionVAO(skybox_positions, sizeof(skybox_positions), VAOskybox, VBOskybox);

    //shader-texture number
    lightingShader.use();
    lightingShader.setInt("material.diffuseSampler", 0);
    lightingShader.setInt("material.specularSampler", 1);
    lightingShader.setInt("material.normalSampler", 2);
    lightingShader.setInt("depthMapSampler", 3);
    lightingShader.setFloat("material.shininess", 64.f);    // set shininess to constant value.
    aniShader.use();
    aniShader.setInt("material.diffuseSampler", 0);
    aniShader.setInt("material.specularSampler", 1);
    aniShader.setInt("material.normalSampler", 2);
    aniShader.setInt("depthMapSampler", 3);
    aniShader.setFloat("material.shininess", 64.f);    // set shininess to constant value.    
    blurShader.use();
    blurShader.setInt("image", 0);
    bloomShader.use();
    bloomShader.setInt("scene", 0);
    bloomShader.setInt("bloomBlur", 1);
    skyboxShader.use();
    skyboxShader.setInt("skyboxTexture1", 0);

    // configure (floating point) framebuffers
    //(1)depth texture - shadow
    DepthMapTexture depth = DepthMapTexture(SHADOW_WIDTH, SHADOW_HEIGHT);
    //(2) hdr
    unsigned int hdrFBO;
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    // create 2 floating point color buffers (1 for normal rendering, other for brightness treshold values)
    unsigned int colorBuffers[2];
    glGenTextures(2, colorBuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // attach texture to framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
    }
    // create and attach depth buffer (renderbuffer)
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);
    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //(3) pingpong
    unsigned int pingpongFBO[2];
    unsigned int pingpongColorbuffers[2];
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongColorbuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
        // also check if framebuffers are complete (no need for depth buffer)
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
    }

    //directionlight
    DirectionalLight sun(30.0f, 30.0f, glm::vec3(0.8f));
    float oldTime = 0;
    while (!glfwWindowShouldClose(window))// render loop
    {
        float currentTime = glfwGetTime();
        float dt = currentTime - oldTime;
        deltaTime = dt;
        oldTime = currentTime;

        // input
        processInput(window, &sun);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // (1) render shadow map!
        float near_plane = 0.05f, far_plane = 70;
        glm::mat4 lightProjection = glm::ortho(-45.0f, 45.0f, -45.0f, 45.0f, near_plane ,far_plane);
        glm::mat4 lightView = glm::lookAt(glm::vec3(0.0, 0.5*planeSize-2,0.0), glm::vec3(0.0f), glm::vec3(1.0, 0.0, 0.0));

        shadowShader.use();
        shadowShader.setMat4("lightSpaceMatrix", lightProjection * lightView);
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depth.depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depth.ID);

        for (int i = 0; i < Models.size(); i++)
        {
            shadowShader.setMat4("world", Models_world[i]);
            Models[i].Draw(shadowShader);
        }

        glBindVertexArray(quadVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depth.ID);

        for (int i = 0; i < planes_world.size(); i++) {
            shadowShader.setMat4("world", planes_world[i]);
            glBindTexture(GL_TEXTURE_2D, planes_texture[i].ID);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // (2) render objects in the scene!
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT); // reset viewport
        
        aniShader.use();
        glDepthFunc(GL_LEQUAL);
        glm::vec3 ani_location = cal_ani_location(camera, jump, starting_jump, currentTime);
        glm::mat4 ani_world = glm::rotate(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::scale(glm::vec3(0.003 * planeSize));
        ani_world = glm::rotate(glm::radians(-camera.Yaw + 90.0f+ani_direction), glm::vec3(0.0f, 1.0f, 0.0f)) * ani_world;
        ani_world = glm::translate(ani_location) * ani_world;
        aniShader.setMat4("world", ani_world);
        aniShader.setFloat("jump_height", ani_location.y);
        aniShader.setMat4("view", view);
        aniShader.setMat4("projection", projection);
        aniShader.setFloat("useLighting", float(useLighting));
        aniShader.setVec3("dirLight.color", sun.lightColor);
        aniShader.setVec3("dirLight.direction", sun.lightDir);
        aniShader.setVec3("viewPos", camera.Position);
        aniShader.setMat4("lightSpaceMatrix", lightProjection* lightView);

        aniShader.setVec3("spotLights[0].position", 0.f, planeSize, 0.f);
        aniShader.setVec3("spotLights[0].direction", 0.f, -planeSize, 0.f);
        aniShader.setVec3("spotLights[0].ambient", 0.4f, 0.4f, 0.4f);
        aniShader.setVec3("spotLights[0].diffuse", 4.0f, 4.0f, 4.0f);
        aniShader.setVec3("spotLights[0].specular", 1.0f, 1.0f, 1.0f);
        aniShader.setFloat("spotLights[0].constant", 1.0f);
        aniShader.setFloat("spotLights[0].linear", 0.002);
        aniShader.setFloat("spotLights[0].quadratic", 0.001);
        aniShader.setFloat("spotLights[0].cutOff", glm::cos(glm::radians(40.f)));
        aniShader.setFloat("spotLights[0].outerCutOff", glm::cos(glm::radians(50.f)));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ani_texture);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, depth.ID);

        const int simul = 0;
        Assimp::Importer importer;
        const aiScene* scene_w = importer.ReadFile("../resources/animation/test.dae", aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

        vector <BoneInfo> boneInfos_w = testModel.boneInfos;
        map<string, int> boneMap_w = testModel.boneMap;
        AnimationComponent animation_comp(scene_w, boneInfos_w, boneMap_w);
        vector<glm::mat4> transfered_bone = animation_comp.ExtractBoneTransforms(currentTime);

        for (int i = 0; i < transfered_bone.size(); i++) {
            aniShader.setMat4("bones[" + to_string(i) + "]", transfered_bone[i]);
        }
        aniShader.setMat4("bones[0]", transfered_bone[0]);
        aniShader.setMat4("bones[1]", transfered_bone[1]);
        aniShader.setMat4("bones[2]", transfered_bone[2]);
        aniShader.setMat4("bones[3]", transfered_bone[3]);
        aniShader.setMat4("bones[4]", transfered_bone[4]);
        aniShader.setMat4("bones[5]", transfered_bone[5]);
        aniShader.setMat4("bones[6]", transfered_bone[6]);
        aniShader.setMat4("bones[7]", transfered_bone[7]);
        aniShader.setMat4("bones[8]", transfered_bone[8]);
        aniShader.setMat4("bones[9]", transfered_bone[9]);
        aniShader.setMat4("bones[10]", transfered_bone[10]);
        aniShader.setMat4("bones[11]", transfered_bone[11]);
        aniShader.setMat4("bones[12]", transfered_bone[12]);
        aniShader.setMat4("bones[13]", transfered_bone[13]);
        aniShader.setMat4("bones[14]", transfered_bone[14]);
        aniShader.setMat4("bones[15]", transfered_bone[15]);
        aniShader.setFloat("max_depth", far_plane);
        testModel.Draw(aniShader);
        
        //use lighting Shader
        lightingShader.use();
        glDepthFunc(GL_LEQUAL);
        lightingShader.setMat4("view", view);
        lightingShader.setMat4("projection", projection);
        lightingShader.setFloat("useLighting", float(useLighting));
        lightingShader.setVec3("dirLight.color", sun.lightColor);
        lightingShader.setVec3("dirLight.direction", sun.lightDir);
        lightingShader.setVec3("viewPos", camera.Position);
        lightingShader.setMat4("lightSpaceMatrix", lightProjection * lightView);
        
        // spotLight
        
        if (score < 12.0f){
        lightingShader.setVec3("spotLights[0].position", camera.Position - glm::vec3(0.f, 0.2f, 0));
        lightingShader.setVec3("spotLights[0].direction", camera.Front- glm::vec3(0.f, 0.1f, 0));
        lightingShader.setVec3("spotLights[0].ambient", 0.0f, 0.0f, 0.0f);
        lightingShader.setVec3("spotLights[0].diffuse", 5.0f, 5.0f, 5.0f);
        lightingShader.setVec3("spotLights[0].specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("spotLights[0].constant", 1.0f);
        lightingShader.setFloat("spotLights[0].linear", 0.02);
        lightingShader.setFloat("spotLights[0].quadratic", 0.01);
        lightingShader.setFloat("spotLights[0].cutOff", glm::cos(glm::radians(4.f)));
        lightingShader.setFloat("spotLights[0].outerCutOff", glm::cos(glm::radians(14.f)));
        }
        else{
        lightingShader.setVec3("spotLights[0].position", 0.f, planeSize, 0.f);
        lightingShader.setVec3("spotLights[0].direction", 0.f, -planeSize, 0.f);
        lightingShader.setVec3("spotLights[0].ambient", 0.4f, 0.4f, 0.4f);
        lightingShader.setVec3("spotLights[0].diffuse", 1.2f, 1.2f, 1.2f);
        lightingShader.setVec3("spotLights[0].specular", 0.5f, 0.5f, 0.5f);
        lightingShader.setFloat("spotLights[0].constant", 1.0f);
        lightingShader.setFloat("spotLights[0].linear", 0.000f);
        lightingShader.setFloat("spotLights[0].quadratic", 0.0001f);
        lightingShader.setFloat("spotLights[0].cutOff", glm::cos(glm::radians(50.f)));
        lightingShader.setFloat("spotLights[0].outerCutOff", glm::cos(glm::radians(60.f)));
        }
        lightingShader.setVec3("spotLights[1].position", glm::vec3(-0.093 * planeSize, 0.275 * planeSize, -0.37 * planeSize));
        lightingShader.setVec3("spotLights[1].direction", glm::vec3(0.7f, -1.3f, 0.5f));
        lightingShader.setVec3("spotLights[1].ambient", 0.0f, 0.0f, 0.0f);
        lightingShader.setVec3("spotLights[1].diffuse", 3.0f, 3.0f, 1.0f);
        lightingShader.setVec3("spotLights[1].specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("spotLights[1].constant", 1.0f);
        lightingShader.setFloat("spotLights[1].linear", 0.02);
        lightingShader.setFloat("spotLights[1].quadratic", 0.01);
        lightingShader.setFloat("spotLights[1].cutOff", glm::cos(glm::radians(4.f)));
        lightingShader.setFloat("spotLights[1].outerCutOff", glm::cos(glm::radians(14.f)));
        
        //multi obj
        //lightingShader.setMat4("world", glm::scale(glm::vec3(0.01 * planeSize)));
        lightingShader.setFloat("coin", 0.0f);
        for (int i=0; i<Models.size(); i++)
        {
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, depth.ID);

            lightingShader.setMat4("world", Models_world[i]);
            Models[i].Draw(lightingShader);
        }
        
        for (int i = Coins_location.size() - 1; i >= 0; --i)
        {
            //if (glm::distance(Coins_location[i].x, ani_location.x) < 1.0) {
            if (pow((Coins_location[i].x - ani_location.x),2)+ pow((Coins_location[i].z - ani_location.z), 2) < 1.0) {
                Coins_location.erase(Coins_location.begin() + i);
                Coins_world.erase(Coins_world.begin() + i);
                score += 1.0f;
            }
        }
        lightingShader.setFloat("coin", 1.0f);
        for (int i = 0; i < Coins_world.size(); i++)
        {
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, depth.ID);

            lightingShader.setMat4("world", Coins_world[i]
                *glm::rotate(glm::radians(-currentTime*600), glm::vec3(1.0f, 0.0f, 0.0f))
                *glm::scale(glm::vec3(0.9f)));
            Coins_model.Draw(lightingShader);
        }
        lightingShader.setFloat("coin", 0.0f);
        //simple posters-buzz
        lightingShader.setFloat("useDiffuseMap", 1.0f);
        lightingShader.setFloat("useSpecularMap", 1.0f);
        glBindVertexArray(quadVAO);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, depth.ID);

        for (int i = 0; i < planes_world.size(); i++) {
            lightingShader.setMat4("world", planes_world[i]);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, planes_texture[i].ID);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
        //
        //bright - lighting_source
        lighting_sourceShader.use();
        lighting_sourceShader.setMat4("view", view);
        lighting_sourceShader.setMat4("projection", projection);
        lighting_sourceShader.setVec3("viewPos", camera.Position);
        lightingShader.setFloat("useNormalMap", float(false));
        glBindVertexArray(cubeVAO);
        for (unsigned int i = 0; i < lighting_source_world.size(); i++)
        {
            lightingShader.setMat4("world", lighting_source_world[i]);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        // don't forget to enable shader before setting uniforms
        
        glDepthFunc(GL_LEQUAL);
        
        // render a skybox
        glBindVertexArray(VAOskybox);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture.textureID);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        // --------------------------------------------------
        // 2. blur bright fragments with two-pass Gaussian Blur 
        
        bool horizontal = true, first_iteration = true;
        unsigned int amount = 10;
        if (useLighting){
        blurShader.use();
        for (unsigned int i = 0; i < amount; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
            blurShader.setInt("horizontal", horizontal);
            glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);  // bind texture of other framebuffer (or scene if first iteration)
            renderscreen();
            horizontal = !horizontal;
            if (first_iteration)
                first_iteration = false;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        // 3. now render floating point color buffer to 2D quad and tonemap HDR colors to default framebuffer's (clamped) color range
        // --------------------------------------------------------------------------------------------------------------------------
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        bloomShader.use();
        bloomShader.setInt("useLighting", useLighting);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
        bloomShader.setInt("bloom", bloom);
        if (useLighting)
        {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
        bloomShader.setFloat("exposure", exposure);
        }
        renderscreen();
       
        
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

unsigned int quadVAO_screen = 0;
unsigned int quadVBO_screen;
void renderscreen()
{
    if (quadVAO_screen == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO_screen);
        glGenBuffers(1, &quadVBO_screen);
        glBindVertexArray(quadVAO_screen);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO_screen);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO_screen);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window, DirectionalLight* sun)
{
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS && isKeyboardDone[GLFW_KEY_V] == false) {
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        char date_char[128];
        sprintf(date_char, "%d_%d_%d_%d_%d_%d.png", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        saveImage(date_char);
        isKeyboardDone[GLFW_KEY_V] = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_V) == GLFW_RELEASE) {
        isKeyboardDone[GLFW_KEY_V] = false;
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.ProcessKeyboard(LEFT, deltaTime);
        ani_direction = 90.f;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.ProcessKeyboard(RIGHT, deltaTime);
        ani_direction = -90.f;
    }

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
        camera.ProcessKeyboard(FORWARD, deltaTime);
        ani_direction = 0.0f;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            ani_direction = -45.f;
        else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            ani_direction = 45.f;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
        camera.ProcessKeyboard(BACKWARD, deltaTime);
        ani_direction = 180.f;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            ani_direction = -135.0f;
        else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            ani_direction = 135.f;
    }
    /*
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !bloomKeyPressed)
    {
        bloom = !bloom;
        bloomKeyPressed = true;
    }
    */
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        if (exposure > 0.0f)
            exposure -= 0.001f;
        else
            exposure = 0.0f;
    }
    else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        exposure += 0.001f;
    }

    float t = 20.0f * deltaTime;
    
    // Arrow key : increase, decrease sun's azimuth, elevation with amount of t.
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        sun->processKeyboard("PLUS_AZIMUTH", t);
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        sun->processKeyboard("MINUS_AZIMUTH", t);
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        sun->processKeyboard("PLUS_ELEVATION", t);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        sun->processKeyboard("MINUS_ELEVATION", t);

    // key 3 : toggle using whole lighting
    glfwSetKeyCallback(window, key_callback);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_RELEASE) return; //only handle press events
    if (key == GLFW_KEY_3) useLighting = !useLighting;
    if (key == GLFW_KEY_2) useShadow = !useShadow;
    if (key == GLFW_KEY_1) useNormalMap = !useNormalMap;
    if (key == GLFW_KEY_J) {
        jump = true;
        starting_jump = glfwGetTime();
    }
    if (key == GLFW_KEY_SPACE)
    bloom = !bloom;
}

glm::vec3 cal_ani_location(Camera camera, bool jump, float starting_jump, float currentTime) {
    glm::vec3 out;
    out = glm::vec3(camera.Position.x + camera.Front.x * 5.5f, 0.0f, camera.Position.z + camera.Front.z * 5.5f);

    /*
    if (out.x < 0.378 && out.z>14.5) 
    {
        out.y = 7.0f;
    }
    */
    float t_gap = 0.9f;
    float delta_t = currentTime - starting_jump;
    float jump_v = 5.0f;
    float jump_a = 2.0f;

    if (jump)
    {
        if (delta_t<t_gap)
            out.y += jump_v * delta_t - jump_v * delta_t * delta_t / t_gap;
        else
            jump = false;

    }
    return out;
}
