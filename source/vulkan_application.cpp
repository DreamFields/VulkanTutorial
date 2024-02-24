#include "vulkan_application.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

void VulkanApplication::run() {
    initWindow();
    initVolume();
    initGeometry();
    initVulkan();
    initImGui();
    mainLoop();
    cleanup();
}

void VulkanApplication::initVolume() {
    volumeRender = std::make_shared<VolumeRender>(currentExampleID);
    // std::string path = "C:\\Users\\Dream\\Documents\\00.Dicom\\ede6fe9eda6e44a98b3ad20da6f9116a Anonymized29\\Unknown Study\\CT Head 5.0000\\";
    // std::string path = "C:\\Users\\Dream\\Documents\\00.Dicom\\mouse512\\";
    // std::string path = "C:\\Users\\Dream\\Documents\\00.Dicom\\liudan_wholebodyls_thorax\\";
    volumeRender->loadDicom(dicomExamples[currentExampleID].path);

    // generate gaussian samples
    volumeRender->GenerateConeSamples();

    // generate ground truth second ray
    volumeRender->GenerateGroundTruthRay();
}

void VulkanApplication::initGeometry() {
    vertices = volumeRender->getBoxVertices();
    // 与vertics中的索引匹配,绘制立方体,按照顺时针绘制
    indices = {
        0, 1, 2, 2, 3, 0,
        1, 5, 6, 6, 2, 1,
        7, 6, 5, 5, 4, 7,
        4, 0, 3, 3, 7, 4,
        4, 5, 1, 1, 0, 4,
        3, 2, 6, 6, 7, 3
    };
}

