#include "vulkan_application.h"

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
