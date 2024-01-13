#pragma once

#define GLM_FORCE_RADIANS
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include <vulkan/vulkan.h>

#include <array>
#include <vector>
#include <unordered_map>


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
};

// 在使用时，着色器中的结构体和这里的结构体布局必须一致
struct DicomUniformBufferObject {
    glm::vec3 voxelSize;
    alignas(16) glm::vec3 voxelResolution;
    alignas(16) glm::vec3 boxSize;
    alignas(16) glm::vec3 realSize;
    glm::vec1 windowCenter;
    glm::vec1 windowWidth;
    glm::vec1 minVal;
    glm::vec1 alphaCorrection;
    glm::vec1 stepLength;
    glm::vec1 glow;
    glm::int32 steps;
    //glm::int32 renderMethod;
};

// 环境光遮蔽的uniform
struct OcclusionUniformBufferObject {
    //alignas(16) float OccInitialStep; // 3
    //alignas(16) float OccRay7AdjWeight; // 0.972955
    alignas(16) glm::vec4 OccConeRayAxes[10];
    //alignas(16) glm::int32 OccConeIntegrationSamples[3]; // 1,14,0
};

// ground truth uniform
struct GroundTruthUBO{
    alignas(16) glm::vec4 raySampleVec[10]; // todo 如果使用vec3，在着色器中会出现对齐错误，why？
};

struct Compute {
    VkQueue queue;								        // Separate queue for compute commands (queue family may differ from the one used for graphics)
    VkCommandPool commandPool;					        // Use a separate command pool (queue family may differ from the one used for graphics)
    VkDescriptorSetLayout descriptorSetLayout;	        // Compute shader binding layout
    std::vector<VkDescriptorSet> descriptorSets;        // Compute shader bindings
    VkPipelineLayout pipelineLayout;			        // Layout of the compute pipeline
    std::vector<VkPipeline> pipelines;			        // Compute pipelines for image filters
    std::vector<VkCommandBuffer> commandBuffers;				        // Command buffer storing the dispatch commands and barriers
    std::vector<VkFence> inFlightFences;                // Compute fences to check compute command buffer completion
    std::vector<VkSemaphore> finishedSemaphores;        // Compute semaphore to wait for compute completion
    std::vector<VkSemaphore> finishedGenMipmapSemaphores; // Compute semaphore to wait for generate mipmap completion
    std::vector<bool> isComplete;                            // judge if compute is complete
};

struct TextureTarget {
    VkImage image;
    VkImageView imageView;
    VkImageLayout imageLayout;
    VkSampler sampler;
    VkDeviceMemory memory;
    uint32_t mipLevels;
    // VkDescriptorImageInfo descriptor;
};

struct ExampleConfig {
    std::string name;
    std::string path;

    glm::vec3 cameraPos = glm::vec3(0.5f, 0.5f, 2.5f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

    double windowWidth = 100.0f;
    double windowCenter = 0.0f;
};

static inline ExampleConfig head = {
    "head",
    "C:\\Users\\Dream\\Documents\\00.Dicom\\ede6fe9eda6e44a98b3ad20da6f9116a Anonymized29\\Unknown Study\\CT Head 5.0000\\",
    glm::vec3(0.0709212, -1.45329, 0.47473),
    glm::vec3(0.214537, 0.976634, 0.012635),
    glm::vec3(-0.0384151, -0.00448895, 0.999252),
    250.0f,
    250.0f
};

static inline ExampleConfig mouse = {
    "mouse",
    "C:\\Users\\Dream\\Documents\\00.Dicom\\mouse512\\",
    glm::vec3(1.0173, 1.20632, 2.29839),
    glm::vec3(-0.258629, -0.353131, -0.899116),
    glm::vec3(-0.121613, 0.935281, -0.332354),
    2000.0f,
    50.0f
};

static inline ExampleConfig chest = {
    "chest",
    "C:\\Users\\Dream\\Documents\\00.Dicom\\liudan_wholebodyls_thorax\\",
    glm::vec3(0.44323, -1.26454, 1.43985),
    glm::vec3(0.0283847, 0.882255, -0.469916),
    glm::vec3(-0.0319244, 0.470665, 0.881734),
    2000.0f,
    50.0f
};

static inline std::unordered_map<int, ExampleConfig> dicomExamples = {
    {0, head},
    {1, mouse},
    {2, chest}
};