void VulkanApplication::create1DTextureImage() {
    int texWidth, texHeight, texChannels;
    // STBI_rgb_alpha 值会强制加载图像的 alpha 通道，即使图像没有 alpha 通道也是如此，这有利于将来与其他纹理保持一致。
    // 中间三个参数用于输出图像的宽度、高度和实际通道数。
    // 返回的指针是像素值数组的第一个元素。
    stbi_uc* pixels = stbi_load("D:\\00.CG_project\\00.VulkanTutorial\\textures\\cm_viridis.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    // unsigned char* pixels = nullptr;
    // volumeRender->getPixelRGBA("C:\\Users\\Dream\\Documents\\00.Dicom\\ede6fe9eda6e44a98b3ad20da6f9116a Anonymized29\\Unknown Study\\CT Head 5.0000\\CT000027.dcm", texWidth, texHeight, texChannels, pixels);

    VkDeviceSize imageSize = texWidth * 4;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    // 创建临时缓冲区
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(
        imageSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory);

    // 将数据复制到临时缓冲区
    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(device, stagingBufferMemory);

    // 释放像素数组
    stbi_image_free(pixels);

    /*
    虽然我们可以设置着色器来访问缓冲区中的像素值，但最好还是使用 Vulkan 中的图像对象来实现这一目的。
    首先，图像对象允许我们使用二维坐标，这将使我们更容易、更快速地检索颜色。图像对象中的像素被称为 texels
     */

     // 创建纹理图像
    createImage(
        texWidth,
        1,
        1,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_TYPE_1D,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        lutImage,
        lutImageMemory,
        VK_SAMPLE_COUNT_1_BIT);

    // 将纹理图像转换为 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    transitionImageLayout(
        lutImage,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // 将缓冲区数据复制到图像对象
    copyBufferToImage(
        stagingBuffer,
        lutImage,
        static_cast<uint32_t>(texWidth),
        static_cast<uint32_t>(1),
        1);

    // 为了能够在着色器中开始从纹理图像中采样，我们需要最后一次转换，为着色器访问纹理图像做好准备。
    // 将纹理图像转换为 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    transitionImageLayout(
        lutImage,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // 清理临时缓冲区
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}


void VulkanApplication::create2DTextureImage() {
    int texWidth, texHeight, texChannels;
    // STBI_rgb_alpha 值会强制加载图像的 alpha 通道，即使图像没有 alpha 通道也是如此，这有利于将来与其他纹理保持一致。
    // 中间三个参数用于输出图像的宽度、高度和实际通道数。
    // 返回的指针是像素值数组的第一个元素。
    stbi_uc* pixels = stbi_load("D:\\00.CG_project\\VulkanTutorial\\textures\\texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    // unsigned char* pixels = nullptr;
    // volumeRender->getPixelRGBA("C:\\Users\\Dream\\Documents\\00.Dicom\\ede6fe9eda6e44a98b3ad20da6f9116a Anonymized29\\Unknown Study\\CT Head 5.0000\\CT000027.dcm", texWidth, texHeight, texChannels, pixels);

    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    // 创建临时缓冲区
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(
        imageSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory);

    // 将数据复制到临时缓冲区
    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(device, stagingBufferMemory);

    // 释放像素数组
    stbi_image_free(pixels);

    /*
    虽然我们可以设置着色器来访问缓冲区中的像素值，但最好还是使用 Vulkan 中的图像对象来实现这一目的。
    首先，图像对象允许我们使用二维坐标，这将使我们更容易、更快速地检索颜色。图像对象中的像素被称为 texels
     */

     // 创建纹理图像
    createImage(
        texWidth,
        texHeight,
        1,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_TYPE_2D,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        test2DImage,
        test2DImageMemory,
        VK_SAMPLE_COUNT_1_BIT);

    // 将纹理图像转换为 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    transitionImageLayout(
        test2DImage,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // 将缓冲区数据复制到图像对象
    copyBufferToImage(
        stagingBuffer,
        test2DImage,
        static_cast<uint32_t>(texWidth),
        static_cast<uint32_t>(texHeight),
        1);

    // 为了能够在着色器中开始从纹理图像中采样，我们需要最后一次转换，为着色器访问纹理图像做好准备。
    // 将纹理图像转换为 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    transitionImageLayout(
        test2DImage,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // 清理临时缓冲区
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void VulkanApplication::create3DTextureImage() {
    int texWidth, texHeight, texDepth = 41;
    // STBI_rgb_alpha 值会强制加载图像的 alpha 通道，即使图像没有 alpha 通道也是如此，这有利于将来与其他纹理保持一致。
    // 中间三个参数用于输出图像的宽度、高度和实际通道数。
    // 返回的指针是像素值数组的第一个元素。
    // stbi_uc* pixels = stbi_load("D:\\00.CG_project\\VulkanTutorial\\textures\\texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    unsigned char* pixels = nullptr;
    short channel = 2;
    volumeRender->getPixelRGBA(texWidth, texHeight, texDepth, pixels, channel);

    VkDeviceSize imageSize = texWidth * texHeight * texDepth * channel;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    // 创建临时缓冲区
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(
        imageSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory);

    // 将数据复制到临时缓冲区
    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(device, stagingBufferMemory);

    // 释放像素数组
    stbi_image_free(pixels);

    /*
    虽然我们可以设置着色器来访问缓冲区中的像素值，但最好还是使用 Vulkan 中的图像对象来实现这一目的。
    首先，图像对象允许我们使用二维坐标，这将使我们更容易、更快速地检索颜色。图像对象中的像素被称为 texels
     */

    uint32_t mipLevels = static_cast<uint32_t>(std::floor(
        std::log2(
            std::max(
                std::max(volumeRender->getDicomTags().voxelResolution[0], volumeRender->getDicomTags().voxelResolution[1]),
                volumeRender->getDicomTags().voxelResolution[2])
        ))) + 1;  // mipmap 级别 s

    // 创建纹理图像
    createImage(
        texWidth,
        texHeight,
        texDepth,
        VK_FORMAT_R8G8_UNORM,
        VK_IMAGE_TYPE_3D,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        texture3DImage,
        texture3DImageMemory,
        VK_SAMPLE_COUNT_1_BIT, mipLevels);

    // 将纹理图像转换为 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    transitionImageLayout(
        texture3DImage,
        VK_FORMAT_R8G8_UNORM,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);

    // 将缓冲区数据复制到图像对象
    copyBufferToImage(
        stagingBuffer,
        texture3DImage,
        static_cast<uint32_t>(texWidth),
        static_cast<uint32_t>(texHeight),
        static_cast<uint32_t>(texDepth));

    // // 为了能够在着色器中开始从纹理图像中采样，我们需要最后一次转换，为着色器访问纹理图像做好准备。
    // // 将纹理图像转换为 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    // transitionImageLayout(
    //     texture3DImage,
    //     VK_FORMAT_R8G8_UNORM,
    //     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    //     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // 清理临时缓冲区
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);


    // ------------------------- generate mipmaps -------------------------
    VkCommandBuffer blitCommandBuffer = beginSingleTimeCommands(commandPool);


    std::cout << "mipLevels = " << mipLevels << std::endl;

    // transitioned to VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL while generating mipmaps
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = texture3DImage;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

    vkCmdPipelineBarrier(
        blitCommandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier);

    int32_t mipWidth = volumeRender->getDicomTags().voxelResolution[0];
    int32_t mipHeight = volumeRender->getDicomTags().voxelResolution[1];
    int32_t mipDepth = volumeRender->getDicomTags().voxelResolution[2];

    // Copy down mips from n-1 to n
    for (uint32_t i = 1; i < mipLevels; i++) {
        // Transition current mip level to transfer dest
        barrier.subresourceRange.baseMipLevel = i;
        barrier.subresourceRange.levelCount = 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        vkCmdPipelineBarrier(
            blitCommandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

        VkImageBlit imageBlit{};

        // Source
        imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // 颜色位
        imageBlit.srcSubresource.layerCount = 1; // 图像层数
        imageBlit.srcSubresource.mipLevel = i - 1; // 图像mip级别
        imageBlit.srcOffsets[1].x = int32_t(mipWidth); // 图像宽度
        imageBlit.srcOffsets[1].y = int32_t(mipHeight); // 图像高度
        imageBlit.srcOffsets[1].z = int32_t(mipDepth); // 图像深度

        // Destination
        imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // 颜色位
        imageBlit.dstSubresource.layerCount = 1; // 图像层数
        imageBlit.dstSubresource.mipLevel = i; // 图像mip级别
        imageBlit.dstOffsets[1].x = mipWidth > 1 ? int32_t(mipWidth / 2) : 1; // 图像宽度
        imageBlit.dstOffsets[1].y = mipHeight > 1 ? int32_t(mipHeight / 2) : 1; // 图像高度
        imageBlit.dstOffsets[1].z = mipDepth > 1 ? int32_t(mipDepth / 2) : 1; // 图像深度

        // Blit from previous level
        vkCmdBlitImage(
            blitCommandBuffer,
            texture3DImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            texture3DImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &imageBlit,
            VK_FILTER_LINEAR);

        // Transition current mip level to transfer source for read in next iteration
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(
            blitCommandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
        if (mipDepth > 1) mipDepth /= 2;
    }

    // After the loop, all mip layers are in TRANSFER_SRC layout, so transition all to GENERAL for compute shader read access
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(
        blitCommandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier);

    // end recording command buffer
    endSingleTimeCommands(blitCommandBuffer, commandPool);

    maxMipLevels = mipLevels; // 记录最大的mipmap级别数
}

void VulkanApplication::createImage(    // 创建图像对象
    uint32_t width,
    uint32_t height,
    uint32_t depth,
    VkFormat format,
    VkImageType imageType, // 图像类型
    VkImageTiling tiling,   // 图像数据的布局
    VkImageUsageFlags usage,    // 图像用途
    VkMemoryPropertyFlags properties,   // 内存属性
    VkImage& image,
    VkDeviceMemory& imageMemory,
    VkSampleCountFlagBits numSamples,
    uint32_t mipLevels) {
    // 创建图像对象
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = imageType;   // 图像类型告诉 Vulkan 图像中的像素将采用哪种坐标系。可以创建一维、二维和三维图像。一维图像可用于存储数据数组或梯度，二维图像主要用于纹理，三维图像可用于存储体素体积等。
    imageInfo.extent.width = static_cast<uint32_t>(width);    // 图像的宽度和高度，以像素为单位
    imageInfo.extent.height = static_cast<uint32_t>(height);
    imageInfo.extent.depth = static_cast<uint32_t>(depth); // 图像的深度，对于 2D 图像，其值必须为 1
    imageInfo.mipLevels = mipLevels;    // 图像的 mipmap 级别数
    imageInfo.arrayLayers = 1;  // 图像的数组层数
    imageInfo.format = format;  // 图像数据的格式
    imageInfo.tiling = tiling;  // 图像数据的布局
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;    // 图像数据的初始布局。vk_image_layout_undefined：GPU 无法使用，第一次转换将丢弃像素。vk_image_layout_preinitialized：GPU 无法使用，第一次转换不会丢弃像素。
    imageInfo.usage = usage;    // 图像用途 

    // 图像对象可以是单个图像，也可以是图像数组。图像数组可以用于存储多幅图像，它们共享相同的格式和大小。
    // 例如，我们可以使用单个图像数组来存储立方体贴图中的六个面。
    // arrayLayers 成员指定图像数组中的图像数量。如果我们要创建一个立方体贴图，那么这个值应该设置为 6。
    // 如果我们要创建一个 2D 数组纹理，那么这个值应该设置为数组中的图像数量。
    imageInfo.arrayLayers = 1;

    imageInfo.samples = numSamples;   // 采样数，指定多重采样纹理的采样数，通常为 1，最大为 VK_SAMPLE_COUNT_64_BIT
    imageInfo.flags = 0;    // 图像创建标志，可以用于指定图像被视为稀疏图像或图像立方体数组等特殊类型。如果您使用三维纹理来制作体素地形，那么您可以使用它来避免分配内存来存储大量的 "空气 "值

    if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;  // 获取图像内存需求
    vkGetImageMemoryRequirements(device, image, &memRequirements);

    // 分配图像内存
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;    // 分配的内存大小
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties); // 分配的内存类型

    if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {   // 分配内存
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(device, image, imageMemory, 0);    // 将内存绑定到图像对象

}

void VulkanApplication::createBackposAttachmentImage() {
    // 创建图像对象
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;   // 图像类型告诉 Vulkan 图像中的像素将采用哪种坐标系。可以创建一维、二维和三维图像。一维图像可用于存储数据数组或梯度，二维图像主要用于纹理，三维图像可用于存储体素体积等。
    imageInfo.extent.width = swapChainExtent.width;    // 图像的宽度和高度，以像素为单位
    imageInfo.extent.height = swapChainExtent.height;
    imageInfo.extent.depth = 1; // 图像的深度，对于 2D 图像，其值必须为 1
    imageInfo.mipLevels = 1;    // 图像的 mipmap 级别数
    imageInfo.arrayLayers = 1;  // 图像的数组层数
    imageInfo.format = VK_FORMAT_R16G16B16A16_SFLOAT;  // 图像数据的格式
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;  // 图像数据的布局
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;    // 图像数据的初始布局。vk_image_layout_undefined：GPU 无法使用，第一次转换将丢弃像素。vk_image_layout_preinitialized：GPU 无法使用，第一次转换不
    imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;    // 图像用途

    // 图像对象可以是单个图像，也可以是图像数组。图像数组可以用于存储多幅图像，它们共享相同的格式和大小。
    // 例如，我们可以使用单个图像数组来存储立方体贴图中的六个面。
    // arrayLayers 成员指定图像数组中的图像数量。如果我们要创建一个立方体贴图，那么这个值应该设置为 6。
    // 如果我们要创建一个 2D 数组纹理，那么这个值应该设置为数组中的图像数量。
    imageInfo.arrayLayers = 1;

    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;   // 采样数，指定多重采样纹理的采样数，通常为 1，最大为 VK_SAMPLE_COUNT_64_BIT
    imageInfo.flags = 0;    // 图像创建标志，可以用于指定图像被视为稀疏图像或图像立方体数组等特殊类型。如果您使用三维纹理来制作体素地形，那么您可以使用它来避免分配内存来存储大量的 "空气 "值

    if (vkCreateImage(device, &imageInfo, nullptr, &backFaceImage) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;  // 获取图像内存需求
    vkGetImageMemoryRequirements(device, backFaceImage, &memRequirements);

    // 分配图像内存
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;    // 分配的内存大小
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT); // 分配的内存类型

    if (vkAllocateMemory(device, &allocInfo, nullptr, &backFaceImageMemory) != VK_SUCCESS) {   // 分配内存
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(device, backFaceImage, backFaceImageMemory, 0);    // 将内存绑定到图像对象

}

void VulkanApplication::createTextureImageView() {
    backFaceImageView = createImageView(backFaceImage, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_VIEW_TYPE_2D);

    // 创建纹理的图像视图
    // textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_VIEW_TYPE_2D);
    texture3DImageView = createImageView(texture3DImage, VK_FORMAT_R8G8_UNORM, VK_IMAGE_VIEW_TYPE_3D, maxMipLevels);
    lutImageView = createImageView(lutImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_VIEW_TYPE_1D);
}


VkImageView VulkanApplication::createImageView(VkImage image, VkFormat format, VkImageViewType viewType, uint32_t mipLevels) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO; // 指定结构体类型
    viewInfo.image = image; // 指定图像对象
    viewInfo.viewType = viewType;    // 指定图像视图类型
    viewInfo.format = format;   // 指定图像数据的格式
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // 指定图像的哪些方面将受到屏障的影响。我们使用颜色位，因为我们只关心颜色数据
    viewInfo.subresourceRange.baseMipLevel = 0;    // 指定图像的哪些 mipmap 级别将受到屏障的影响。我们将其设置为 0，以便它可以影响所有级别
    viewInfo.subresourceRange.levelCount = mipLevels; // 指定图像的 mipmap 级别数量
    viewInfo.subresourceRange.baseArrayLayer = 0; // 指定图像的哪些数组层将受到屏障的影响。我们将其设置为 0，以便它可以影响所有层
    viewInfo.subresourceRange.layerCount = 1;  // 指定图像的数组层数量

    VkImageView imageView;
    if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {   // 创建图像视图
        throw std::runtime_error("failed to create texture image view!");
    }

    return imageView;
}


void VulkanApplication::createTextureSampler() {
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    // magFilter 和 minFilter 参数指定了在纹理被拉伸（放大）或压缩（缩小）时如何处理纹理像素。
    // 我们将它们都设置为 VK_FILTER_LINEAR，以便在纹理被拉伸时获得更好的效果。
    samplerInfo.magFilter = VK_FILTER_LINEAR;    // 放大时的采样方式 VK_FILTER_NEAREST
    samplerInfo.minFilter = VK_FILTER_LINEAR;    // 缩小时的采样方式
    // addressModeU、addressModeV 和 addressModeW 参数指定了对超出纹理范围的坐标进行采样时使用的策略。
    // 它们可以设置为以下值之一：
    // VK_SAMPLER_ADDRESS_MODE_REPEAT：重复纹理地址。这可能是最常见的模式，它在纹理坐标超出 [0,1] 范围时重复纹理图像。
    // VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT：纹理单元超出 [0,1] 范围时，镜像重复纹理图像。
    // VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE：纹理坐标被截断到 [0,1] 范围内，超出的部分将使用边缘的纹理颜色。
    // VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE：超出的坐标被截断到 [0,1] 范围内，然后使用镜像重复模式。
    // VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER：超出的坐标被截断到 [0,1] 范围内，然后使用用户指定的边框颜色。
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT; // U 轴的采样方式
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT; // V 轴的采样方式
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT; // W 轴的采样方式

    // 检索物理设备的属性，以确定支持的最大各向异性采样级别 
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    samplerInfo.anisotropyEnable = VK_TRUE;    // 启用各向异性过滤
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy; // 各向异性过滤的采样数

    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;    // 边框颜色

    // 如果该字段为 VK_TRUE，则可以简单地使用 [0, texWidth) 和 [0, texHeight) 范围内的坐标。
    // 如果该字段为 VK_FALSE，则在所有坐标轴上使用 [0, 1) 范围对纹理进行寻址。
    // 实际应用中几乎总是使用归一化坐标，因为这样就可以使用完全相同坐标的不同分辨率纹理。
    samplerInfo.unnormalizedCoordinates = VK_FALSE; // 指定坐标是否使用非归一化的方式

    samplerInfo.compareEnable = VK_FALSE;   // 指定是否启用比较操作。如果启用了比较功能，则会首先将像素与一个值进行比较，然后将比较结果用于过滤操作。这主要用于阴影贴图的百分比缩小过滤。
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS; // 指定比较操作的比较函数

    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR; // 指定 mipmap 的过滤方式
    samplerInfo.mipLodBias = 0.0f;  // 指定 mipmap 的 LOD 偏移量
    samplerInfo.minLod = 0.0f;  // 指定 mipmap 的最小 LOD
    samplerInfo.maxLod = maxMipLevels;  // 指定 mipmap 的最大 LOD

    // 采样器不会在任何地方引用 VkImage。采样器是一个独特的对象，它提供了一个从纹理中提取颜色的接口。它可以应用于任何图像，无论是一维、二维还是三维图像。
    if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {   // 创建纹理采样器
        throw std::runtime_error("failed to create texture sampler!");
    }

}

void VulkanApplication::transitionImageLayout(    // 图像布局转换
    VkImage image,
    VkFormat format,
    VkImageLayout oldLayout,
    VkImageLayout newLayout,
    uint32_t mipLevels) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(commandPool); // 开始记录命令缓冲区

    // VkImageMemoryBarrier（流水线屏障的一种）通常用于同步资源访问，例如确保在从缓冲区读取数据之前完成对缓冲区的写入
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;  // 旧布局
    barrier.newLayout = newLayout;  // 新布局
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;    // 图像所有权迁移时使用的队列族
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;    // 图像所有权迁移时使用的队列族
    barrier.image = image;  // 图像对象

    // 图像和 subresourceRange 指定了受影响的图像和图像的特定部分
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;  // 图像的哪些方面将受到屏障的影响。我们使用颜色位，因为我们只关心颜色数据
    barrier.subresourceRange.baseMipLevel = 0;  // 图像的哪些 mipmap 级别将受到屏障的影响。我们将其设置为 0，以便它可以影响所有级别
    barrier.subresourceRange.levelCount = mipLevels; // 图像的 mipmap 级别数量
    barrier.subresourceRange.baseArrayLayer = 0; // 图像的哪些数组层将受到屏障的影响。我们将其设置为 0，以便它可以影响所有层
    barrier.subresourceRange.layerCount = 1;  // 图像的数组层数量

    // 需要处理两个过渡：
    // 未定义 → 传输目的地：传输写入时不需要等待任何内容
    // 传输目标→着色器读取：着色器读取应等待传输写入，特别是片段着色器中的着色器读取，因为那是我们要使用纹理的地方。
    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        // 未定义 → 传输目的地：传输写入时不需要等待任何内容
        barrier.srcAccessMask = 0;  // 读取前访问图像的哪些操作。我们不关心旧数据，因为我们将完全覆盖它
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;    // 写入前访问图像的哪些操作。我们希望在传输写入之前等待内存访问，以便在写入之前不会覆盖它
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT; // 指定在屏障前执行的管线阶段
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;    // 指定在屏障后执行的管线阶段。VK_PIPELINE_STAGE_TRANSFER_BIT 并不是图形和计算流水线中的一个真正阶段。它更像是一个发生传输的伪阶段。
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        // 传输目标→着色器读取：着色器读取应等待传输写入，特别是片段着色器中的着色器读取，因为那是我们要使用纹理的地方。
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;    // 写入前访问图像的哪些操作。我们希望在传输写入之前等待内存访问，以便在写入之前不会覆盖它
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;   // 读取前访问图像的哪些操作。我们不关心旧数据，因为我们将完全覆盖它
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;    // 指定在屏障后执行的管线阶段
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_GENERAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT; // 在管线开始之前执行转换
        destinationStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;    // 计算着色器阶段等待布局转换完成
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_GENERAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;    // 计算着色器阶段等待布局转换完成
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_GENERAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    // for capture image
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    // 在命令缓冲区中执行图像布局转换
    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage,    // 指定在屏障前执行的管线阶段
        destinationStage,   // 指定在屏障后执行的管线阶段
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier);

    endSingleTimeCommands(commandBuffer, commandPool);   // !结束记录命令缓冲区,否则会报如下错误：
    /*
    validation layer: Validation Error: [ UNASSIGNED-CoreValidation-DrawState-InvalidImageLayout ]
    Object 0: handle = 0x24f88480270, type = VK_OBJECT_TYPE_COMMAND_BUFFER;
    Object 1: handle = 0xd897d90000000016, type = VK_OBJECT_TYPE_IMAGE; | MessageID = 0x4dae5635 | vkQueueSubmit(): pSubmits[0].pCommandBuffers[0]
    command buffer VkCommandBuffer 0x24f88480270[] expects VkImage 0xd897d90000000016[] (subresource: aspectMask 0x1 array layer 0, mip level 0) to
    be in layout VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL--instead, current layout is VK_IMAGE_LAYOUT_UNDEFINED.
     */
}

// 将缓冲区数据复制到图像对象，类似于copyBuffer()函数
void VulkanApplication::copyBufferToImage(
    VkBuffer buffer,
    VkImage image,
    uint32_t width,
    uint32_t height,
    uint32_t depth) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(commandPool); // 开始记录命令缓冲区

    // 将缓冲区数据复制到图像对象
    VkBufferImageCopy region{};
    region.bufferOffset = 0;    // 缓冲区偏移量指定像素值在缓冲区中的字节偏移量
    region.bufferRowLength = 0; // 缓冲区行长度
    region.bufferImageHeight = 0;   // 缓冲区图像高度

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;  // 图像的哪些方面将受到屏障的影响。我们使用颜色位，因为我们只关心颜色数据
    region.imageSubresource.mipLevel = 0;    // 图像的哪些 mipmap 级别将受到屏障的影响。我们将其设置为 0，以便它可以影响所有级别
    region.imageSubresource.baseArrayLayer = 0; // 图像的哪些数组层将受到屏障的影响。我们将其设置为 0，以便它可以影响所有层
    region.imageSubresource.layerCount = 1;  // 图像的数组层数量

    region.imageOffset = { 0, 0, 0 };   // 图像偏移量
    region.imageExtent = { width, height, depth };    // 图像范围

    // 将缓冲区数据复制到图像对象
    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, // 图像布局
        1, // regionCount
        &region); // pRegions

    endSingleTimeCommands(commandBuffer, commandPool);   // 结束记录命令缓冲区
}

