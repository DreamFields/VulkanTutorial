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
struct GroundTruthUBO {
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
    bool isComplete;                            // judge if compute is complete
};

// 只在初始化时使用一次的计算着色器
struct StaticCompute {
    VkQueue queue;								        // Separate queue for compute commands (queue family may differ from the one used for graphics)
    VkCommandPool commandPool;					        // Use a separate command pool (queue family may differ from the one used for graphics)
    VkDescriptorSetLayout descriptorSetLayout;	        // Compute shader binding layout
    VkDescriptorSet descriptorSet;                     // Compute shader bindings
    VkPipelineLayout pipelineLayout;			        // Layout of the compute pipeline
    std::vector<VkPipeline> pipelines;			        // Compute pipelines for image filters
    VkCommandBuffer commandBuffer;				        // Command buffer storing the dispatch commands and barriers
    VkFence inFlightFence;                             // Compute fences to check compute command buffer completion
    VkSemaphore finishedSemaphore;
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

// 生成高斯滤波的计算着色器所需的推送常量
struct GenGaussianMMPushConstants {
    glm::vec1 currentLevel;
    bool isCustomResolution;
    int customResolution;
};

struct ExampleConfig {
    std::string name;
    std::string path;

    glm::vec3 cameraPos = glm::vec3(0.5f, 0.5f, 2.5f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

    double windowWidth = 100.0f;
    double windowCenter = 0.0f;
    double stepLength = 0.01f;

    double alphaCorrection = 100.0f;
    double glow = 1.5f;
};
/*
-------------angle0-------------
    glm::vec3(0.0709212, -1.45329, 0.47473),
    glm::vec3(0.214537, 0.976634, 0.012635),
    glm::vec3(-0.0384151, -0.00448895, 0.999252),
-------------angle1-------------
    glm::vec3(-1.40192, -0.117714, 0.534995),
    glm::vec3(0.950949, 0.308853, -0.017497),
    glm::vec3(0.0168407, 0.00479104, 0.999847),
-------------立方体角度----------
    glm::vec3(1.8618, 1.31775, 1.71523),
    glm::vec3(-0.6809, -0.408873, -0.607617),
    glm::vec3(-0.274107, 0.911624, -0.306278),
-------------nrrd angle 0-------------
    glm::vec3(0.0292941, 2.44313, 0.553435),
    glm::vec3(0.235348, -0.971544, -0.0267169),
    glm::vec3(-0.016332, 0.0235319, -0.99959),
-------------nrrd angle 1-------------
    glm::vec3(-1.36239, 1.2285, 0.530957),
    glm::vec3(0.931176, -0.364242, -0.0154783),
    glm::vec3(-0.0159918, 0.00160645, -0.999871),
-------------wlww0-------------
    250.0f, // windowCenter
    250.0f, // windowWidth
    0.001f, // stepLength
    100.0f, // alphaCorrection
-------------wlww1-------------
    1500.0f, // windowCenter
    50.0f, // windowWidth
    0.001f, // stepLength
    200.0f, // alphaCorrection
 */
static inline ExampleConfig head = {
    "head512*512*41",
    "C:\\Users\\Dream\\Documents\\00.Dicom\\ede6fe9eda6e44a98b3ad20da6f9116a Anonymized29\\Unknown Study\\CT Head 5.0000\\",
    glm::vec3(0.0292941, 2.44313, 0.553435),
    glm::vec3(0.235348, -0.971544, -0.0267169),
    glm::vec3(-0.016332, 0.0235319, -0.99959),
    250.0f, // windowCenter
    250.0f, // windowWidth
    0.001f, // stepLength
    100.0f, // alphaCorrection
    1.5f // glow
};


/*
--------------angle0-------------
    glm::vec3(1.0173, 1.20632, 2.29839),
    glm::vec3(-0.258629, -0.353131, -0.899116),
    glm::vec3(-0.121613, 0.935281, -0.332354),
-------------angle1-------------
    glm::vec3(-0.956015, 1.27532, 1.6312),
    glm::vec3(0.727942, -0.387625, -0.56555),
    glm::vec3(0.247846, 0.91784, -0.31007),
------------nrrd angle0-------------
    glm::vec3(0.895611, -0.815965, -0.953459),
    glm::vec3( -0.197786, 0.657915, 0.726655),
    glm::vec3( -0.293191, -0.747069, 0.596596),
-------------nrrd angle1-------------
    glm::vec3(-0.758372, -0.067949, -0.947371),
    glm::vec3(0.629111, 0.283941, 0.7236),
    glm::vec3(0.156054, -0.958079, 0.240274),
-------------wlww0-------------
    2000.0f,
    50.0f,
    0.01f, // stepLength
    25.0f,  // alphaCorrection
-------------wlww1-------------
    540.0f,
    800.0f,
    0.003f,
    25.0f,
 */
static inline ExampleConfig mouse = {
    "mouse512*512*512",
    "C:\\Users\\Dream\\Documents\\00.Dicom\\mouse512\\",
    glm::vec3(0.895611, -0.815965, -0.953459),
    glm::vec3( -0.197786, 0.657915, 0.726655),
    glm::vec3( -0.293191, -0.747069, 0.596596),
    2000.0f, // windowCenter
    50.0f, // windowWidth
    0.01f, // stepLength
    25.0f,  // alphaCorrection
    2.5f // glow
};

// static inline ExampleConfig thorax = {
//     "thorax512*512*49",
//     "C:\\Users\\Dream\\Documents\\00.Dicom\\liudan_wholebodyls_thorax\\",
//     glm::vec3(0.44323, -1.26454, 1.43985),
//     glm::vec3(0.0283847, 0.882255, -0.469916),
//     glm::vec3(-0.0319244, 0.470665, 0.881734),
//     2000.0f,
//     50.0f,
//     0.001f,
//     200.0f,
//     4.0f
// };

static inline ExampleConfig brain ={
    "brain192*192*47",
    "C:\\Users\\Dream\\Documents\\00.Dicom\\brain\\",
    glm::vec3(1.58988, -0.58501, -0.778855),
    glm::vec3(-0.544902, 0.542467, 0.639383),
    glm::vec3(-0.583504, 0.302292, -0.753752),
    1325.0f,
    3100.0f,
    0.001f,
    40.0f,
    4.5f
};

static inline ExampleConfig whole_body = {
    "wholeBody250*250*756",
    "C:\\Users\\Dream\\Documents\\00.Dicom\\zzw_wholebodyls_756\\",
    glm::vec3(-1.03452, 1.3814, 0.204739),
    glm::vec3(0.855314, -0.491278, 0.164574),
    glm::vec3(0.1547, -0.0609937, -0.986077),
    1025.0f,
    575.0f,
    0.0001f,
    93.75f,
    8.75f
};

static inline ExampleConfig frog = {
    "frog_256x256x44_uint8",
    "C:\\Users\\Dream\\Documents\\00.NRRD\\frog_256x256x44_uint8",
    glm::vec3(1.47557, 0.912226, -1.19671),
    glm::vec3(-0.487754, -0.206101, 0.848303),
    glm::vec3(-0.769853, -0.35662, -0.52929),
    105.0f,
    100.0f,
    0.001f,
    500.0f,
    1.25f
};

static inline ExampleConfig tooth = {
    "tooth_103x94x161_uint8",
    "C:\\Users\\Dream\\Documents\\00.NRRD\\tooth_103x94x161_uint8",
    glm::vec3(0.189795, 1.87083, 1.92314),
    glm::vec3(0.155088, -0.685352, -0.711505),
    glm::vec3(0.0876571, -0.707832, 0.700921),
    125.0f,
    175.0f,
    0.001f,
    500.0f,
    2.0f
};

// static inline ExampleConfig vertebra={
//     "vertebra_512x512x512_uint16",
//     "C:\\Users\\Dream\\Documents\\00.NRRD\\vertebra_512x512x512_uint16",
//     glm::vec3(1.47557, 0.912226, -1.19671),
//     glm::vec3(-0.487754, -0.206101, 0.848303),
//     glm::vec3(-0.769853, -0.35662, -0.52929),
//     400.0f,
//     750.0f,
//     0.001f,
//     256.0f,
//     1.5f
// };

static inline ExampleConfig stage_beetle = {
    "stag_beetle_832x832x494_uint16",
    "C:\\Users\\Dream\\Documents\\00.NRRD\\stag_beetle_832x832x494_uint16",
    glm::vec3(0.683341, 1.93495, -0.881284),
    glm::vec3(-0.0916631, -0.717417, 0.690587),
    glm::vec3(0.181913, -0.6939, -0.696714),
    1175.0f,
    750.0f,
    0.001f,
    55.0f,
    8.0f
};

// 数据读取错误
// static inline ExampleConfig chameleon={
//     "chameleon_1024x1024x1080_uint16",
//     "C:\\Users\\Dream\\Documents\\00.NRRD\\chameleon_1024x1024x1080_uint16",
//     glm::vec3(1.47557, 0.912226, -1.19671),
//     glm::vec3(-0.487754, -0.206101, 0.848303),
//     glm::vec3(-0.769853, -0.35662, -0.52929),
//     250.0f,
//     150.0f,
//     0.001f,
//     100.0f,
//     1.5f
// };

/* -----------------配置1-----------------
    glm::vec3(0.237374, -1.47586, 0.332677),
    glm::vec3(0.131297, 0.987808, 0.0836511),
    glm::vec3( -0.00399074, -0.0838542, 0.99647),
    225.0f,
    1300.0f,
    0.001f,
    500.0f,
    1.375f
-----------------配置2-----------------
    glm::vec3(-0.469145, -0.642887, 1.82505),
    glm::vec3(0.484501, 0.571359, -0.662425),
    glm::vec3(0.481504, -0.806387, -0.343356),
    2515.0f,
    1725.0f,
    0.001f,
    1000.0f,
    1.8f
 
*/
static inline ExampleConfig prone = {
    "prone_512x512x463_uint16",
    "C:\\Users\\Dream\\Documents\\00.NRRD\\prone_512x512x463_uint16",
    glm::vec3(0.237374, -1.47586, 0.332677),
    glm::vec3(0.131297, 0.987808, 0.0836511),
    glm::vec3( -0.00399074, -0.0838542, 0.99647),
    225.0f,
    1300.0f,
    0.001f,
    500.0f,
    1.375f
};

// 数据读取错误
// static inline ExampleConfig beechnut={
//     "beechnut_1024x1024x1546_uint16",
//     "C:\\Users\\Dream\\Documents\\00.NRRD\\beechnut_1024x1024x1546_uint16",
//     glm::vec3(-0.180568, -1.38087, 0.502245),
//     glm::vec3(0.340248, 0.940335, -0.00112224),
//     glm::vec3(0.138838, -0.0514169, -0.98898),
//     650.0f,
//     1475.0f,
//     0.001f,
//     500.0f,
//     1.5f
// };

static inline std::unordered_map<int, ExampleConfig> dicomExamples = {
    {0, head},
    {1, mouse},
    // {2, thorax},
    {2, brain},
    {3, whole_body},
    {4, frog},
    {5, tooth},
    // {6, vertebra},
    {6, stage_beetle},
    // {7, chameleon}
    {7, prone},
    // {8, beechnut}
};

const std::string lutPath = "D:\\00.CG_project\\00.VulkanTutorial\\textures\\";
static inline std::unordered_map<int, std::string> lookUpTables = {
    {0,"cm_viridis"},
    {1,"pet"},
    {2,"normal"},
    {3,"cm_gray"},
    {4,"spectral"},
    {5,"hot_iron"},
    {6,"hot_metal_blue"}
};

