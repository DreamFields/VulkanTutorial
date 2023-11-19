# VulkanTutorial

```cpp
// 包含必要的 Vulkan 头文件
#include <vulkan/vulkan.h>

// 根据您的需求，定义渲染通道、帧缓冲和纹理的相关变量

// 渲染通道和帧缓冲
VkRenderPass renderPass;
VkFramebuffer framebuffer;

// 背面纹理和纹理图像内存
VkImage backFaceImage;
VkDeviceMemory backFaceImageMemory;
VkImageView backFaceImageView;

// 正面纹理和纹理图像内存
VkImage frontFaceImage;
VkDeviceMemory frontFaceImageMemory;
VkImageView frontFaceImageView;

// 顶点数据和缓冲
std::vector<Vertex> vertices;
VkBuffer vertexBuffer;
VkDeviceMemory vertexBufferMemory;

// 着色器模块
VkShaderModule vertexShaderModule;
VkShaderModule fragmentShaderModule;

// Vulkan 设备、队列、命令池等对象的相关变量
VkDevice device;
VkQueue graphicsQueue;
VkCommandPool commandPool;

// 创建 Vulkan 设备、渲染通道、帧缓冲、纹理等对象
void createDevice()
{
    // 创建 Vulkan 设备和队列
    ...

    // 创建命令池
    VkCommandPoolCreateInfo commandPoolInfo {};
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.queueFamilyIndex = ...; // 指定图形队列的队列族索引
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    
    if (vkCreateCommandPool(device, &commandPoolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        // 错误处理
    }

    // 创建渲染通道
    ...

    // 创建帧缓冲
    ...

    // 创建背面纹理和纹理图像
    ...

    // 创建正面纹理和纹理图像
    ...

    // 创建顶点缓冲
    ...

    // 创建着色器模块
    ...
}

// 渲染背面
void renderBackFace()
{
    // 使用背面剔除的渲染参数
    ...

    // 绑定背面渲染通道和帧缓冲
    ...

    // 绑定背面着色器模块
    ...

    // 渲染顶点数据
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkRenderPassBeginInfo renderPassInfo {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = framebuffer;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChainExtent;
    std::array<VkClearValue, 2> clearValues {};
    // 指定深度缓冲区和颜色附件的清除值：
    clearValues[0].depthStencil = {1.0f, 0};
    clearValues[1].color = {0.0f, 0.0f, 0.0f, 0.0f};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    VkBuffer vertexBuffers[] = {vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
    
    vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);
    
    vkCmdEndRenderPass(commandBuffer);
    
    endSingleTimeCommands(commandBuffer);
}

// 渲染正面
void renderFrontFace()
{
    // 不使用背面剔除的渲染参数

    ...
    
    // 绑定正面渲染通道和帧缓冲
    ...

    // 绑定正面着色器模块
    ...

    // 渲染顶点数据
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkRenderPassBeginInfo renderPassInfo {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = framebuffer;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChainExtent;
    std::array<VkClearValue, 2> clearValues {};
    // 指定深度缓冲区和颜色附件的清除值：
    clearValues[0].depthStencil = {1.0f, 0};
    clearValues[1].color = {0.0f, 0.0f, 0.0f, 0.0f};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    VkBuffer vertexBuffers[] = {vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
    
    vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);
    
    vkCmdEndRenderPass(commandBuffer);
    
    endSingleTimeCommands(commandBuffer);
}

// 保存背面的世界空间坐标到纹理
void saveBackFaceToWorldSpaceCoords()
{
    // 使用背面纹理的图像布局转换为一般传输布局
    ...

    // 创建命令缓冲
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    // 开始图像布局转换
    VkImageMemoryBarrier barrier {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.srcAccessMask = 0; // 没有访问前一布局的计划
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = backFaceImage;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    // 在片段着色器中，将背面的世界空间坐标保存到纹理中
    ...

    // 结束图像布局转换
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    endSingleTimeCommands(commandBuffer);
}

// 保存正面的世界空间坐标到纹理
void saveFrontFaceToWorldSpaceCoords()
{
    // 使用正面纹理的图像布局转换为一般传输布局
    ...

    // 创建命令缓冲
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();


    // 开始图像布局转换
    VkImageMemoryBarrier barrier {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.srcAccessMask = 0; // 没有访问前一布局的计划
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = frontFaceImage;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    // 在片段着色器中，将正面的世界空间坐标保存到纹理中
    ...

    // 结束图像布局转换
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    endSingleTimeCommands(commandBuffer);
}
```