VkCommandBuffer VulkanApplication::beginSingleTimeCommands(VkCommandPool cmdPool) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    // commandPool是用于分配命令缓冲区的命令池
    allocInfo.commandPool = cmdPool;
    // level参数指定分配的命令缓冲区是否是主要或辅助缓冲区。主要缓冲区可以被提交到队列中，但不能从其他命令缓冲区调用。
    // 辅助缓冲区可以从主缓冲区和其他辅助缓冲区调用。我们将使用主缓冲区来执行实际的数据传输操作。
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    // 分配命令缓冲区
    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    // 开始记录命令缓冲区
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    // flags参数指定如何使用命令缓冲区。我们希望使用它一次并在使用后立即返回。
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void VulkanApplication::endSingleTimeCommands(VkCommandBuffer commandBuffer, VkCommandPool cmdPool) {
    // 结束记录命令缓冲区
    vkEndCommandBuffer(commandBuffer);

    // 执行命令缓冲区
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    // commandBufferCount和pCommandBuffers参数指定要提交的命令缓冲区数量和指针。
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    // 我们在这里使用了 vkQueueSubmit，而不是 vkQueueWaitIdle，因为后者是同步的，而前者是异步的。
    // 这意味着 vkQueueSubmit 可以立即返回，而不是等待复制操作完成。
    // 我们将在稍后使用信号量来同步操作，以便在复制操作完成之前不会使用缓冲区。
    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    // 等待命令缓冲区执行完成
    vkQueueWaitIdle(graphicsQueue);

    // 释放命令缓冲区
    vkFreeCommandBuffers(device, cmdPool, 1, &commandBuffer);
}

