#pragma once 
#include "volume_render.h"
#include "camera.h"
#include "struct.h"
#include <debugdraw_vert.h> // 通过库文件的形式来引入着色器文件
#include <debugdraw_frag.h>
#include <generateExtinctionCoef_comp.h>
#include <generateExtinctionCoefMipmap_comp.h>

#include <backpos_vert.h>
#include <backpos_frag.h>
#include <composition_vert.h>
#include <composition_frag.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// include imgui
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <chrono>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <optional>
#include <set>
#include <array>

const uint32_t WIDTH = 1920;
const uint32_t HEIGHT = 1080;

const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;
	std::optional<uint32_t> computeFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value() && computeFamily.has_value();
	}
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class VulkanApplication {
public:
	void run();
	std::shared_ptr<VolumeRender> volumeRender;
	std::shared_ptr<Camera> camera;

private:
	int currentExampleID = 0; // * 当前的示例ID
	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;
	GLFWwindow* window;

	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkSurfaceKHR surface;

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;

	VkQueue graphicsQueue;
	VkQueue presentQueue;

	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImageView> swapChainImageViews;
	std::vector<VkFramebuffer> swapChainFramebuffers;

	VkRenderPass renderPass;

	struct {
		VkDescriptorSetLayout backFaceDescriptorSetLayout;
		VkDescriptorSetLayout compositionDescriptorSetLayout;
	} descriptorSetLayout;
	// VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorPool descriptorPool;
	struct {
		std::vector<VkDescriptorSet> backFaceDescriptorSets;
		std::vector<VkDescriptorSet> compositionDescriptorSets;
	} descriptorSets;
	// std::vector<VkDescriptorSet> descriptorSets;

	struct {
		VkPipelineLayout offscreen;
		VkPipelineLayout composition;
	} pipelineLayout;
	// VkPipelineLayout pipelineLayout;

	struct {
		VkPipeline offscreen;
		VkPipeline composition;
	} graphicsPipeline;
	// VkPipeline graphicsPipeline;

	VkCommandPool commandPool;

	// 3D纹理 用于存储体数据
	VkImage texture3DImage;
	VkImageView texture3DImageView;
	VkDeviceMemory texture3DImageMemory;
	float maxMipLevels;

	VkSampler textureSampler;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	// 背面纹理和纹理图像内存
	VkImage backFaceImage;
	VkDeviceMemory backFaceImageMemory;
	VkImageView backFaceImageView;

	// look up table texture 1D
	VkImage lutImage;
	VkDeviceMemory lutImageMemory;
	VkImageView lutImageView;

	VkImage test2DImage;
	VkDeviceMemory test2DImageMemory;
	VkImageView test2DImageView;

	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;
	std::vector<void*> uniformBuffersMapped;

	std::vector<VkBuffer> dicomUniformBuffers;
	std::vector<VkDeviceMemory> dicomUniformBuffersMemory;
	std::vector<void*> dicomUniformBuffersMapped;

	std::vector<VkBuffer> occlusionUniformBuffers;
	std::vector<VkDeviceMemory> occlusionUniformBuffersMemory;
	std::vector<void*> occlusionUniformBuffersMapped;

	// for ground truth second ray
	std::vector<VkBuffer> groundTruthRayUniformBuffers;
	std::vector<VkDeviceMemory> groundTruthRayUniformBuffersMemory;
	std::vector<void*> groundTruthRayUniformBuffersMapped;

	std::vector<VkCommandBuffer> commandBuffers;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	uint32_t currentFrame = 0;

	// imgui
	VkDescriptorPool imguiDescriptorPool;
	VkRenderPass imguiRenderPass;
	VkCommandPool imguiCommandPool;
	std::vector<VkCommandBuffer> imguiCommandBuffers;
	std::vector<VkSemaphore> imguiFinishedSemaphores;
	std::vector<VkFence> imguiInFlightFences;
	std::vector<VkFramebuffer> imguiFramebuffers;

	bool framebufferResized = false;
	bool shouldExit = false;

	void initVolume();
	void initGeometry();
	void create1DTextureImage();
	void create2DTextureImage();
	void create3DTextureImage();
	void createTextureImageView();
	void createImage(uint32_t width, uint32_t height, uint32_t depth, VkFormat format, VkImageType imageType, VkImageTiling tiling, VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, VkSampleCountFlagBits numSamples, uint32_t mipLevels = 1);
	VkImageView createImageView(VkImage image, VkFormat format, VkImageViewType viewType, uint32_t mipLevels = 1);
	void createTextureSampler();
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels = 1);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t depth);
	void createBackposAttachmentImage();

	// begin imgui
	void initImGui();
	void createImGuiDescriptorPool();
	void createImGuiRenderPass();
	void createImGuiCommandPool();
	void createImGuiCommandBuffers();
	void createImGuiFramebuffers();
	void createImGuiSyncObjects();
	void drawImGui();
	void recordImGuiCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
	// end imgui

	// begin compute
	Compute computeResources;   // Compute resources
	TextureTarget textureTarget; // Target image for compute shader writes
	StaticCompute gaussianComputeResources; // Gaussian compute resources
	void prepareTextureTarget();
	void prepareCompute();
	void recordComputeCommandBuffer(uint32_t currentFrame);
	void recordGenExtCoffMipmaps(uint32_t currentFrame);
	void prepareGaussianCompute();
	void recordGenGaussianMipmaps();
	// end compute

	// begin TexOccConeSectionsInfo
	TextureTarget texOccConeSectionsInfo;
	void prepareTexOccConeSectionsInfo();
	// end TexOccConeSectionsInfo 

	// begin capture
	bool isNeedCapture = false; // 是否需要截图
	int currentCaptureID = -1; // 当前的捕获ID
	void captureImage();
	void copyImage(VkImage srcImage,
		VkImage dstImage,
		uint32_t width,
		uint32_t height); // 将交换链图像复制到图像对象	
	void copyImageToBuffer(VkImage srcImage,
		VkBuffer dstBuffer,
		uint32_t width,
		uint32_t height); // 将交换链图像复制到缓冲区对象
	// end capture

	// https://vulkan-tutorial.com/Texture_mapping/Images#page_Layout-transitions
	// 将记录和执行命令缓冲区的逻辑抽象为单独的函数
	VkCommandBuffer beginSingleTimeCommands(VkCommandPool cmdPool);
	void endSingleTimeCommands(VkCommandBuffer commandBuffer, VkCommandPool cmdPool);

	void initWindow() {
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
		// 设置鼠标移动和按键回调函数
		camera = std::make_shared<Camera>(currentExampleID);
		// 鼠标移动回调函数
		glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
			auto app = reinterpret_cast<VulkanApplication*>(glfwGetWindowUserPointer(window));
			app->camera->onMouseMove(window, xpos, ypos);
			});
		// 鼠标按键回调函数
		glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
			// 如果 ImGui 希望捕获鼠标事件，则不传递给主程序
			if (action == GLFW_PRESS && ImGui::GetIO().WantCaptureMouse) {
				return;
			}
			auto app = reinterpret_cast<VulkanApplication*>(glfwGetWindowUserPointer(window));
			app->camera->onMouseButton(window, button, action, mods);
			});
		// 将鼠标指针模式设置为 `GLFW_CURSOR_NORMAL`不隐藏鼠标
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		// 设置键盘按键回调函数
		glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
			auto app = reinterpret_cast<VulkanApplication*>(glfwGetWindowUserPointer(window));
			if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
				app->shouldExit = true;
			}
			});
	}

	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto app = reinterpret_cast<VulkanApplication*>(glfwGetWindowUserPointer(window));
		app->framebufferResized = true;
	}

	void initVulkan() {
		createInstance();
		setupDebugMessenger();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
		createImageViews();
		createRenderPass();
		createDescriptorSetLayout();
		createGraphicsPipeline();
		createCommandPool();
		// create2DTextureImage();
		createBackposAttachmentImage(); // 创建附件图像，用于存储上一阶段渲染的图像，而不是从文件传入的纹理
		create3DTextureImage();
		create1DTextureImage();

		// generate TexOccConeSectionsInfo
		prepareTexOccConeSectionsInfo();

		createTextureImageView(); // 创建纹理图像视图
		createFramebuffers();
		createTextureSampler();
		createVertexBuffer();
		createIndexBuffer();
		createUniformBuffers();
		createDescriptorPool();

		prepareCompute();

		prepareGaussianCompute();	

		createDescriptorSets();
		createCommandBuffers();
		createSyncObjects();
	}

	void mainLoop() {
		while (!glfwWindowShouldClose(window) && !shouldExit) {
			glfwPollEvents();
			drawImGui();
			drawFrame();

			// 检查是否按下ESC键
			if (shouldExit) {
				glfwSetWindowShouldClose(window, GLFW_TRUE);
			}
		}

		vkDeviceWaitIdle(device);
	}

	void cleanupSwapChain() {
		for (auto framebuffer : swapChainFramebuffers) {
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		}

		for (auto imageView : swapChainImageViews) {
			vkDestroyImageView(device, imageView, nullptr);
		}

		vkDestroySwapchainKHR(device, swapChain, nullptr);
	}

	void cleanup() {
		// cleanup compute resource
		vkDestroyImage(device, textureTarget.image, nullptr);
		vkFreeMemory(device, textureTarget.memory, nullptr);
		vkDestroyImageView(device, textureTarget.imageView, nullptr);
		vkDestroyDescriptorSetLayout(device, computeResources.descriptorSetLayout, nullptr);
		vkDestroyPipelineLayout(device, computeResources.pipelineLayout, nullptr);
		vkDestroyPipeline(device, computeResources.pipelines[0], nullptr);
		vkDestroyCommandPool(device, computeResources.commandPool, nullptr);
		for (auto fence : computeResources.inFlightFences) {
			vkDestroyFence(device, fence, nullptr);
		}
		for (auto semaphore : computeResources.finishedSemaphores) {
			vkDestroySemaphore(device, semaphore, nullptr);
		}
		for (auto semaphore : computeResources.finishedGenMipmapSemaphores) {
			vkDestroySemaphore(device, semaphore, nullptr);
		}

		// cleanup gaussian compute resource
		vkDestroyDescriptorSetLayout(device, gaussianComputeResources.descriptorSetLayout, nullptr);
		vkDestroyPipelineLayout(device, gaussianComputeResources.pipelineLayout, nullptr);
		vkDestroyPipeline(device, gaussianComputeResources.pipelines[0], nullptr);
		vkDestroyCommandPool(device, gaussianComputeResources.commandPool, nullptr);
		vkDestroyFence(device,gaussianComputeResources.inFlightFence, nullptr);
		vkDestroySemaphore(device, gaussianComputeResources.finishedSemaphore, nullptr);

		// Cleanup DearImGui
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
		vkDestroyDescriptorPool(device, imguiDescriptorPool, nullptr);
		vkDestroyRenderPass(device, imguiRenderPass, nullptr);
		vkDestroyCommandPool(device, imguiCommandPool, nullptr);
		for (auto framebuffer : imguiFramebuffers) {
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		}
		for (auto fence : imguiInFlightFences) {
			vkDestroyFence(device, fence, nullptr);
		}
		for (auto semaphore : imguiFinishedSemaphores) {
			vkDestroySemaphore(device, semaphore, nullptr);
		}


		cleanupSwapChain();

		vkDestroySampler(device, textureSampler, nullptr);
		vkDestroyImageView(device, texture3DImageView, nullptr);
		vkDestroyImage(device, texture3DImage, nullptr);
		vkFreeMemory(device, texture3DImageMemory, nullptr);

		vkDestroyImageView(device, backFaceImageView, nullptr);
		vkDestroyImage(device, backFaceImage, nullptr);
		vkFreeMemory(device, backFaceImageMemory, nullptr);

		vkDestroyImageView(device, lutImageView, nullptr);
		vkDestroyImage(device, lutImage, nullptr);
		vkFreeMemory(device, lutImageMemory, nullptr);

		vkDestroyImageView(device, texOccConeSectionsInfo.imageView, nullptr);
		vkDestroyImage(device, texOccConeSectionsInfo.image, nullptr);
		vkFreeMemory(device, texOccConeSectionsInfo.memory, nullptr);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroyBuffer(device, uniformBuffers[i], nullptr);
			vkDestroyBuffer(device, dicomUniformBuffers[i], nullptr);
			vkDestroyBuffer(device, occlusionUniformBuffers[i], nullptr);
			vkDestroyBuffer(device, groundTruthRayUniformBuffers[i], nullptr);
			vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
			vkFreeMemory(device, dicomUniformBuffersMemory[i], nullptr);
			vkFreeMemory(device, occlusionUniformBuffersMemory[i], nullptr);
			vkFreeMemory(device, groundTruthRayUniformBuffersMemory[i], nullptr);
		}
		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout.backFaceDescriptorSetLayout, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout.compositionDescriptorSetLayout, nullptr);

		vkDestroyBuffer(device, vertexBuffer, nullptr);
		vkFreeMemory(device, vertexBufferMemory, nullptr);

		vkDestroyBuffer(device, indexBuffer, nullptr);
		vkFreeMemory(device, indexBufferMemory, nullptr);

		vkDestroyPipeline(device, graphicsPipeline.offscreen, nullptr);
		vkDestroyPipeline(device, graphicsPipeline.composition, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout.offscreen, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout.composition, nullptr);

		vkDestroyRenderPass(device, renderPass, nullptr);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(device, inFlightFences[i], nullptr);
		}

		vkDestroyCommandPool(device, commandPool, nullptr);

		vkDestroyDevice(device, nullptr);

		if (enableValidationLayers) {
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}

		vkDestroySurfaceKHR(instance, surface, nullptr);
		vkDestroyInstance(instance, nullptr);

		glfwDestroyWindow(window);

		glfwTerminate();
	}

	void recreateSwapChain() {
		int width = 0, height = 0;
		glfwGetFramebufferSize(window, &width, &height);
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(device);

		cleanupSwapChain();

		createSwapChain();
		createImageViews();
		createFramebuffers();

		// We also need to take care of the UI
		for (auto framebuffer : imguiFramebuffers) {
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		}
		vkFreeCommandBuffers(device, imguiCommandPool, static_cast<uint32_t>(imguiCommandBuffers.size()), imguiCommandBuffers.data());
		ImGui_ImplVulkan_SetMinImageCount(swapChainImages.size());
		createImGuiFramebuffers();
		createImGuiCommandBuffers();
	}

	void createInstance() {
		if (enableValidationLayers && !checkValidationLayerSupport()) {
			throw std::runtime_error("validation layers requested, but not available!");
		}

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		auto extensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else {
			createInfo.enabledLayerCount = 0;

			createInfo.pNext = nullptr;
		}

		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
			throw std::runtime_error("failed to create instance!");
		}
	}

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
	}

	void setupDebugMessenger() {
		if (!enableValidationLayers) return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		populateDebugMessengerCreateInfo(createInfo);

		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}

	void createSurface() {
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface!");
		}
	}

	void pickPhysicalDevice() {
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

		if (deviceCount == 0) {
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		for (const auto& device : devices) {
			if (isDeviceSuitable(device)) {
				physicalDevice = device;
				break;
			}
		}

		if (physicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("failed to find a suitable GPU!");
		}
	}

	void createLogicalDevice() {
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = VK_TRUE; // 启用各向异性过滤

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();

		createInfo.pEnabledFeatures = &deviceFeatures;

		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
			throw std::runtime_error("failed to create logical device!");
		}

		vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
		vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
	}

	void createSwapChain() {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		// createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT; // *有可能使用输入附件，所以加上VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT


		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		if (indices.graphicsFamily != indices.presentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;

		if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
			throw std::runtime_error("failed to create swap chain!");
		}

		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

		swapChainImageFormat = surfaceFormat.format;
		swapChainExtent = extent;
	}

	void createImageViews() {
		swapChainImageViews.resize(swapChainImages.size());

		for (size_t i = 0; i < swapChainImages.size(); i++) {
			swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_VIEW_TYPE_2D);
		}
	}

	void createRenderPass() {
		VkAttachmentDescription colorAttachments[2]{}; // 两个附件
		// color attachment
		colorAttachments[0].format = swapChainImageFormat;
		colorAttachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // !最终布局是颜色附件，因为最终显示的是UI

		// backpos attachment
		colorAttachments[1].format = VK_FORMAT_R16G16B16A16_SFLOAT;
		colorAttachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachments[1].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		// two subpasses
		std::array<VkSubpassDescription, 2> subpasses{};

		// 第一个子通道,用于渲染到帧缓冲区
		subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		VkAttachmentReference colorAttachmentRefs[2];
		colorAttachmentRefs[0] = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
		colorAttachmentRefs[1] = { 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
		subpasses[0].colorAttachmentCount = 2;
		subpasses[0].pColorAttachments = colorAttachmentRefs;

		// 第二个子通道，用于使用第一个子通道渲染的图像进行后处理
		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		subpasses[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpasses[1].colorAttachmentCount = 1;
		subpasses[1].pColorAttachments = &colorAttachmentRef;
		// 使用输入附件
		VkAttachmentReference inputAttachmentRef{};
		inputAttachmentRef.attachment = 1; // *这里的附件索引是1，因为我们定义了两个附件
		inputAttachmentRef.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		subpasses[1].inputAttachmentCount = 1;
		subpasses[1].pInputAttachments = &inputAttachmentRef;

		VkSubpassDependency dependencies[3]{}; // 两个依赖关系
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

		// This dependency transitions the input attachment from color attachment to input attachment read
		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = 1;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[2].srcSubpass = 1;
		dependencies[2].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[2].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[2].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[2].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		dependencies[2].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 2;
		renderPassInfo.pAttachments = colorAttachments;
		renderPassInfo.subpassCount = 2;
		renderPassInfo.pSubpasses = subpasses.data();
		renderPassInfo.dependencyCount = 3;
		renderPassInfo.pDependencies = dependencies;

		if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}
	}

	// https://vulkan-tutorial.com/Uniform_buffers/Descriptor_layout_and_buffer#page_Descriptor-set-layout
	void createDescriptorSetLayout() {
		// ----------------compositionDescriptorSetLayout----------------
		// 创建uniform缓冲区描述符布局的绑定
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0; // 着色器中的绑定索引
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		// stageFlags参数指定在哪个着色器阶段使用此描述符布局。我们将在顶点着色器中使用uniform缓冲区，因此我们将其设置为VK_SHADER_STAGE_VERTEX_BIT。
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr; // 可选，用于纹理采样

		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = 1; // 着色器中的绑定索引
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		// stageFlags参数指定在哪个着色器阶段使用此描述符布局。我们将在片段着色器中使用纹理采样器，因此我们将其设置为VK_SHADER_STAGE_FRAGMENT_BIT。
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		samplerLayoutBinding.pImmutableSamplers = nullptr; // 可选，用于纹理采样

		VkDescriptorSetLayoutBinding backFaceLayoutBinding{};
		backFaceLayoutBinding.binding = 2; // 着色器中的绑定索引
		backFaceLayoutBinding.descriptorCount = 1;
		backFaceLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT; // *用于输入附件
		// stageFlags参数指定在哪个着色器阶段使用此描述符布局。我们将在片段着色器中使用纹理采样器，因此我们将其设置为VK_SHADER_STAGE_FRAGMENT_BIT。
		backFaceLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		backFaceLayoutBinding.pImmutableSamplers = nullptr; // 可选，用于纹理采样

		VkDescriptorSetLayoutBinding dicomUboLayoutBinding{};
		dicomUboLayoutBinding.binding = 3;
		dicomUboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		dicomUboLayoutBinding.descriptorCount = 1;
		dicomUboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		dicomUboLayoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBinding lutLayoutBinding{};
		lutLayoutBinding.binding = 4;
		lutLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		lutLayoutBinding.descriptorCount = 1;
		lutLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		lutLayoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBinding extCoefBingding{};
		extCoefBingding.binding = 5;
		extCoefBingding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		extCoefBingding.descriptorCount = 1;
		extCoefBingding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		extCoefBingding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBinding TexOccConeSectionsInfoBinding{};
		TexOccConeSectionsInfoBinding.binding = 6;
		TexOccConeSectionsInfoBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		TexOccConeSectionsInfoBinding.descriptorCount = 1;
		TexOccConeSectionsInfoBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		TexOccConeSectionsInfoBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBinding occlusionUboLayoutBinding{};
		occlusionUboLayoutBinding.binding = 7;
		occlusionUboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		occlusionUboLayoutBinding.descriptorCount = 1;
		occlusionUboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		occlusionUboLayoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBinding groundTruthRayUboLayoutBinding{};
		groundTruthRayUboLayoutBinding.binding = 8;
		groundTruthRayUboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		groundTruthRayUboLayoutBinding.descriptorCount = 1;
		groundTruthRayUboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		groundTruthRayUboLayoutBinding.pImmutableSamplers = nullptr;

		std::array<VkDescriptorSetLayoutBinding, 9> bindings = {
			uboLayoutBinding,
			samplerLayoutBinding,
			backFaceLayoutBinding,
			dicomUboLayoutBinding,
			lutLayoutBinding,
			extCoefBingding,
			TexOccConeSectionsInfoBinding,
			occlusionUboLayoutBinding,
			groundTruthRayUboLayoutBinding
		};

		// 创建描述符布局
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size()); // 描述符绑定数量
		layoutInfo.pBindings = bindings.data(); // 描述符绑定 

		if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout.compositionDescriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}


		// ----------------backFaceDescriptorSetLayout----------------
		VkDescriptorSetLayoutCreateInfo backFaceLayoutInfo{};
		backFaceLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		backFaceLayoutInfo.bindingCount = 1;
		backFaceLayoutInfo.pBindings = &uboLayoutBinding;

		if (vkCreateDescriptorSetLayout(device, &backFaceLayoutInfo, nullptr, &descriptorSetLayout.backFaceDescriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

	void createGraphicsPipeline() {
		// auto vertShaderCode = readFile("shaders/vert.spv");
		// auto fragShaderCode = readFile("shaders/frag.spv");

		VkShaderModule backposVertShaderModule = createShaderModule(BACKPOS_VERT);
		VkShaderModule backposFragShaderModule = createShaderModule(BACKPOS_FRAG);

		VkPipelineShaderStageCreateInfo backposVertShaderStageInfo{};
		backposVertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		backposVertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		backposVertShaderStageInfo.module = backposVertShaderModule;
		backposVertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo backposFragShaderStageInfo{};
		backposFragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		backposFragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		backposFragShaderStageInfo.module = backposFragShaderModule;
		backposFragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo backposShaderStages[] = { backposVertShaderStageInfo, backposFragShaderStageInfo };

		VkShaderModule vertShaderModule = createShaderModule(COMPOSITION_VERT);
		VkShaderModule fragShaderModule = createShaderModule(COMPOSITION_FRAG);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		auto bindingDescription = Vertex::getBindingDescription();
		auto attributeDescriptions = Vertex::getAttributeDescriptions();
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		VkPipelineRasterizationStateCreateInfo backposRasterizer{};
		backposRasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		backposRasterizer.depthClampEnable = VK_FALSE;
		backposRasterizer.rasterizerDiscardEnable = VK_FALSE;
		backposRasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		backposRasterizer.lineWidth = 1.0f;
		backposRasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		backposRasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		backposRasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		// rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE; // 由于我们在投影矩阵中进行了 Y 翻转，顶点现在是按逆时针顺序而不是顺时针顺序绘制的。这导致反面剔除启动，无法绘制任何几何体。
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; // 通过将顶点顺序更改为顺时针顺序，我们可以解决这个问题，或者我们可以通过将 frontFace 设置为 VK_FRONT_FACE_COUNTER_CLOCKWISE 来保持顶点顺序不变。
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = 0xf;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;

		// VkPipelineColorBlendAttachmentState offscreenColorBlendAttachment{};
		// offscreenColorBlendAttachment.colorWriteMask = 0XF;
		// offscreenColorBlendAttachment.blendEnable = VK_FALSE;
		// *因为subpass 0 的colorattachments 有两个，所以这里需要两个colorBlendAttachment
		std::array<VkPipelineColorBlendAttachmentState, 2> offscreenColorBlendAttachment{};
		offscreenColorBlendAttachment[0].colorWriteMask = 0xf;
		offscreenColorBlendAttachment[0].blendEnable = VK_FALSE;
		offscreenColorBlendAttachment[1].colorWriteMask = 0xf;
		offscreenColorBlendAttachment[1].blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo offscreenColorBlending{};
		offscreenColorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		offscreenColorBlending.attachmentCount = 2;
		offscreenColorBlending.pAttachments = offscreenColorBlendAttachment.data();

		// ----------------offscreen----------------
		VkPipelineLayoutCreateInfo offscreenPipelineLayoutInfo{};
		offscreenPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		offscreenPipelineLayoutInfo.setLayoutCount = 1; // 描述符布局数量
		offscreenPipelineLayoutInfo.pSetLayouts = &descriptorSetLayout.backFaceDescriptorSetLayout; // 引用描述符布局

		if (vkCreatePipelineLayout(device, &offscreenPipelineLayoutInfo, nullptr, &pipelineLayout.offscreen) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}

		VkGraphicsPipelineCreateInfo offscreenPipelineInfo{};
		offscreenPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		offscreenPipelineInfo.stageCount = 2;
		offscreenPipelineInfo.pStages = backposShaderStages;
		offscreenPipelineInfo.pVertexInputState = &vertexInputInfo;
		offscreenPipelineInfo.pInputAssemblyState = &inputAssembly;
		offscreenPipelineInfo.pViewportState = &viewportState;
		offscreenPipelineInfo.pRasterizationState = &backposRasterizer; // 修改光栅化状态
		offscreenPipelineInfo.pMultisampleState = &multisampling;
		offscreenPipelineInfo.pColorBlendState = &offscreenColorBlending;
		offscreenPipelineInfo.pDynamicState = &dynamicState;
		offscreenPipelineInfo.layout = pipelineLayout.offscreen; // 修改管线布局
		offscreenPipelineInfo.renderPass = renderPass;
		offscreenPipelineInfo.subpass = 0;
		offscreenPipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		offscreenPipelineInfo.basePipelineIndex = -1;

		if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &offscreenPipelineInfo, nullptr, &graphicsPipeline.offscreen) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics pipeline!");
		}


		// ----------------composition----------------
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1; // 描述符布局数量
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout.compositionDescriptorSetLayout; // 引用描述符布局



		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout.composition) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer; // 修改光栅化状态
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = pipelineLayout.composition; // 修改管线布局
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 1;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline.composition) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics pipeline!");
		}

		vkDestroyShaderModule(device, backposVertShaderModule, nullptr);
		vkDestroyShaderModule(device, backposFragShaderModule, nullptr);
		vkDestroyShaderModule(device, fragShaderModule, nullptr);
		vkDestroyShaderModule(device, vertShaderModule, nullptr);
	}

	void createFramebuffers() {
		swapChainFramebuffers.resize(swapChainImageViews.size());

		for (size_t i = 0; i < swapChainImageViews.size(); i++) {
			std::array<VkImageView, 2> attachments = {
				swapChainImageViews[i],
				backFaceImageView
			};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = 2;
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = swapChainExtent.width;
			framebufferInfo.height = swapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
	}

	void createCommandPool() {
		QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

		if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create command pool!");
		}
	}

	// https://vulkan-tutorial.com/Vertex_buffers/Vertex_buffer_creation
	// Vulkan 中的缓冲区是用于存储显卡可读取的任意数据的内存区域。它们可以用于多种目的，例如存储顶点数据，存储索引数据，存储一致性数据等。
	void createVertexBuffer() {
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

		// 现在，我们将更改 createVertexBuffer，使其仅使用主机可见缓冲区作为临时缓冲区，而使用设备本地缓冲区作为实际顶点缓冲区。
		// https://vulkan-tutorial.com/Vertex_buffers/Staging_buffer#page_Using-a-staging-buffer
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		// 创建临时缓冲区
		createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory);


		// 将数据复制到临时缓冲区
		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(device, stagingBufferMemory);

		// 创建顶点缓冲区
		createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			vertexBuffer,
			vertexBufferMemory);

		// 将数据从临时缓冲区复制到顶点缓冲区
		copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

		// 销毁临时缓冲区
		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	// 结合缓冲区的要求和自己应用程序的要求，找到合适的内存类型
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			// typeFilter 参数将用于指定合适内存类型的位字段。这意味着，我们只需遍历这些内存类型，并检查相应位是否设置为 1;
			// 同时检查该内存类型是否具有我们需要的属性。
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}

	// https://vulkan-tutorial.com/Vertex_buffers/Staging_buffer#page_Abstracting-buffer-creation
	void createBuffer(
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkBuffer& buffer,
		VkDeviceMemory& bufferMemory) {
		// 创建缓冲区
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size; // 缓冲区大小
		bufferInfo.usage = usage; // 缓冲区用途
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // 缓冲区共享模式

		if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create buffer!");
		}

		// 获取缓冲区内存需求
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

		// 为缓冲区分配内存
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size; // 内存大小
		// findMemoryType的第二个参数用于定义内存的特殊功能，例如可以映射内存，这样我们就可以从 CPU 向内存写入数据，而 GPU 就可以访问它了。
		// VK_MEMORY_PROPERTY_HOST_COHERENT_BIT表示使用与主机一致的内存堆，确保映射到GPU的内存始终与分配内存的内容相匹配。
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties); // 内存类型

		if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate buffer memory!");
		}

		// 将内存与缓冲区关联。第四个参数是内存区域内的偏移量。由于该内存是专门为顶点缓冲区分配的，因此偏移量仅为 0。
		vkBindBufferMemory(device, buffer, bufferMemory, 0);
	}

	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
		VkCommandBuffer commandBuffer = beginSingleTimeCommands(commandPool);

		// 复制缓冲区
		VkBufferCopy copyRegion{};
		copyRegion.size = size;
		// srcOffset和dstOffset参数指定要复制的字节偏移量。我们将从缓冲区的起始位置开始复制。
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		endSingleTimeCommands(commandBuffer, commandPool);

	}

	void createIndexBuffer() {
		VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		// 创建临时缓冲区
		createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory);

		// 将数据复制到临时缓冲区
		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), (size_t)bufferSize);
		vkUnmapMemory(device, stagingBufferMemory);

		// 
		createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			indexBuffer,
			indexBufferMemory);

		// 将数据从临时缓冲区复制到顶点缓冲区
		copyBuffer(stagingBuffer, indexBuffer, bufferSize);

		// 销毁临时缓冲区
		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	void createUniformBuffers() {
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);
		uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
		uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

		VkDeviceSize dicomBufferSize = sizeof(DicomUniformBufferObject);
		dicomUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		dicomUniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
		dicomUniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

		VkDeviceSize occBufferSize = sizeof(OcclusionUniformBufferObject);
		occlusionUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		occlusionUniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
		occlusionUniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

		VkDeviceSize gtBufferSize = sizeof(GroundTruthUBO);
		groundTruthRayUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		groundTruthRayUniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
		groundTruthRayUniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

		// 创建uniform缓冲区
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			createBuffer(
				bufferSize,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				uniformBuffers[i],
				uniformBuffersMemory[i]);
			// 创建缓冲区后，我们会立即使用 vkMapMemory 映射缓冲区，以获得一个指针，以便以后写入数据。在应用程序的整个生命周期中，缓冲区都会映射到这个指针。
			// 这种技术称为 "持久映射"，适用于所有 Vulkan 实现。由于映射不是免费的，因此无需在每次更新时映射缓冲区，从而提高了性能。
			// uniform data将用于所有绘制调用，因此包含uniform data的缓冲区只有在我们停止渲染时才会被销毁。
			vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);

			createBuffer(
				dicomBufferSize,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				dicomUniformBuffers[i],
				dicomUniformBuffersMemory[i]);
			vkMapMemory(device, dicomUniformBuffersMemory[i], 0, dicomBufferSize, 0, &dicomUniformBuffersMapped[i]);

			createBuffer(
				occBufferSize,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				occlusionUniformBuffers[i],
				occlusionUniformBuffersMemory[i]);
			vkMapMemory(device, occlusionUniformBuffersMemory[i], 0, occBufferSize, 0, &occlusionUniformBuffersMapped[i]);

			createBuffer(
				gtBufferSize,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				groundTruthRayUniformBuffers[i],
				groundTruthRayUniformBuffersMemory[i]);
			vkMapMemory(device, groundTruthRayUniformBuffersMemory[i], 0, gtBufferSize, 0, &groundTruthRayUniformBuffersMapped[i]);
		}
	}

	void createDescriptorPool() {
		// 创建描述符池
		std::array<VkDescriptorPoolSize, 4> poolSize{};
		poolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // 描述符类型
		poolSize[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * 4); // 描述符数量

		poolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; // 描述符类型
		poolSize[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * 2);

		poolSize[2].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT; // 描述符类型
		poolSize[2].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * 2);

		poolSize[3].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE; // 描述符类型
		poolSize[3].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * 2);

		// 描述符池信息
		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSize.size()); // 描述符池大小
		poolInfo.pPoolSizes = poolSize.data();
		poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * 10); // 描述符集数量

		if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	// https://vulkan-tutorial.com/Uniform_buffers/Descriptor_pool_and_sets#page_Descriptor-set
	void createDescriptorSets() {
		// ----------------backFace----------------
		// 创建描述符集布局 
		std::vector<VkDescriptorSetLayout> backFaceLayouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout.backFaceDescriptorSetLayout);
		VkDescriptorSetAllocateInfo backFaceAllocInfo{};
		backFaceAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		backFaceAllocInfo.descriptorPool = descriptorPool; // 描述符池
		backFaceAllocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT); // 描述符集数量
		backFaceAllocInfo.pSetLayouts = backFaceLayouts.data();

		descriptorSets.backFaceDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
		if (vkAllocateDescriptorSets(device, &backFaceAllocInfo, descriptorSets.backFaceDescriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate backFace descriptor sets!");
		}

		// ----------------composition----------------
		// 创建描述符集布局
		std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout.compositionDescriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool; // 描述符池
		allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT); // 描述符集数量
		allocInfo.pSetLayouts = layouts.data();

		descriptorSets.compositionDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
		if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.compositionDescriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate composition descriptor sets!");
		}

		// 更新描述符集
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = uniformBuffers[i]; // 缓冲区
			bufferInfo.offset = 0; // 偏移量
			bufferInfo.range = sizeof(UniformBufferObject); // 范围

			std::array<VkWriteDescriptorSet, 1> backFaceDescriptorWrite{};
			backFaceDescriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			backFaceDescriptorWrite[0].dstSet = descriptorSets.backFaceDescriptorSets[i]; // 描述符集
			backFaceDescriptorWrite[0].dstBinding = 0; // 描述符绑定
			backFaceDescriptorWrite[0].dstArrayElement = 0; // 描述符数组元素
			backFaceDescriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // 描述符类型
			backFaceDescriptorWrite[0].descriptorCount = 1; // 描述符数量
			backFaceDescriptorWrite[0].pBufferInfo = &bufferInfo;
			backFaceDescriptorWrite[0].pImageInfo = nullptr; // 图像信息
			backFaceDescriptorWrite[0].pTexelBufferView = nullptr; // 缓冲区视图

			vkUpdateDescriptorSets(device, static_cast<uint32_t>(backFaceDescriptorWrite.size()), backFaceDescriptorWrite.data(), 0, nullptr);

			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // 图像布局
			imageInfo.imageView = texture3DImageView; // 图像视图
			imageInfo.sampler = textureSampler; // 纹理采样器

			VkDescriptorImageInfo backFaceImageInfo{};
			backFaceImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // 图像布局
			backFaceImageInfo.imageView = backFaceImageView; // 图像视图
			backFaceImageInfo.sampler = VK_NULL_HANDLE; // *纹理采样器这里暂时为空

			VkDescriptorImageInfo lutImageInfo{};
			lutImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // 图像布局
			lutImageInfo.imageView = lutImageView; // 图像视图
			lutImageInfo.sampler = textureSampler; // 纹理采样器

			VkDescriptorBufferInfo dicomBufferInfo{};
			dicomBufferInfo.buffer = dicomUniformBuffers[i]; // 缓冲区
			dicomBufferInfo.offset = 0; // 偏移量
			dicomBufferInfo.range = sizeof(DicomUniformBufferObject); // 范围

			VkDescriptorImageInfo extCoefImageInfo{};
			extCoefImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL; // 图像布局
			extCoefImageInfo.imageView = textureTarget.imageView;
			extCoefImageInfo.sampler = textureSampler; // 纹理采样器

			VkDescriptorImageInfo texOccConeSectionsImageInfo{};
			texOccConeSectionsImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			texOccConeSectionsImageInfo.imageView = texOccConeSectionsInfo.imageView;
			texOccConeSectionsImageInfo.sampler = textureSampler;

			VkDescriptorBufferInfo occBufferInfo{};
			occBufferInfo.buffer = occlusionUniformBuffers[i]; // 缓冲区
			occBufferInfo.offset = 0; // 偏移量
			occBufferInfo.range = sizeof(OcclusionUniformBufferObject); // 范围

			VkDescriptorBufferInfo gtBufferInfo{};
			gtBufferInfo.buffer = groundTruthRayUniformBuffers[i]; // 缓冲区
			gtBufferInfo.offset = 0; // 偏移量
			gtBufferInfo.range = sizeof(GroundTruthUBO); // 范围

			std::array<VkWriteDescriptorSet, 9> descriptorWrite{};
			descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite[0].dstSet = descriptorSets.compositionDescriptorSets[i]; // 描述符集
			descriptorWrite[0].dstBinding = 0; // 描述符绑定
			descriptorWrite[0].dstArrayElement = 0; // 描述符数组元素
			descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // 描述符类型
			descriptorWrite[0].descriptorCount = 1; // 描述符数量
			descriptorWrite[0].pBufferInfo = &bufferInfo;
			descriptorWrite[0].pImageInfo = nullptr; // 图像信息
			descriptorWrite[0].pTexelBufferView = nullptr; // 缓冲区视图

			descriptorWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite[1].dstSet = descriptorSets.compositionDescriptorSets[i]; // 描述符集
			descriptorWrite[1].dstBinding = 1; // 描述符绑定
			descriptorWrite[1].dstArrayElement = 0; // 描述符数组元素
			descriptorWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; // 描述符类型
			descriptorWrite[1].descriptorCount = 1; // 描述符数量
			descriptorWrite[1].pBufferInfo = nullptr;
			descriptorWrite[1].pImageInfo = &imageInfo; // 图像信息
			descriptorWrite[1].pTexelBufferView = nullptr; // 缓冲区视图

			descriptorWrite[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite[2].dstSet = descriptorSets.compositionDescriptorSets[i]; // 描述符集
			descriptorWrite[2].dstBinding = 2; // 描述符绑定
			descriptorWrite[2].dstArrayElement = 0; // 描述符数组元素
			descriptorWrite[2].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT; // 描述符类型
			descriptorWrite[2].descriptorCount = 1; // 描述符数量
			descriptorWrite[2].pBufferInfo = nullptr;
			descriptorWrite[2].pImageInfo = &backFaceImageInfo; // *上一阶段的图像信息
			descriptorWrite[2].pTexelBufferView = nullptr; // 缓冲区视图

			descriptorWrite[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite[3].dstSet = descriptorSets.compositionDescriptorSets[i]; // 描述符集
			descriptorWrite[3].dstBinding = 3; // 描述符绑定
			descriptorWrite[3].dstArrayElement = 0; // 描述符数组元素
			descriptorWrite[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // 描述符类型
			descriptorWrite[3].descriptorCount = 1; // 描述符数量
			descriptorWrite[3].pBufferInfo = &dicomBufferInfo;
			descriptorWrite[3].pImageInfo = nullptr; // 图像信息
			descriptorWrite[3].pTexelBufferView = nullptr; // 缓冲区视图

			descriptorWrite[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite[4].dstSet = descriptorSets.compositionDescriptorSets[i]; // 描述符集
			descriptorWrite[4].dstBinding = 4; // 描述符绑定
			descriptorWrite[4].dstArrayElement = 0; // 描述符数组元素
			descriptorWrite[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; // 描述符类型
			descriptorWrite[4].descriptorCount = 1; // 描述符数量
			descriptorWrite[4].pBufferInfo = nullptr;
			descriptorWrite[4].pImageInfo = &lutImageInfo; // 图像信息
			descriptorWrite[4].pTexelBufferView = nullptr; // 缓冲区视图

			descriptorWrite[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite[5].dstSet = descriptorSets.compositionDescriptorSets[i]; // 描述符集
			descriptorWrite[5].dstBinding = 5; // 描述符绑定
			descriptorWrite[5].dstArrayElement = 0; // 描述符数组元素
			descriptorWrite[5].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; // 描述符类型
			descriptorWrite[5].descriptorCount = 1; // 描述符数量
			descriptorWrite[5].pBufferInfo = nullptr;
			descriptorWrite[5].pImageInfo = &extCoefImageInfo; // 图像信息
			descriptorWrite[5].pTexelBufferView = nullptr; // 缓冲区视图

			descriptorWrite[6].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite[6].dstSet = descriptorSets.compositionDescriptorSets[i]; // 描述符集
			descriptorWrite[6].dstBinding = 6; // 描述符绑定
			descriptorWrite[6].dstArrayElement = 0; // 描述符数组元素
			descriptorWrite[6].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; // 描述符类型
			descriptorWrite[6].descriptorCount = 1; // 描述符数量
			descriptorWrite[6].pBufferInfo = nullptr;
			descriptorWrite[6].pImageInfo = &texOccConeSectionsImageInfo; // 图像信息
			descriptorWrite[6].pTexelBufferView = nullptr; // 缓冲区视图

			descriptorWrite[7].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite[7].dstSet = descriptorSets.compositionDescriptorSets[i]; // 描述符集
			descriptorWrite[7].dstBinding = 7; // 描述符绑定
			descriptorWrite[7].dstArrayElement = 0; // 描述符数组元素
			descriptorWrite[7].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // 描述符类型
			descriptorWrite[7].descriptorCount = 1; // 描述符数量
			descriptorWrite[7].pBufferInfo = &occBufferInfo;
			descriptorWrite[7].pImageInfo = nullptr; // 图像信息
			descriptorWrite[7].pTexelBufferView = nullptr; // 缓冲区视图

			descriptorWrite[8].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite[8].dstSet = descriptorSets.compositionDescriptorSets[i]; // 描述符集
			descriptorWrite[8].dstBinding = 8; // 描述符绑定
			descriptorWrite[8].dstArrayElement = 0; // 描述符数组元素
			descriptorWrite[8].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // 描述符类型
			descriptorWrite[8].descriptorCount = 1; // 描述符数量
			descriptorWrite[8].pBufferInfo = &gtBufferInfo;
			descriptorWrite[8].pImageInfo = nullptr; // 图像信息
			descriptorWrite[8].pTexelBufferView = nullptr; // 缓冲区视图

			vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrite.size()), descriptorWrite.data(), 0, nullptr);
		}
	}

	void createCommandBuffers() {
		commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

		if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChainExtent;

		VkClearValue clearValues[2];
		clearValues[0].color = { 0.0f, 0.0f, 0.0f, 0.0f }; // 颜色
		clearValues[1].color = { 0.0f, 0.0f, 0.0f, 0.0f }; // 颜色
		renderPassInfo.clearValueCount = 2;
		renderPassInfo.pClearValues = clearValues;

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)swapChainExtent.width;
		viewport.height = (float)swapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = swapChainExtent;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		// 第一个subpass 只渲染立方体的背面
		{
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.offscreen);
			vkCmdBindDescriptorSets(
				commandBuffer, // 命令缓冲区
				VK_PIPELINE_BIND_POINT_GRAPHICS, // 管线绑定点
				pipelineLayout.offscreen, // 管线布局
				0, // 第一个描述符集绑定到顶点着色器
				1, // 描述符集数量
				&descriptorSets.backFaceDescriptorSets[currentFrame], // 描述符集
				0, // 动态偏移量数量
				nullptr); // 动态偏移量

			VkBuffer vertexBuffers[] = { vertexBuffer };
			VkDeviceSize offsets[] = { 0 };
			// 第2、3个参数指定了我们要指定顶点缓冲区的绑定偏移和数量
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

			vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);

			// vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);
			/*
			void vkCmdDrawIndexed(
				VkCommandBuffer                             commandBuffer, // 命令缓冲区
				uint32_t                                    indexCount, // 索引数量
				uint32_t                                    instanceCount, // 实例数量
				uint32_t                                    firstIndex, // 第一个索引
				int32_t                                     vertexOffset, // 顶点偏移
				uint32_t                                    firstInstance); // 第一个实例
			 */
			vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
		}

		// 第二个subpass 渲染立方体的正面，同时将上一个subpass的图像作为输入附件
		{
			vkCmdNextSubpass(commandBuffer, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.composition);

			vkCmdBindDescriptorSets(
				commandBuffer, // 命令缓冲区
				VK_PIPELINE_BIND_POINT_GRAPHICS, // 管线绑定点
				pipelineLayout.composition, // 管线布局
				0, // 第一个描述符集绑定到顶点着色器
				1, // 描述符集数量
				&descriptorSets.compositionDescriptorSets[currentFrame], // 描述符集
				0, // 动态偏移量数量
				nullptr); // 动态偏移量

			VkBuffer vertexBuffers[] = { vertexBuffer };
			VkDeviceSize offsets[] = { 0 };
			// 第2、3个参数指定了我们要指定顶点缓冲区的绑定偏移和数量
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
			vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);
			vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
		}

		vkCmdEndRenderPass(commandBuffer);

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}

	void createSyncObjects() {
		imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create synchronization objects for a frame!");
			}
		}
	}

	void updateUniformBuffer(uint32_t currentFrame) {
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		// 计算时间差
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		UniformBufferObject ubo{};
		// rotation 90 degrees per second
		// ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.model = glm::mat4(1.0f);
		// 根据相机的参数更新视图矩阵
		ubo.view = camera->getViewMatrix();
		ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);

		// GLM 最初是为 OpenGL 设计的，其中剪辑坐标的 Y 坐标是反转的。最简单的补偿方法是翻转投影矩阵中 Y 轴缩放因子的符号。如果不这样做，图像将被颠倒渲染
		ubo.proj[1][1] *= -1;

		// 将数据复制到映射的内存中
		memcpy(uniformBuffersMapped[currentFrame], &ubo, sizeof(ubo));

		// 更新dicomUniformBuffer
		DicomUniformBufferObject dicomUbo{};
		dicomUbo.voxelSize = static_cast<glm::vec3>(volumeRender->getDicomTags().voxelSize);
		dicomUbo.voxelResolution = static_cast<glm::vec3>(volumeRender->getDicomTags().voxelResolution);
		dicomUbo.boxSize = static_cast<glm::vec3>(volumeRender->getDicomTags().boxSize);
		dicomUbo.realSize = static_cast<glm::vec3>(volumeRender->getDicomTags().realSize);
		dicomUbo.windowCenter = static_cast<glm::vec1>(volumeRender->dicomParamControl.windowCenter);
		dicomUbo.windowWidth = static_cast<glm::vec1>(volumeRender->dicomParamControl.windowWidth);
		dicomUbo.minVal = static_cast<glm::vec1>(volumeRender->getDicomTags().minVal);
		dicomUbo.alphaCorrection = static_cast<glm::vec1>(volumeRender->dicomParamControl.alphaCorrection);
		dicomUbo.steps = static_cast<glm::int16>(volumeRender->dicomParamControl.steps);
		dicomUbo.stepLength = static_cast<glm::vec1>(volumeRender->dicomParamControl.stepLength);
		dicomUbo.glow = static_cast<glm::vec1>(volumeRender->dicomParamControl.glow);
		memcpy(dicomUniformBuffersMapped[currentFrame], &dicomUbo, sizeof(dicomUbo));

		// 更新occlusionUniformBuffer
		OcclusionUniformBufferObject occUbo{};
		//occUbo.OccInitialStep = static_cast<float>(volumeRender->sampler_occlusion.GetInitialStep());
		//occUbo.OccRay7AdjWeight = static_cast<float>(volumeRender->sampler_occlusion.GetRay7AdjacentWeight());
		std::vector<glm::vec3> occ_cone_axes;
		// std::vector<glm::vec4> test(10);
		occ_cone_axes.push_back(volumeRender->sampler_occlusion.Get3ConeRayID(0));
		occ_cone_axes.push_back(volumeRender->sampler_occlusion.Get3ConeRayID(1));
		occ_cone_axes.push_back(volumeRender->sampler_occlusion.Get3ConeRayID(2));
		occ_cone_axes.push_back(volumeRender->sampler_occlusion.Get7ConeRayID(0));
		occ_cone_axes.push_back(volumeRender->sampler_occlusion.Get7ConeRayID(1));
		occ_cone_axes.push_back(volumeRender->sampler_occlusion.Get7ConeRayID(2));
		occ_cone_axes.push_back(volumeRender->sampler_occlusion.Get7ConeRayID(3));
		occ_cone_axes.push_back(volumeRender->sampler_occlusion.Get7ConeRayID(4));
		occ_cone_axes.push_back(volumeRender->sampler_occlusion.Get7ConeRayID(5));
		occ_cone_axes.push_back(volumeRender->sampler_occlusion.Get7ConeRayID(6));
		for (size_t i = 0; i < 10; i++)
		{
			occUbo.OccConeRayAxes[i] = glm::vec4(occ_cone_axes[i], 1.0f);
			// test[i] = occUbo.OccConeRayAxes[i] * 255.0f;
		}
		//occUbo.OccConeIntegrationSamples[0] = static_cast<glm::int32>(volumeRender->sampler_occlusion.gaussian_samples_1);
		//occUbo.OccConeIntegrationSamples[1] = static_cast<glm::int32>(volumeRender->sampler_occlusion.gaussian_samples_3);
		//occUbo.OccConeIntegrationSamples[2] = static_cast<glm::int32>(volumeRender->sampler_occlusion.gaussian_samples_7);
		memcpy(occlusionUniformBuffersMapped[currentFrame], &occUbo, sizeof(occUbo));

		// 更新ground truth次级光线的uniformbuffer
		GroundTruthUBO gtUbo{};
		// std::vector<glm::vec4> testGtUbo(10);
		// std::vector<float> testGtUbo2(10);
		for (int i = 0; i < 10; i++)
		{
			gtUbo.raySampleVec[i] = glm::vec4(volumeRender->occ_kernel_vectors[i], 1.0f);
			// testGtUbo[i] = (gtUbo.raySampleVec[i] + glm::vec4(1.0f)) / 2.0f * 255.0f;
			// testGtUbo2[i] = gtUbo.raySampleVec[i].x * gtUbo.raySampleVec[i].x + gtUbo.raySampleVec[i].y * gtUbo.raySampleVec[i].y + gtUbo.raySampleVec[i].z * gtUbo.raySampleVec[i].z;
		}
		memcpy(groundTruthRayUniformBuffersMapped[currentFrame], &gtUbo, sizeof(gtUbo));
	}

	// https://vulkan-tutorial.com/Compute_Shader#page_Synchronizing-graphics-and-compute
	void drawFrame() {
		updateUniformBuffer(currentFrame);

		// --------------------Compute submission-----------------
		// !不必每一帧都提交计算命令，只有在计算命令完成之前才需要提交计算命令，如果之后修改了一些参数，只需要把isComplete设置为false即可
		if (!computeResources.isComplete) {
			// vkWaitForFences(device, 1, &computeResources.inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

			// update uniform buffer
			// ...

			// vkResetFences(device, 1, &computeResources.inFlightFences[currentFrame]);

			vkResetCommandBuffer(computeResources.commandBuffers[currentFrame], 0);
			// Build a single command buffer containing the compute dispatch commands
			recordComputeCommandBuffer(currentFrame);

			// Submit to the compute queue
			VkSubmitInfo computeSubmitInfo = {};
			computeSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			computeSubmitInfo.commandBufferCount = 1;
			computeSubmitInfo.pCommandBuffers = &computeResources.commandBuffers[currentFrame];
			computeSubmitInfo.signalSemaphoreCount = 1;
			computeSubmitInfo.pSignalSemaphores = &computeResources.finishedSemaphores[currentFrame]; // 计算命令完成时发送信号

			if (vkQueueSubmit(computeResources.queue, 1, &computeSubmitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
				throw std::runtime_error("failed to submit compute command buffer!");
			}

			computeResources.isComplete = true;

			recordGenExtCoffMipmaps(currentFrame);

			recordGenGaussianMipmaps();
		}


		// --------------------Graphics submission-----------------
		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
		VkResult waitRes = vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
		if (waitRes != VK_SUCCESS) {
			throw std::runtime_error("failed to wait for inFlightFences[currentFrame]!");
		}

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		// update uniform buffer
		camera->updateCameraMove(window);

		vkResetFences(device, 1, &inFlightFences[currentFrame]);

		vkResetCommandBuffer(commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);

		// Submit graphics commands
		recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

		VkSemaphore graphicSignalSemaphores[] = { renderFinishedSemaphores[currentFrame] }; // 图形命令完成时发送信号

		// if (!computeResources.isComplete[currentFrame]) {
		//     VkSemaphore graphicWaitSemaphores[] = { computeResources.finishedSemaphores[currentFrame], imageAvailableSemaphores[currentFrame] };
		//     VkPipelineStageFlags graphicWaitStages[] = { VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT , VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		//     VkSubmitInfo graphicsSubmitInfo{};
		//     graphicsSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		//     std::array<VkCommandBuffer, 1> submitCommandBuffers = { commandBuffers[currentFrame] };
		//     graphicsSubmitInfo.waitSemaphoreCount = 2;
		//     graphicsSubmitInfo.pWaitSemaphores = graphicWaitSemaphores;
		//     graphicsSubmitInfo.pWaitDstStageMask = graphicWaitStages;
		//     graphicsSubmitInfo.commandBufferCount = static_cast<uint32_t>(submitCommandBuffers.size());
		//     graphicsSubmitInfo.pCommandBuffers = submitCommandBuffers.data();
		//     graphicsSubmitInfo.signalSemaphoreCount = 1;
		//     graphicsSubmitInfo.pSignalSemaphores = graphicSignalSemaphores;

		//     if (vkQueueSubmit(graphicsQueue, 1, &graphicsSubmitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
		//         throw std::runtime_error("failed to submit draw command buffer!");
		//     }

		//     std::cout << "compute not complete" << std::endl;

		//     recordGenExtCoffMipmaps(currentFrame);
		// }
		// else {
		// std::cout << "compute complete" << std::endl;
		
		VkSemaphore graphicWaitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
		VkPipelineStageFlags graphicWaitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		VkSubmitInfo graphicsSubmitInfo{};
		graphicsSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		std::array<VkCommandBuffer, 1> submitCommandBuffers = { commandBuffers[currentFrame] };
		graphicsSubmitInfo.waitSemaphoreCount = 1;
		graphicsSubmitInfo.pWaitSemaphores = graphicWaitSemaphores;
		graphicsSubmitInfo.pWaitDstStageMask = graphicWaitStages;
		graphicsSubmitInfo.commandBufferCount = static_cast<uint32_t>(submitCommandBuffers.size());
		graphicsSubmitInfo.pCommandBuffers = submitCommandBuffers.data();
		graphicsSubmitInfo.signalSemaphoreCount = 1;
		graphicsSubmitInfo.pSignalSemaphores = graphicSignalSemaphores;

		if (vkQueueSubmit(graphicsQueue, 1, &graphicsSubmitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}
		// }

		// --------------------捕获图片--------------------
		if (isNeedCapture)
		{
			captureImage();
			isNeedCapture = false;
		}


		// --------------------ImGui submission-----------------
		VkResult waitImguiRes = vkWaitForFences(device, 1, &imguiInFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
		if (waitImguiRes != VK_SUCCESS) {
			throw std::runtime_error("failed to wait for imguiInFlightFences[currentFrame]!");
		}

		vkResetFences(device, 1, &imguiInFlightFences[currentFrame]);

		vkResetCommandBuffer(imguiCommandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
		// record UI command buffer
		recordImGuiCommandBuffer(imguiCommandBuffers[currentFrame], imageIndex);
		VkSemaphore waitSemaphores[] = { renderFinishedSemaphores[currentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		VkSubmitInfo imguiSubmitInfo{};
		imguiSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		imguiSubmitInfo.waitSemaphoreCount = 1;
		imguiSubmitInfo.pWaitSemaphores = waitSemaphores;
		imguiSubmitInfo.pWaitDstStageMask = waitStages;
		imguiSubmitInfo.commandBufferCount = 1;
		imguiSubmitInfo.pCommandBuffers = &imguiCommandBuffers[currentFrame];
		imguiSubmitInfo.signalSemaphoreCount = 1;
		imguiSubmitInfo.pSignalSemaphores = &imguiFinishedSemaphores[currentFrame]; // UI命令完成时发送信号

		if (vkQueueSubmit(graphicsQueue, 1, &imguiSubmitInfo, imguiInFlightFences[currentFrame]) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit imgui command buffer!");
		}

		// --------------------将渲染结果提交到交换链，以便将图像显示到屏幕上--------------------
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &imguiFinishedSemaphores[currentFrame];

		VkSwapchainKHR swapChains[] = { swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		result = vkQueuePresentKHR(presentQueue, &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
			framebufferResized = false;
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}

		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;


	}

	VkShaderModule createShaderModule(const std::vector<unsigned char>& code) {
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module!");
		}

		return shaderModule;
	}

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_R8G8B8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
		if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)()) {
			return capabilities.currentExtent;
		}
		else {
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	bool isDeviceSuitable(VkPhysicalDevice device) {
		QueueFamilyIndices indices = findQueueFamilies(device);

		bool extensionsSupported = checkDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (extensionsSupported) {
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		// 检查设备是否支持所需的队列族
		VkPhysicalDeviceFeatures supportedFeatures;
		// vkGetPhysicalDeviceFeatures 重新利用了 VkPhysicalDeviceFeatures 结构，通过设置布尔值来指示支持哪些功能，而不是请求哪些功能。
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

		return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
	}

	bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies) {
			// 检查队列族是否支持 VK_QUEUE_GRAPHICS_BIT 标志和 VK_QUEUE_COMPUTE_BIT 标志
			if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) && (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
				indices.graphicsFamily = i;
				indices.computeFamily = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

			if (presentSupport) {
				indices.presentFamily = i;
			}

			if (indices.isComplete()) {
				break;
			}

			i++;
		}

		return indices;
	}

	std::vector<const char*> getRequiredExtensions() {
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (enableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}

	bool checkValidationLayerSupport() {
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : validationLayers) {
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound) {
				return false;
			}
		}

		return true;
	}

	static std::vector<char> readFile(const std::string& filename) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}
};

