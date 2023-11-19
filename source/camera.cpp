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
        float yoffset = lastMouseY - ypos; // y轴反转

        lastMouseX = xpos;
        lastMouseY = ypos;

        float sensitivity = 0.1f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        float yaw = glm::radians(xoffset);
        float pitch = glm::radians(yoffset);

        // 根据鼠标移动计算相机的旋转
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), yaw, glm::vec3(0.0f, 1.0f, 0.0f)) *
            glm::rotate(glm::mat4(1.0f), pitch, glm::vec3(1.0f, 0.0f, 0.0f));

        // 转换相机的方向向量
        cameraFront = glm::mat3(rotation) * cameraFront;
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
}