void VulkanApplication::initImGui() {
    // 初始化 ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui::StyleColorsDark();

    // set fonts size
    // 获取当前的字体图谱
    ImGuiIO& io = ImGui::GetIO();
    ImFontAtlas* fonts = io.Fonts;

    // 设置字体的缩放因子（大小）
    float fontScale = 2.0f; // 调整为你想要的大小
    fonts->Clear(); // 清除默认字体
    io.FontGlobalScale = fontScale; // 设置全局缩放因子

    // init some imgui specefic resources
    createImGuiDescriptorPool();
    createImGuiRenderPass();
    createImGuiCommandPool();
    createImGuiCommandBuffers();
    createImGuiFramebuffers();
    createImGuiSyncObjects();

    // init imgui for vulkan
    ImGui_ImplGlfw_InitForVulkan(window, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = instance;
    init_info.PhysicalDevice = physicalDevice;
    init_info.Device = device;
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    init_info.QueueFamily = indices.graphicsFamily.value();
    init_info.Queue = graphicsQueue;
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = imguiDescriptorPool;
    init_info.Allocator = nullptr;
    init_info.MinImageCount = swapChainImages.size();
    init_info.ImageCount = swapChainImages.size();

    ImGui_ImplVulkan_Init(&init_info, imguiRenderPass);

    // upload fonts
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(imguiCommandPool);
    ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
    endSingleTimeCommands(commandBuffer, imguiCommandPool);
    ImGui_ImplVulkan_DestroyFontUploadObjects();

}

// Copied this code from DearImgui's setup:
// https://github.com/ocornut/imgui/blob/master/examples/example_glfw_vulkan/main.cpp
void VulkanApplication::createImGuiDescriptorPool() {
    VkDescriptorPoolSize pool_sizes[] = {
        {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}
    };
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
    pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;
    if (vkCreateDescriptorPool(device, &pool_info, nullptr, &imguiDescriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create imgui descriptor pool!");
    }
}

void VulkanApplication::createImGuiRenderPass() {
    // create an attachment description for the renderpass
    VkAttachmentDescription attachment = {};
    attachment.format = swapChainImageFormat;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    // !在渲染通道开始时，我们不希望图像中的内容被清除，因为我们要在上面绘制 Dear ImGui 的 UI。
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;// Need UI to be drawn on top of main
    // we keep the attachment stored when the renderpass ends
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    // we don't care about stencil
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // we don't know or care about the starting layout of the attachment
    attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    // after the renderpass ends, the image has to be on a layout ready for display
    attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // create an attachment reference for the renderpass
    VkAttachmentReference color_attachment = {};
    color_attachment.attachment = 0;
    color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // create a subpass description for the renderpass
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    // this subpass has 1 color attachment
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment;

    // Create a subpass dependency to synchronize our main and UI render passes
    // We want to render the UI after the geometry has been written to the framebuffer
    // so we need to configure a subpass dependency as such
    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0; // our subpass index is 0, since we're creating the only one
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    // we need to wait for the swapchain to finish reading from the image before we can access it
    dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // create the renderpass
    VkRenderPassCreateInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = 1;
    render_pass_info.pAttachments = &attachment;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;

    if (vkCreateRenderPass(device, &render_pass_info, nullptr, &imguiRenderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create imgui render pass!");
    }
}

void VulkanApplication::createImGuiCommandPool() {
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value(); // 指定队列族
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;  // 指定命令缓冲区的标志

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &imguiCommandPool) != VK_SUCCESS) { // 创建命令池
        throw std::runtime_error("failed to create command pool!");
    }
}

void VulkanApplication::createImGuiCommandBuffers() {
    // create a command buffer for max number of frames in flight
    imguiCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = imguiCommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)imguiCommandBuffers.size();

    if (vkAllocateCommandBuffers(device, &allocInfo, imguiCommandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate imgui command buffers!");
    }
}

void VulkanApplication::createImGuiFramebuffers() {
    // create a framebuffer for each swapchain image
    imguiFramebuffers.resize(swapChainImageViews.size());

    // create a framebuffer for each swapchain image
    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        VkImageView attachments[] = {
            swapChainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = imguiRenderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &imguiFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create imgui framebuffer!");
        }
    }
}

void VulkanApplication::createImGuiSyncObjects() {
    // fence and semaphores
    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // 创建时已经处于信号状态

    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    imguiInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    imguiFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateFence(device, &fenceCreateInfo, nullptr, &imguiInFlightFences[i]) != VK_SUCCESS) { // 创建栅栏
            throw std::runtime_error("failed to create synchronization object for a frame!");
        }
        if (vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &imguiFinishedSemaphores[i]) != VK_SUCCESS) { // 创建信号量
            throw std::runtime_error("failed to create synchronization object for a frame!");
        }
    }
}

void VulkanApplication::drawImGui() {
    // start the Dear ImGui frame
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // imgui state
    static bool showDemoWindow = false;
    static bool showAnotherWindow = false;
    // clear color
    static ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // 1. Show a simple window
    // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets automatically appears in a window called "Debug"
    {
        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin("Dicom Param Control");
        ImGui::Text(" %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
            ImGui::GetIO().Framerate);

        ImGui::SliderFloat("alphaCorrection", &volumeRender->dicomParamControl.alphaCorrection, 0.0f, 256.0f);
        ImGui::SliderInt("steps", &volumeRender->dicomParamControl.steps, 0, 1500);
        ImGui::SliderFloat("stepLength", &volumeRender->dicomParamControl.stepLength, 0.0f, 0.02f);
        if(ImGui::SliderFloat("WindowWidth", &volumeRender->dicomParamControl.windowWidth, 0.0f, 540.0f)){
            computeResources.isComplete = false;
        }
        if(ImGui::SliderFloat("WindowCenter", &volumeRender->dicomParamControl.windowCenter, 50.0f, 800.0f)){
            computeResources.isComplete = false;
        }
        ImGui::SliderFloat("glow", &volumeRender->dicomParamControl.glow, 0.0f, 30.0f);

        // add button ,once clicked, add the counter
        if (ImGui::Button("Capture Image")) {
            ++currentCaptureID;
            isNeedCapture = true;
        }

        // ImGui::SliderInt("method", &volumeRender->dicomParamControl.renderMethod, 0, 3);

        ImGui::Checkbox("Demo Window", &showDemoWindow);      // Edit bools storing our window open/close state
        ImGui::Checkbox("Another Window", &showAnotherWindow);
        ImGui::End();
    }

    // 2. Show another simple window, this time using an explicit Begin/End pair
    if (showAnotherWindow) {
        ImGui::Begin("Another Window", &showAnotherWindow);
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me")) {
            showAnotherWindow = false;
        }
        ImGui::End();
    }

    // 3. Show the ImGui demo window. Most of the sample code is in ImGui::ShowDemoWindow()
    if (showDemoWindow) {
        ImGui::ShowDemoWindow(&showDemoWindow);
    }

    // Rendering
    ImGui::Render();
}

void VulkanApplication::recordImGuiCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    // record imgui commands into command buffer
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    // we're only submitting the ui commands once, so we use the one time submit bit
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording imgui command buffer!");
    }

    // start the renderpass
    VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = imguiRenderPass;
    renderPassInfo.framebuffer = imguiFramebuffers[imageIndex];
    renderPassInfo.renderArea.extent.width = swapChainExtent.width;
    renderPassInfo.renderArea.extent.height = swapChainExtent.height;
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    // start the renderpass
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    // record imgui draw data and draw funcs into command buffer
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

    // end the renderpass
    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record imgui command buffer!");
    }
}

