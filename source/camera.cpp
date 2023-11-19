#include "camera.h"

void Camera::onMouseMove(GLFWwindow* window, double xpos, double ypos)
{
    if (leftMouseDown) {
        if (firstMouse) {
            lastMouseX = xpos;
            lastMouseY = ypos;
            firstMouse = false;
        }

        float xoffset = xpos - lastMouseX;

        // 鼠标坐标的y是从上到下增大的，而摄像机的y是从下到上增大的，所以要取反
        float yoffset = lastMouseY - ypos;

        lastMouseX = xpos;
        lastMouseY = ypos;

        float sensitivity = 0.002f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        // 根据offset值直接计算出新的方向向量
        glm::vec3 newFront = cameraFront + xoffset * glm::normalize(glm::cross(cameraFront, cameraUp)) + yoffset * cameraUp;
        cameraFront = glm::normalize(newFront);

        // 根据方向向量计算出旋转矩阵，然后将旋转矩阵应用到摄像机的up向量上
        glm::vec3 newUp = glm::cross(glm::cross(cameraFront, cameraUp), cameraFront);
        cameraUp = glm::normalize(newUp);

        std::cout << "cameraFront: " << cameraFront.x << ", " << cameraFront.y << ", " << cameraFront.z << std::endl;
        std::cout << "cameraUp: " << cameraUp.x << ", " << cameraUp.y << ", " << cameraUp.z << std::endl;
    }
}

void Camera::onMouseButton(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            rightMouseDown = true;
            firstMouse = true; // 重置firstMouse，使鼠标移动时不会有跳跃
        }
        else if (action == GLFW_RELEASE) {
            rightMouseDown = false;
        }
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            leftMouseDown = true;
            firstMouse = true; // 重置firstMouse，使鼠标移动时不会有跳跃
        }
        else if (action == GLFW_RELEASE) {
            leftMouseDown = false;
        }
    }
}

void Camera::updateCameraMove(GLFWwindow* window){
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        cameraPos += cameraSpeed * cameraFront;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        cameraPos -= cameraSpeed * cameraFront;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
    // 通过键盘的E和Q来控制相机上升和下降
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        cameraPos += cameraSpeed * cameraUp;
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        cameraPos -= cameraSpeed * cameraUp;
    }
}

glm::mat4 Camera::getViewMatrix()
{
    // 根据相机参数手动计算view矩阵
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    return view;
}