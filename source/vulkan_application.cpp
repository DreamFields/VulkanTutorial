#include "vulkan_application.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
void VulkanApplication::initVolumeRender() {
    volumeRender = std::make_shared<VolumeRender>();
    volumeRender->loadDicom("C:\\Users\\Dream\\Documents\\00.Dicom\\ede6fe9eda6e44a98b3ad20da6f9116a Anonymized29\\Unknown Study\\CT Head 5.0000\\CT000027.dcm");
}

void VulkanApplication::run() {
    initWindow();
    initVulkan();
    // initVolumeRender();
    mainLoop();
    cleanup();
}

void VulkanApplication::createTextureImage() {
    int texWidth, texHeight, texChannels;
    // STBI_rgb_alpha 值会强制加载图像的 alpha 通道，即使图像没有 alpha 通道也是如此，这有利于将来与其他纹理保持一致。
    // 中间三个参数用于输出图像的宽度、高度和实际通道数。
    // 返回的指针是像素值数组的第一个元素。
    // stbi_uc* pixels = stbi_load("D:\\00.CG_project\\VulkanTutorial\\textures\\texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    unsigned char* pixels = nullptr;
    volumeRender->getPixelRGBA("C:\\Users\\Dream\\Documents\\00.Dicom\\ede6fe9eda6e44a98b3ad20da6f9116a Anonymized29\\Unknown Study\\CT Head 5.0000\\CT000027.dcm", texWidth, texHeight, texChannels, pixels);

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
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        textureImage,
        textureImageMemory);

    // 将纹理图像转换为 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    transitionImageLayout(
        textureImage,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // 将缓冲区数据复制到图像对象
    copyBufferToImage(
        stagingBuffer,
        textureImage,
        static_cast<uint32_t>(texWidth),
        static_cast<uint32_t>(texHeight));

    // 为了能够在着色器中开始从纹理图像中采样，我们需要最后一次转换，为着色器访问纹理图像做好准备。
    // 将纹理图像转换为 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    transitionImageLayout(
        textureImage,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // 清理临时缓冲区
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void VulkanApplication::createImage(    // 创建图像对象
    uint32_t width,
    uint32_t height,
    VkFormat format,
    VkImageTiling tiling,   // 图像数据的布局
    VkImageUsageFlags usage,    // 图像用途
    VkMemoryPropertyFlags properties,   // 内存属性
    VkImage& image,
    VkDeviceMemory& imageMemory) {
    // 创建图像对象
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;   // 图像类型告诉 Vulkan 图像中的像素将采用哪种坐标系。可以创建一维、二维和三维图像。一维图像可用于存储数据数组或梯度，二维图像主要用于纹理，三维图像可用于存储体素体积等。
    imageInfo.extent.width = static_cast<uint32_t>(width);    // 图像的宽度和高度，以像素为单位
    imageInfo.extent.height = static_cast<uint32_t>(height);
    imageInfo.extent.depth = 1; // 图像的深度，对于 2D 图像，其值必须为 1
    imageInfo.mipLevels = 1;    // 图像的 mipmap 级别数
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

    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;   // 采样数，指定多重采样纹理的采样数，通常为 1，最大为 VK_SAMPLE_COUNT_64_BIT
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

void VulkanApplication::createTextureImageView() {
    // 创建纹理的图像视图
    textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB);
}


VkImageView VulkanApplication::createImageView(VkImage image, VkFormat format) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO; // 指定结构体类型
    viewInfo.image = image; // 指定图像对象
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;    // 指定图像视图类型
    viewInfo.format = format;   // 指定图像数据的格式
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // 指定图像的哪些方面将受到屏障的影响。我们使用颜色位，因为我们只关心颜色数据
    viewInfo.subresourceRange.baseMipLevel = 0;    // 指定图像的哪些 mipmap 级别将受到屏障的影响。我们将其设置为 0，以便它可以影响所有级别
    viewInfo.subresourceRange.levelCount = 1; // 指定图像的 mipmap 级别数量
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
    samplerInfo.magFilter = VK_FILTER_LINEAR;    // 放大时的采样方式
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
    samplerInfo.maxLod = 0.0f;  // 指定 mipmap 的最大 LOD

    // 采样器不会在任何地方引用 VkImage。采样器是一个独特的对象，它提供了一个从纹理中提取颜色的接口。它可以应用于任何图像，无论是一维、二维还是三维图像。
    if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {   // 创建纹理采样器
        throw std::runtime_error("failed to create texture sampler!");
    }

}

void VulkanApplication::transitionImageLayout(    // 图像布局转换
    VkImage image,
    VkFormat format,
    VkImageLayout oldLayout,
    VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(); // 开始记录命令缓冲区

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
    barrier.subresourceRange.levelCount = 1; // 图像的 mipmap 级别数量
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

    endSingleTimeCommands(commandBuffer);   // !结束记录命令缓冲区,否则会报如下错误：
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
    uint32_t height) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(); // 开始记录命令缓冲区

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
    region.imageExtent = { width, height, 1 };    // 图像范围

    // 将缓冲区数据复制到图像对象
    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, // 图像布局
        1, // regionCount
        &region); // pRegions

    endSingleTimeCommands(commandBuffer);   // 结束记录命令缓冲区
}

VkCommandBuffer VulkanApplication::beginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    // commandPool是用于分配命令缓冲区的命令池
    allocInfo.commandPool = commandPool;
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

void VulkanApplication::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
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
    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}