void VulkanApplication::prepareTextureTarget() {
    VkFormatProperties formatProperties;

    // Get device properties for the requested texture format
    vkGetPhysicalDeviceFormatProperties(physicalDevice, VK_FORMAT_R8G8B8A8_UNORM, &formatProperties);
    // Check if requested image format supports image storage operations
    assert(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT);

    // Prepare blit target texture
    int32_t mipWidth, mipHeight, mipDepth;
    if (isHighResolution) {
        mipWidth = 512;
        mipHeight = 512;
        mipDepth = 512;
    }
    else if(isLowResolution){
        mipWidth = lowResVal;
        mipHeight = lowResVal;
        mipDepth = lowResVal;
    } 
    else{
        mipWidth = volumeRender->getDicomTags().voxelResolution[0];
        mipHeight = volumeRender->getDicomTags().voxelResolution[1];
        mipDepth = volumeRender->getDicomTags().voxelResolution[2];
    } 
    VkImageCreateInfo imageCreateInfo = {};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_3D;
    imageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageCreateInfo.extent = { static_cast<unsigned int>(mipWidth)
        , static_cast<unsigned int>(mipHeight)
        , static_cast<unsigned int>(mipDepth) };
    textureTarget.mipLevels = static_cast<uint32_t>(std::floor(
        std::log2(
            std::max(
                std::max(mipWidth, mipHeight),
                mipDepth)
        ))) + 1;  // mipmap 级别  
    imageCreateInfo.mipLevels = textureTarget.mipLevels;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    // Image will be sampled in the fragment shader and used as storage target in the compute shader
    imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    imageCreateInfo.flags = 0;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    // If compute and graphics queue family indices differ, we create an image that can be shared between them
    // This can result in worse performance than exclusive sharing mode, but save some synchronization to keep the sample simple
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    if (indices.graphicsFamily.value() != indices.computeFamily.value()) {
        uint32_t queueFamilyIndices[] = { (uint32_t)indices.graphicsFamily.value(), (uint32_t)indices.computeFamily.value() };

        imageCreateInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
        imageCreateInfo.queueFamilyIndexCount = 2;
        imageCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
    }

    if (vkCreateImage(device, &imageCreateInfo, nullptr, &textureTarget.image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image!");
    }

    VkMemoryRequirements memReqs;
    vkGetImageMemoryRequirements(device, textureTarget.image, &memReqs);

    VkMemoryAllocateInfo memAllocInfo{};
    memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAllocInfo.allocationSize = memReqs.size;
    // Request a host visible memory type that can be used to copy our data do
    memAllocInfo.memoryTypeIndex = findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(device, &memAllocInfo, nullptr, &textureTarget.memory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate memory!");
    }

    if (vkBindImageMemory(device, textureTarget.image, textureTarget.memory, 0) != VK_SUCCESS) {
        throw std::runtime_error("failed to bind image memory!");
    }

    // 将图像布局转换为VK_IMAGE_LAYOUT_GENERAL
    textureTarget.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    transitionImageLayout(textureTarget.image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, textureTarget.mipLevels);

    // Create sampler
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR; // 放大过滤器
    samplerInfo.minFilter = VK_FILTER_LINEAR; // 缩小过滤器
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR; // mipmap模式
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; // U轴寻址模式
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; // V轴寻址模式
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; // W轴寻址模式
    samplerInfo.mipLodBias = 0.0f; // mipmap偏移
    samplerInfo.compareOp = VK_COMPARE_OP_NEVER; // 比较操作
    samplerInfo.minLod = 0.0f; // 最小LOD
    samplerInfo.maxLod = static_cast<float>(textureTarget.mipLevels); // 最大LOD
    samplerInfo.maxAnisotropy = 1.0f; // 各向异性过滤
    samplerInfo.anisotropyEnable = VK_FALSE; // 是否启用各向异性过滤
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK; // 边框颜色 
    samplerInfo.unnormalizedCoordinates = VK_FALSE; // 是否使用非归一化坐标

    if (vkCreateSampler(device, &samplerInfo, nullptr, &textureTarget.sampler) != VK_SUCCESS) { // 创建采样器
        throw std::runtime_error("failed to create texture sampler!");
    }
    // textureTarget.sampler = textureSampler;

    // Create image view
    textureTarget.imageView = createImageView(textureTarget.image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_VIEW_TYPE_3D, textureTarget.mipLevels);
}

void VulkanApplication::prepareCompute()
{
    computeResources.isComplete = false;

    // 创建纹理目标
    prepareTextureTarget();

    // Get a compute queue from the device
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    vkGetDeviceQueue(device, indices.computeFamily.value(), 0, &computeResources.queue);

    // Create compute pipeline
    // Compute pipelines are created separate from graphics pipelines even if they use the same queue
    VkDescriptorSetLayoutBinding inputImageLayoutBinding{};
    inputImageLayoutBinding.binding = 0; // 绑定点
    inputImageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; // 描述符类型
    inputImageLayoutBinding.descriptorCount = 1; // 描述符数量
    inputImageLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT; // 着色器阶段
    inputImageLayoutBinding.pImmutableSamplers = nullptr; // 不使用采样器

    VkDescriptorSetLayoutBinding outputImageLayoutBinding{};
    outputImageLayoutBinding.binding = 1; // 绑定点
    outputImageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE; // 描述符类型
    outputImageLayoutBinding.descriptorCount = 1; // 描述符数量
    outputImageLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT; // 着色器阶段
    outputImageLayoutBinding.pImmutableSamplers = nullptr; // 不使用采样器

    VkDescriptorSetLayoutBinding dicomUboLayoutBinding{};
    dicomUboLayoutBinding.binding = 2;
    dicomUboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dicomUboLayoutBinding.descriptorCount = 1;
    dicomUboLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    dicomUboLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding lutLayoutBinding{};
    lutLayoutBinding.binding = 3;
    lutLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    lutLayoutBinding.descriptorCount = 1;
    lutLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    lutLayoutBinding.pImmutableSamplers = nullptr;

    std::array<VkDescriptorSetLayoutBinding, 4> bindings = {
        inputImageLayoutBinding,
        outputImageLayoutBinding,
        dicomUboLayoutBinding,
        lutLayoutBinding
    };

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size()); // 描述符绑定数量
    layoutInfo.pBindings = bindings.data(); // 描述符绑定

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &computeResources.descriptorSetLayout) != VK_SUCCESS) { // 创建描述符集布局
        throw std::runtime_error("failed to create descriptor set layout!");
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1; // 描述符集布局数量
    pipelineLayoutInfo.pSetLayouts = &computeResources.descriptorSetLayout; // 描述符集布局

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &computeResources.pipelineLayout) != VK_SUCCESS) { // 创建管线布局
        throw std::runtime_error("failed to create pipeline layout!");
    }

    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, computeResources.descriptorSetLayout); // 描述符集布局
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool; // 描述符池
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT); // 描述符集数量
    allocInfo.pSetLayouts = layouts.data(); // 描述符集布局

    computeResources.descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(device, &allocInfo, computeResources.descriptorSets.data()) != VK_SUCCESS) { // 分配描述符集
        throw std::runtime_error("failed to allocate computeResources descriptor sets!");
    }

    // 更新描述符集
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        // 输入的是3D的纹理
        VkDescriptorImageInfo inputImageInfo{};
        inputImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // 图像布局
        inputImageInfo.imageView = texture3DImageView; // 图像视图
        inputImageInfo.sampler = textureTarget.sampler; // 纹理采样器

        VkWriteDescriptorSet inputImageDescriptorWrite{};
        inputImageDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        inputImageDescriptorWrite.dstSet = computeResources.descriptorSets[i]; // 目标描述符集
        inputImageDescriptorWrite.dstBinding = 0; // 目标绑定点
        inputImageDescriptorWrite.dstArrayElement = 0; // 目标数组元素
        inputImageDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; // 描述符类型
        inputImageDescriptorWrite.descriptorCount = 1; // 描述符数量
        inputImageDescriptorWrite.pBufferInfo = nullptr; // 缓冲区信息
        inputImageDescriptorWrite.pImageInfo = &inputImageInfo; // 图像信息

        // 输出的是3D的纹理
        VkDescriptorImageInfo outputImageInfo{};
        outputImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL; // 图像布局
        outputImageInfo.imageView = textureTarget.imageView; // 图像视图
        outputImageInfo.sampler = textureTarget.sampler; // 纹理采样器

        VkWriteDescriptorSet outputImageDescriptorWrite{};
        outputImageDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        outputImageDescriptorWrite.dstSet = computeResources.descriptorSets[i]; // 目标描述符集
        outputImageDescriptorWrite.dstBinding = 1; // 目标绑定点
        outputImageDescriptorWrite.dstArrayElement = 0; // 目标数组元素
        outputImageDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE; // 描述符类型
        outputImageDescriptorWrite.descriptorCount = 1; // 描述符数量
        outputImageDescriptorWrite.pBufferInfo = nullptr; // 缓冲区信息
        outputImageDescriptorWrite.pImageInfo = &outputImageInfo; // 图像信息

        VkDescriptorBufferInfo dicomBufferInfo{};
        dicomBufferInfo.buffer = dicomUniformBuffers[i]; // 缓冲区
        dicomBufferInfo.offset = 0; // 偏移量
        dicomBufferInfo.range = sizeof(DicomUniformBufferObject); // 范围

        VkWriteDescriptorSet dicomBufferDescriptorWrite{};
        dicomBufferDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        dicomBufferDescriptorWrite.dstSet = computeResources.descriptorSets[i]; // 目标描述符集
        dicomBufferDescriptorWrite.dstBinding = 2; // 目标绑定点
        dicomBufferDescriptorWrite.dstArrayElement = 0; // 目标数组元素
        dicomBufferDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // 描述符类型
        dicomBufferDescriptorWrite.descriptorCount = 1; // 描述符数量
        dicomBufferDescriptorWrite.pBufferInfo = &dicomBufferInfo; // 缓冲区信息
        dicomBufferDescriptorWrite.pImageInfo = nullptr; // 图像信息

        VkDescriptorImageInfo lutImageInfo{};
        lutImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // 图像布局
        lutImageInfo.imageView = lutImageView; // 图像视图
        lutImageInfo.sampler = textureSampler; // 纹理采样器

        VkWriteDescriptorSet lutImageDescriptorWrite{};
        lutImageDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        lutImageDescriptorWrite.dstSet = computeResources.descriptorSets[i]; // 目标描述符集
        lutImageDescriptorWrite.dstBinding = 3; // 目标绑定点
        lutImageDescriptorWrite.dstArrayElement = 0; // 目标数组元素
        lutImageDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; // 描述符类型
        lutImageDescriptorWrite.descriptorCount = 1; // 描述符数量
        lutImageDescriptorWrite.pBufferInfo = nullptr; // 缓冲区信息
        lutImageDescriptorWrite.pImageInfo = &lutImageInfo; // 图像信息

        std::array<VkWriteDescriptorSet, 4> computeWrites = {
            inputImageDescriptorWrite,
            outputImageDescriptorWrite,
            dicomBufferDescriptorWrite,
            lutImageDescriptorWrite
        };

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(computeWrites.size()), computeWrites.data(), 0, nullptr); // 更新描述符集
    }

    // Create compute shader pipelines
    VkComputePipelineCreateInfo computePipelineCreateInfo{};
    computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    computePipelineCreateInfo.layout = computeResources.pipelineLayout; // 管线布局

    // Create shader modules
    VkShaderModule computeShaderModule = createShaderModule(GENERATEEXTINCTIONCOEF_COMP);
    computePipelineCreateInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    computePipelineCreateInfo.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT; // 着色器阶段
    computePipelineCreateInfo.stage.module = computeShaderModule; // 着色器模块
    computePipelineCreateInfo.stage.pName = "main"; // 着色器入口函数名称

    VkPipeline pipeline;
    if (vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &pipeline) != VK_SUCCESS) { // 创建管线
        throw std::runtime_error("failed to create compute pipeline!");
    }
    computeResources.pipelines.push_back(pipeline);

    // Destroy shader modules
    vkDestroyShaderModule(device, computeShaderModule, nullptr);

    // create command pool
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = indices.computeFamily.value(); // 指定队列族
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;  // 指定命令缓冲区的标志

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &computeResources.commandPool) != VK_SUCCESS) { // 创建命令池
        throw std::runtime_error("failed to create command pool!");
    }

    // Create a command buffer for compute operations
    computeResources.commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo cmdBufAllocateInfo{};
    cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdBufAllocateInfo.commandPool = computeResources.commandPool; // 命令池
    cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // 命令缓冲区级别
    cmdBufAllocateInfo.commandBufferCount = (uint32_t)computeResources.commandBuffers.size(); // 命令缓冲区数量

    if (vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, computeResources.commandBuffers.data()) != VK_SUCCESS) { // 分配命令缓冲区
        throw std::runtime_error("failed to allocate command buffers!");
    }

    // fence and semaphores
    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // 创建时已经处于信号状态

    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    computeResources.inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    computeResources.finishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    computeResources.finishedGenMipmapSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateFence(device, &fenceCreateInfo, nullptr, &computeResources.inFlightFences[i]) != VK_SUCCESS) { // 创建栅栏
            throw std::runtime_error("failed to create synchronization object for a frame!");
        }
        if (vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &computeResources.finishedSemaphores[i]) != VK_SUCCESS) { // 创建信号量
            throw std::runtime_error("failed to create synchronization object for a frame!");
        }
        if (vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &computeResources.finishedGenMipmapSemaphores[i]) != VK_SUCCESS) { // 创建信号量
            throw std::runtime_error("failed to create synchronization object for a frame!");
        }
    }
}

