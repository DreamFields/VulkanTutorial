#pragma once

#define GLM_FORCE_RADIANS
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include <vulkan/vulkan.h>

#include <array>


struct Vertex {
    // 交错顶点属性，把位置和颜色放在一起
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    // https://vulkan-tutorial.com/Vertex_buffers/Vertex_input_description#page_Attribute-descriptions
    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

        attributeDescriptions[0].binding = 0; // 告诉 Vulkan 每顶点数据来自哪个绑定
        attributeDescriptions[0].location = 0; // 从哪个位置读取顶点数据
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos); // 从顶点数据的哪个位置开始读取

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }
};

// https://vulkan-tutorial.com/Uniform_buffers/Descriptor_layout_and_buffer
// 使用资源描述符（是着色器自由访问缓冲区和图像等资源的一种方式）创建uniform全局变量（很容易每一帧都发生变化）
struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec1 slice;
};

// 在使用时，着色器中的结构体和这里的结构体布局必须一致
struct DicomUniformBufferObject {
    glm::vec3 voxelSize;
    alignas(16) glm::vec3 voxelResolution;
    alignas(16) glm::vec3 boxSize;
    glm::vec1 windowCenter;
    glm::vec1 windowWidth;
    glm::vec1 minVal;
    glm::vec1 alphaCorrection;
    glm::vec1 stepLength;
    glm::vec1 glow;
    glm::int32 steps;
    
};