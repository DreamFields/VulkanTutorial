#include "vulkan_application.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
void VulkanApplication::initVolumeRender() {
    volumeRender = std::make_shared<VolumeRender>();
    volumeRender->loadDicom("C:\\Users\\Dream\\Documents\\00.Dicom\\ede6fe9eda6e44a98b3ad20da6f9116a Anonymized29\\Unknown Study\\CT Head 5.0000\\CT000000.dcm");
}

void VulkanApplication::run() {
    initWindow();
    initVulkan();
    initVolumeRender();
    mainLoop();
    cleanup();
}

void VulkanApplication::createTextureImage(){
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load("D:\\00.CG_project\\VulkanTutorial\\textures\\texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        VkDeviceSize imageSize = texWidth * texHeight * 4;

        if (!pixels) {
            throw std::runtime_error("failed to load texture image!");
        }

    }