void VulkanApplication::recordComputeCommandBuffer(uint32_t currentFrame) {
    // Flush the queue if we're rebuilding the command buffer after a pipeline change to ensure it's not currently in use
    vkQueueWaitIdle(computeResources.queue);

    // Create a command buffer for compute operations
    VkCommandBufferBeginInfo cmdBufBeginInfo{};
    cmdBufBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(computeResources.commandBuffers[currentFrame], &cmdBufBeginInfo) != VK_SUCCESS) { // 开始记录命令缓冲区
        throw std::runtime_error("failed to begin command buffer!");
    }

    // Bind compute pipeline
    vkCmdBindPipeline(computeResources.commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_COMPUTE, computeResources.pipelines[0]);

    // Bind descriptor sets
    vkCmdBindDescriptorSets(computeResources.commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_COMPUTE, computeResources.pipelineLayout, 0, 1, &computeResources.descriptorSets[currentFrame], 0, 0);

    // Dispatch the compute job
    if (isHighResolution) vkCmdDispatch(computeResources.commandBuffers[currentFrame], 512 / 8, 512 / 8, 512 / 8);
    else if (isLowResolution) vkCmdDispatch(computeResources.commandBuffers[currentFrame], lowResVal / 8, lowResVal / 8, lowResVal / 8);
    else vkCmdDispatch(computeResources.commandBuffers[currentFrame], volumeRender->getDicomTags().voxelResolution[0] / 8, volumeRender->getDicomTags().voxelResolution[1] / 8, volumeRender->getDicomTags().voxelResolution[2] / 8);


    // end recording command buffer
    if (vkEndCommandBuffer(computeResources.commandBuffers[currentFrame]) != VK_SUCCESS) { // 结束记录命令缓冲区
        throw std::runtime_error("failed to record command buffer!");
    }

}

void VulkanApplication::prepareGaussianCompute() {
    // Get a compute queue from the device
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    vkGetDeviceQueue(device, indices.computeFamily.value(), 0, &gaussianComputeResources.queue);

    // Create compute pipeline
    // Compute pipelines are created separate from graphics pipelines even if they use the same queue
    VkDescriptorSetLayoutBinding inputImageLayoutBinding{};
    inputImageLayoutBinding.binding = 0; // 绑定点
    inputImageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; // 描述符类型
    inputImageLayoutBinding.descriptorCount = 1; // 描述符数量
    inputImageLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT; // 着色器阶段
    inputImageLayoutBinding.pImmutableSamplers = nullptr; // 不使用采样器

    VkDescriptorSetLayoutBinding outputImageLayoutBinding{};
    outputImageLayoutBinding.binding = 1; // 绑定点
    outputImageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE; // 描述符类型
    outputImageLayoutBinding.descriptorCount = 1; // 描述符数量
    outputImageLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT; // 着色器阶段
    outputImageLayoutBinding.pImmutableSamplers = nullptr; // 不使用采样器

    VkDescriptorSetLayoutBinding dicomUboLayoutBinding{};
    dicomUboLayoutBinding.binding = 2;
    dicomUboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dicomUboLayoutBinding.descriptorCount = 1;
    dicomUboLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    dicomUboLayoutBinding.pImmutableSamplers = nullptr;

    std::array<VkDescriptorSetLayoutBinding, 3> bindings = {
        inputImageLayoutBinding,
        outputImageLayoutBinding,
        dicomUboLayoutBinding
    };

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size()); // 描述符绑定数量
    layoutInfo.pBindings = bindings.data(); // 描述符绑定

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &gaussianComputeResources.descriptorSetLayout) != VK_SUCCESS) { // 创建描述符集布局
        throw std::runtime_error("failed to create descriptor set layout!");
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1; // 描述符集布局数量
    pipelineLayoutInfo.pSetLayouts = &gaussianComputeResources.descriptorSetLayout; // 描述符集布局

    //setup push constants
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT; // 着色器阶段
    pushConstantRange.offset = 0; // 偏移量
    pushConstantRange.size = sizeof(GenGaussianMMPushConstants); // 大小

    pipelineLayoutInfo.pushConstantRangeCount = 1; // 推送常量范围数量
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange; // 推送常量范围

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &gaussianComputeResources.pipelineLayout) != VK_SUCCESS) { // 创建管线布局
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool; // 描述符池
    allocInfo.descriptorSetCount = 1; // 描述符集数量
    allocInfo.pSetLayouts = &gaussianComputeResources.descriptorSetLayout; // 描述符集布局

    if (vkAllocateDescriptorSets(device, &allocInfo, &gaussianComputeResources.descriptorSet) != VK_SUCCESS) { // 分配描述符集
        throw std::runtime_error("failed to allocate computeResources descriptor sets!");
    }

    // Create compute shader pipelines
    VkComputePipelineCreateInfo computePipelineCreateInfo{};
    computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    computePipelineCreateInfo.layout = gaussianComputeResources.pipelineLayout; // 管线布局

    // Create shader modules
    VkShaderModule computeShaderModule = createShaderModule(GENERATEEXTINCTIONCOEFMIPMAP_COMP);
    computePipelineCreateInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    computePipelineCreateInfo.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT; // 着色器阶段
    computePipelineCreateInfo.stage.module = computeShaderModule; // 着色器模块
    computePipelineCreateInfo.stage.pName = "main"; // 着色器入口函数名称

    VkPipeline pipeline;
    if (vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &pipeline) != VK_SUCCESS) { // 创建管线
        throw std::runtime_error("failed to create compute pipeline!");
    }
    gaussianComputeResources.pipelines.push_back(pipeline);

    // Destroy shader modules
    vkDestroyShaderModule(device, computeShaderModule, nullptr);

    // create command pool
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = indices.computeFamily.value(); // 指定队列族
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;  // 指定命令缓冲区的标志

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &gaussianComputeResources.commandPool) != VK_SUCCESS) { // 创建命令池
        throw std::runtime_error("failed to create command pool!");
    }

    // Create a command buffer for compute operations
    VkCommandBufferAllocateInfo cmdBufAllocateInfo{};
    cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdBufAllocateInfo.commandPool = gaussianComputeResources.commandPool; // 命令池
    cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // 命令缓冲区级别
    cmdBufAllocateInfo.commandBufferCount = 1; // 命令缓冲区数量

    if (vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, &gaussianComputeResources.commandBuffer) != VK_SUCCESS) { // 分配命令缓冲区
        throw std::runtime_error("failed to allocate command buffers!");
    }

    // fence and semaphores
    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = 0; // 标志位

    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;


    if (vkCreateFence(device, &fenceCreateInfo, nullptr, &gaussianComputeResources.inFlightFence) != VK_SUCCESS) { // 创建fence
        throw std::runtime_error("gaussianComputeResources failed to create fence!");
    }
    if (vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &gaussianComputeResources.finishedSemaphore) != VK_SUCCESS) { // 创建信号量
        throw std::runtime_error("gaussianComputeResources failed to create semaphores!");
    }

}

