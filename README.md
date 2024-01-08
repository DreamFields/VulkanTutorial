# VulkanTutorial

## 启动
```bash
cmake -S . -B build
```

## Vulkan的多pass渲染

- https://github.com/SaschaWillems/Vulkan/tree/master/examples/subpasses

## ImGUI集成到Vulkan

- https://frguthmann.github.io/posts/vulkan_imgui/
- https://tstullich.github.io/posts/vulkan-sandbox/

## Vulkan编程要点

### 交换链数、并发帧数、帧缓冲数


在Vulkan中，imageCount是指交换链（Swapchain）中图像的数量，它表示在交换链中可以同时存在的图像数量。这个值通常由应用程序在创建交换链时指定，根据应用程序的需求和硬件的限制来确定。较大的imageCount值可以提供更多的图像用于渲染和呈现，但也可能增加内存消耗。

Framebuffer的数量是指用于渲染每一帧的帧缓冲对象的数量。帧缓冲对象是用于存储渲染操作的中间结果的Vulkan对象。每个帧缓冲对象与一个交换链图像相关联，用于将渲染操作的结果呈现到屏幕上。因此，Framebuffer的数量通常等于交换链中图像的数量（imageCount）。每个帧缓冲对象都需要分配内存和管理资源，因此较多的帧缓冲对象数量可能会增加内存消耗。

max_inflight_frames是指在应用程序中同时处理的帧（或图像）数量。它通常用于控制并发执行的帧数，以平衡CPU和GPU的工作负载。max_inflight_frames的值应小于或等于imageCount，以确保不会超过交换链中可用的图像数量。

在典型的Vulkan应用程序中，交换链图像的数量（imageCount）和最大并发帧数（max_inflight_frames）之间的关系如下：

- 如果max_inflight_frames小于或等于imageCount，则可以在每个帧中为交换链中的每个图像分配一个帧缓冲，并同时处理多个帧。这样可以最大程度地利用GPU的并行性能。
- 如果max_inflight_frames大于imageCount，则无法为每个帧分配一个帧缓冲，因为帧缓冲数量受限于交换链中的图像数量。这可能会导致性能下降和资源浪费。

# TODO

- 纹理结构体抽象
- 创建资源抽象
- GPU同步的完善
- 立方体渲染有棱角的影子