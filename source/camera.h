#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>

#define GLM_FORCE_RADIANS
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include "volume_render.h"

class Camera
{
public:
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;
    bool rightMouseDown = false;  // 是否按下右键
    bool leftMouseDown = false;  // 是否按下左键
    glm::vec3 cameraTarget = glm::vec3(0.5f, 0.5f, 0.5f); 
    glm::vec3 cameraPos = glm::vec3(0.5f, 0.5f, 2.5f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    float cameraSpeed = 0.005f;
    bool firstMouse = true;
    bool isSurround = true; // 相机是否环绕
public:
    Camera(int exampleID);
    void updateCameraByConfig(int exampleID);
    void onMouseMove(GLFWwindow* window, double xpos, double ypos);
    void onMouseButton(GLFWwindow* window, int button, int action, int mods);
    void updateCameraMove(GLFWwindow* window);
    // 得到视图矩阵
    glm::mat4 getViewMatrix();
};