void VulkanApplication::recordGenExtCoffMipmaps(uint32_t currentFrame) {
    vkQueueWaitIdle(computeResources.queue);

    // Check if image format supports linear blitting
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(physicalDevice, VK_FORMAT_R8G8B8A8_UNORM, &formatProperties);

    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        throw std::runtime_error("texture image format does not support linear blitting!");
    }

    VkCommandBuffer blitCommandBuffer = beginSingleTimeCommands(commandPool);

    // std::cout << "mipLevels = " << textureTarget.mipLevels << std::endl;

    // transitioned to VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL while generating mipmaps
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = textureTarget.image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = textureTarget.mipLevels;
    barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

    vkCmdPipelineBarrier(
        blitCommandBuffer,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier);

    int32_t mipWidth, mipHeight, mipDepth;
    if (isHighResolution) {
        mipWidth = 512;
        mipHeight = 512;
        mipDepth = 512;
    }
    else if(isLowResolution){
        mipWidth = lowResVal;
        mipHeight = lowResVal;
        mipDepth = lowResVal;
    }
    else{
        mipWidth = volumeRender->getDicomTags().voxelResolution[0];
        mipHeight = volumeRender->getDicomTags().voxelResolution[1];
        mipDepth = volumeRender->getDicomTags().voxelResolution[2];
    }

    // Copy down mips from n-1 to n
    for (uint32_t i = 1; i < textureTarget.mipLevels; i++) {
        // Transition current mip level to transfer dest
        barrier.subresourceRange.baseMipLevel = i;
        barrier.subresourceRange.levelCount = 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.srcAccessMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
        barrier.dstAccessMask = VK_PIPELINE_STAGE_TRANSFER_BIT;

        vkCmdPipelineBarrier(
            blitCommandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

        VkImageBlit imageBlit{};

        // Source
        imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // 颜色位
        imageBlit.srcSubresource.layerCount = 1; // 图像层数
        imageBlit.srcSubresource.mipLevel = i - 1; // 图像mip级别
        imageBlit.srcOffsets[1].x = int32_t(mipWidth); // 图像宽度
        imageBlit.srcOffsets[1].y = int32_t(mipHeight); // 图像高度
        imageBlit.srcOffsets[1].z = int32_t(mipDepth); // 图像深度

        // Destination
        imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // 颜色位
        imageBlit.dstSubresource.layerCount = 1; // 图像层数
        imageBlit.dstSubresource.mipLevel = i; // 图像mip级别
        imageBlit.dstOffsets[1].x = mipWidth > 1 ? int32_t(mipWidth / 2) : 1; // 图像宽度
        imageBlit.dstOffsets[1].y = mipHeight > 1 ? int32_t(mipHeight / 2) : 1; // 图像高度
        imageBlit.dstOffsets[1].z = mipDepth > 1 ? int32_t(mipDepth / 2) : 1; // 图像深度

        // Blit from previous level
        vkCmdBlitImage(
            blitCommandBuffer,
            textureTarget.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            textureTarget.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &imageBlit,
            VK_FILTER_LINEAR);

        // Transition current mip level to transfer source for read in next iteration
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(
            blitCommandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
        if (mipDepth > 1) mipDepth /= 2;
    }

    // After the loop, all mip layers are in TRANSFER_SRC layout, so transition all to GENERAL for compute shader read access
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = textureTarget.mipLevels;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(
        blitCommandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier);

    // end recording command buffer
    endSingleTimeCommands(blitCommandBuffer, commandPool);
}

void VulkanApplication::recordGenGaussianMipmaps() {
    vkQueueWaitIdle(computeResources.queue);

    int32_t mipWidth, mipHeight, mipDepth;
    if (isHighResolution) {
        mipWidth = 512;
        mipHeight = 512;
        mipDepth = 512;
    }
    else if(isLowResolution){
        mipWidth = lowResVal;
        mipHeight = lowResVal;
        mipDepth = lowResVal;
    }
    else{
        mipWidth = volumeRender->getDicomTags().voxelResolution[0];
        mipHeight = volumeRender->getDicomTags().voxelResolution[1];
        mipDepth = volumeRender->getDicomTags().voxelResolution[2];
    }
    for (size_t currentLevel = 1; currentLevel < textureTarget.mipLevels; currentLevel++) {

        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
        if (mipDepth > 1) mipDepth /= 2;

        if (mipWidth <= 1 && mipHeight <= 1 && mipDepth <= 1) break;

        // create previous mip level image view
        VkImageViewCreateInfo inputViewInfo = {};
        inputViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        inputViewInfo.image = textureTarget.image;
        inputViewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
        inputViewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
        inputViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        inputViewInfo.subresourceRange.baseMipLevel = 0;
        inputViewInfo.subresourceRange.levelCount = textureTarget.mipLevels;
        inputViewInfo.subresourceRange.baseArrayLayer = 0;
        inputViewInfo.subresourceRange.layerCount = 1;

        VkImageView inputImageView;
        if (vkCreateImageView(device, &inputViewInfo, nullptr, &inputImageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create input texture image view!");
        }

        // create current mip level image view
        VkImageViewCreateInfo outputViewInfo = {};
        outputViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        outputViewInfo.image = textureTarget.image;
        outputViewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
        outputViewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
        outputViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        outputViewInfo.subresourceRange.baseMipLevel = currentLevel; // 目标mipmap层级
        outputViewInfo.subresourceRange.levelCount = 1; // 我们只关注一个层级
        outputViewInfo.subresourceRange.baseArrayLayer = 0;
        outputViewInfo.subresourceRange.layerCount = 1;

        VkImageView outputImageView;
        if (vkCreateImageView(device, &outputViewInfo, nullptr, &outputImageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create output texture image view!");
        }

        // 输入的是上一级的高斯纹理
        VkDescriptorImageInfo inputImageInfo{};
        inputImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL; // 图像布局
        inputImageInfo.imageView = inputImageView; // 图像视图
        inputImageInfo.sampler = textureTarget.sampler; // 纹理采样器

        VkWriteDescriptorSet inputImageDescriptorWrite{};
        inputImageDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        inputImageDescriptorWrite.dstSet = gaussianComputeResources.descriptorSet; // 目标描述符集
        inputImageDescriptorWrite.dstBinding = 0; // 目标绑定点
        inputImageDescriptorWrite.dstArrayElement = 0; // 目标数组元素
        inputImageDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; // 描述符类型
        inputImageDescriptorWrite.descriptorCount = 1; // 描述符数量
        inputImageDescriptorWrite.pBufferInfo = nullptr; // 缓冲区信息
        inputImageDescriptorWrite.pImageInfo = &inputImageInfo; // 图像信息

        // 输出的是currentLevel的高斯纹理
        VkDescriptorImageInfo outputImageInfo{};
        outputImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL; // 图像布局
        outputImageInfo.imageView = outputImageView; // 图像视图
        outputImageInfo.sampler = textureTarget.sampler; // 纹理采样器

        VkWriteDescriptorSet outputImageDescriptorWrite{};
        outputImageDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        outputImageDescriptorWrite.dstSet = gaussianComputeResources.descriptorSet; // 目标描述符集
        outputImageDescriptorWrite.dstBinding = 1; // 目标绑定点
        outputImageDescriptorWrite.dstArrayElement = 0; // 目标数组元素
        outputImageDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE; // 描述符类型
        outputImageDescriptorWrite.descriptorCount = 1; // 描述符数量
        outputImageDescriptorWrite.pBufferInfo = nullptr; // 缓冲区信息
        outputImageDescriptorWrite.pImageInfo = &outputImageInfo; // 图像信息

        VkDescriptorBufferInfo dicomBufferInfo{};
        dicomBufferInfo.buffer = dicomUniformBuffers[0]; // 缓冲区
        dicomBufferInfo.offset = 0; // 偏移量
        dicomBufferInfo.range = sizeof(DicomUniformBufferObject); // 范围

        VkWriteDescriptorSet dicomBufferDescriptorWrite{};
        dicomBufferDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        dicomBufferDescriptorWrite.dstSet = gaussianComputeResources.descriptorSet; // 目标描述符集
        dicomBufferDescriptorWrite.dstBinding = 2; // 目标绑定点
        dicomBufferDescriptorWrite.dstArrayElement = 0; // 目标数组元素
        dicomBufferDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // 描述符类型
        dicomBufferDescriptorWrite.descriptorCount = 1; // 描述符数量
        dicomBufferDescriptorWrite.pBufferInfo = &dicomBufferInfo; // 缓冲区信息
        dicomBufferDescriptorWrite.pImageInfo = nullptr; // 图像信息

        std::array<VkWriteDescriptorSet, 3> computeWrites = {
            inputImageDescriptorWrite,
            outputImageDescriptorWrite,
            dicomBufferDescriptorWrite
        };
        vkUpdateDescriptorSets(device, static_cast<uint32_t>(computeWrites.size()), computeWrites.data(), 0, nullptr); // 更新描述符集


        vkResetCommandBuffer(gaussianComputeResources.commandBuffer, 0); // 重置命令缓冲区
        // Create a command buffer for compute operations
        VkCommandBufferBeginInfo cmdBufBeginInfo{};
        cmdBufBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(gaussianComputeResources.commandBuffer, &cmdBufBeginInfo) != VK_SUCCESS) { // 开始记录命令缓冲区
            throw std::runtime_error("failed to begin command buffer!");
        }

        // Bind compute pipeline
        vkCmdBindPipeline(gaussianComputeResources.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, gaussianComputeResources.pipelines[0]);

        // Bind descriptor sets
        vkCmdBindDescriptorSets(gaussianComputeResources.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, gaussianComputeResources.pipelineLayout, 0, 1, &gaussianComputeResources.descriptorSet, 0, nullptr);

        // upload push constants
        GenGaussianMMPushConstants pushConstants{};
        pushConstants.currentLevel = static_cast<glm::vec1>(currentLevel);

        vkCmdPushConstants(gaussianComputeResources.commandBuffer, gaussianComputeResources.pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(GenGaussianMMPushConstants), &pushConstants);

        // Dispatch compute shader
        vkCmdDispatch(gaussianComputeResources.commandBuffer, mipWidth / 2, mipHeight / 2, mipDepth / 2);

        // End recording command buffer
        if (vkEndCommandBuffer(gaussianComputeResources.commandBuffer) != VK_SUCCESS) { // 结束记录命令缓冲区
            throw std::runtime_error("failed to record command buffer!");
        }

        // Submit to the compute queue
        VkSubmitInfo computeSubmitInfo{};
        computeSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        computeSubmitInfo.commandBufferCount = 1; // 命令缓冲区数量
        computeSubmitInfo.pCommandBuffers = &gaussianComputeResources.commandBuffer; // 命令缓冲区

        if (vkQueueSubmit(gaussianComputeResources.queue, 1, &computeSubmitInfo, VK_NULL_HANDLE) != VK_SUCCESS) { // 提交命令缓冲区
            throw std::runtime_error("failed to submit compute command buffer!");
        }

        // Flush the queue if we're rebuilding the command buffer after a pipeline change to ensure it's not currently in use
        vkQueueWaitIdle(gaussianComputeResources.queue);

        // Destroy image views
        vkDestroyImageView(device, inputImageView, nullptr);
        vkDestroyImageView(device, outputImageView, nullptr);
        std::cout << "current level:" << currentLevel << std::endl;
    }


}

void VulkanApplication::prepareTexOccConeSectionsInfo() {
    float* pixels = nullptr;
    volumeRender->sampler_occlusion.GetConeSectionsInfoTex(pixels);
    int texWidth = volumeRender->sampler_occlusion.GetNumberOfComputedConeSections();
    VkDeviceSize imageSize = texWidth * 16;

    std::cout << "texWidth = " << texWidth << std::endl;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    // 创建临时缓冲区
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(
        imageSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory);

    // 将数据复制到临时缓冲区
    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(device, stagingBufferMemory);

    // 释放像素数组
    stbi_image_free(pixels);

    /*
    虽然我们可以设置着色器来访问缓冲区中的像素值，但最好还是使用 Vulkan 中的图像对象来实现这一目的。
    首先，图像对象允许我们使用二维坐标，这将使我们更容易、更快速地检索颜色。图像对象中的像素被称为 texels
     */

     // 创建纹理图像
    createImage(
        texWidth,
        1,
        1,
        VK_FORMAT_R32G32B32A32_SFLOAT,
        VK_IMAGE_TYPE_1D,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        texOccConeSectionsInfo.image,
        texOccConeSectionsInfo.memory,
        VK_SAMPLE_COUNT_1_BIT);

    // 将纹理图像转换为 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    transitionImageLayout(
        texOccConeSectionsInfo.image,
        VK_FORMAT_R32G32B32A32_SFLOAT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // 将缓冲区数据复制到图像对象
    copyBufferToImage(
        stagingBuffer,
        texOccConeSectionsInfo.image,
        static_cast<uint32_t>(texWidth),
        static_cast<uint32_t>(1),
        1);

    // 为了能够在着色器中开始从纹理图像中采样，我们需要最后一次转换，为着色器访问纹理图像做好准备。
    // 将纹理图像转换为 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    transitionImageLayout(
        texOccConeSectionsInfo.image,
        VK_FORMAT_R32G32B32A32_SFLOAT,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // 清理临时缓冲区
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);

    // 创建图像视图
    texOccConeSectionsInfo.imageView = createImageView(texOccConeSectionsInfo.image, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_VIEW_TYPE_1D);
}

void VulkanApplication::captureImage() {
    // 1. 创建临时缓冲区
    VkDeviceSize imageSize = swapChainExtent.width * swapChainExtent.height * 4;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(
        imageSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory);

    // 将交换链图像转换为 VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL，才能作为源图像
    transitionImageLayout(
        swapChainImages[currentFrame],
        swapChainImageFormat,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    // 将图像对象复制到缓冲区
    copyImageToBuffer(
        swapChainImages[currentFrame],
        stagingBuffer,
        swapChainExtent.width,
        swapChainExtent.height);

    // 从缓冲区中读取像素值
    void* image_data;
    vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &image_data);

    unsigned char* uchar_ptr = reinterpret_cast<unsigned char*>(image_data);
    // 修复像素顺序
    for (int i = 0; i < swapChainExtent.width * swapChainExtent.height; i++) {
        unsigned char temp = uchar_ptr[i * 4];
        uchar_ptr[i * 4] = uchar_ptr[i * 4 + 2]; // 交换B和R通道的值
        uchar_ptr[i * 4 + 2] = temp;
    }

    // 保存图像
    std::string filename = "D:\\00.CG_project\\00.VulkanTutorial\\capture\\capture_" + std::to_string(currentCaptureID) + ".png";
    stbi_write_png(filename.c_str(), swapChainExtent.width, swapChainExtent.height, 4, uchar_ptr, swapChainExtent.width * 4);

    // 清理临时缓冲区
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);

    transitionImageLayout(
        swapChainImages[currentFrame],
        swapChainImageFormat,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
}

void VulkanApplication::copyImage(
    VkImage srcImage,
    VkImage dstImage,
    uint32_t width,
    uint32_t height) {
    // 创建命令缓冲区
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(commandPool);

    // 设置图像复制
    VkImageCopy region{};
    region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // 颜色位
    region.srcSubresource.layerCount = 1; // 图像层数
    region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // 颜色位
    region.dstSubresource.layerCount = 1; // 图像层数
    region.extent.width = width; // 图像宽度
    region.extent.height = height; // 图像高度
    region.extent.depth = 1; // 图像深度

    // 设置图像复制
    vkCmdCopyImage(
        commandBuffer,
        srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1, &region);

    // 结束记录命令缓冲区
    endSingleTimeCommands(commandBuffer, commandPool);
}

void VulkanApplication::copyImageToBuffer(
    VkImage srcImage,
    VkBuffer dstBuffer,
    uint32_t width,
    uint32_t height) {
    // 创建命令缓冲区
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(commandPool);

    // 设置图像复制
    VkBufferImageCopy region{};
    region.bufferOffset = 0; // 缓冲区偏移量
    region.bufferRowLength = 0; // 缓冲区行长度
    region.bufferImageHeight = 0; // 缓冲区图像高度
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // 颜色位
    region.imageSubresource.mipLevel = 0; // 图像mip级别
    region.imageSubresource.baseArrayLayer = 0; // 图像基本数组层

    // 设置图像复制
    region.imageSubresource.layerCount = 1; // 图像层数
    region.imageOffset = { 0, 0, 0 }; // 图像偏移量
    region.imageExtent = { width, height, 1 }; // 图像范围

    // 设置图像复制
    vkCmdCopyImageToBuffer(
        commandBuffer,
        srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        dstBuffer,
        1, &region);

    // 结束记录命令缓冲区
    endSingleTimeCommands(commandBuffer, commandPool);
}