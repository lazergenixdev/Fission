#include <Fission/PlatformConfig.hpp>
#if defined(FISSION_PLATFORM_WINDOWS)
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#include <Fission/Core/Engine.hh>
#include <Fission/Core/Console.hh>
#include "Version.h"
#include <optional>
#include <algorithm>
#include <format>

extern fs::Engine engine;

#ifdef PROFILE
#include "profiler.hpp"
#define SCOPED_TRACE(NAME) profiler::scoped_trace __trace{session.get(), 0, NAME}
extern std::unique_ptr<profiler::Session> session;
#else
#define SCOPED_TRACE(NAME)
#endif

#undef assert // yeah? fuck you too
#define count32 (uint32_t)std::size
#define foru32(N) for (u32 i = 0; i < N; ++i)

void display_fatal_graphics_error(VkResult r, char const* what);
void display_fatal_graphics_error(char const* what);
#define check_result(R, WHAT) if (VkResult _result = (R)) { display_fatal_graphics_error(_result, WHAT); return true; } (void)0

VKAPI_ATTR VkBool32 VKAPI_CALL on_graphics_vaidation_error(
	VkDebugUtilsMessageSeverityFlagBitsEXT       messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT              messageType,
	const VkDebugUtilsMessengerCallbackDataEXT*  pCallbackData,
	void*                                        pUserData
);

template <typename T>
struct vk_ptr {
	T handle;
	constexpr vk_ptr(): handle(nullptr) {}
	constexpr operator T&() { return handle; }
	constexpr T release() { auto h = handle; handle = nullptr; return h; }
	constexpr T* operator& () { return &handle; }
};

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

struct Queue_Indicies {
	std::optional<uint32_t> graphics;
	std::optional<uint32_t> present;
};
Queue_Indicies findDeviceQueueFamilies(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
	Queue_Indicies families;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queueFamilyCount, queueFamilies.data());

	for (uint32_t i = 0; i < queueFamilyCount; ++i) {
		const auto flags = queueFamilies[i].queueFlags;

		if (!families.graphics.has_value() && (flags & VK_QUEUE_GRAPHICS_BIT))
			families.graphics = std::make_optional(i);

		VkBool32 supports_surface = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &supports_surface);

		if (!families.present.has_value() && supports_surface)
			families.present = std::make_optional(i);
	}

	return families;
}
fs::u32 _graphics_queue_family_index;

struct SwapChainResult {
	VkResult result;
	VkExtent2D extent;
	VkFormat format;
};
SwapChainResult createSwapChain(VkPhysicalDevice pdevice, VkDevice device, VkSurfaceKHR surface, VkPresentModeKHR pm, int width, int height, VkSwapchainKHR old, VkSwapchainKHR* swap_chain)
{
	SwapChainResult r;
	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities{};
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	} details;

	auto queue_families = findDeviceQueueFamilies(pdevice, surface);

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pdevice, surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(pdevice, surface, &formatCount, nullptr);

	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(pdevice, surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(pdevice, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(pdevice, surface, &presentModeCount, details.presentModes.data());
	}

	VkSwapchainCreateInfoKHR createInfo{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
	createInfo.preTransform = details.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.clipped = VK_TRUE; /* "... allows more efficient presentation methods to be used on some platforms." */
	createInfo.surface = surface;
	createInfo.imageArrayLayers = 1; /* For non-stereoscopic-3D applications, this value is 1 */
	createInfo.oldSwapchain = old; // first time?
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // might want only transfer here

	uint32_t queueFamilyIndices[] = {*queue_families.graphics, *queue_families.present};
	if (*queue_families.graphics != *queue_families.present) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	auto chooseSwapSurfaceFormat = [](std::vector<VkSurfaceFormatKHR> const& availableFormats) {
		for (auto const& availableFormat : availableFormats)
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				return availableFormat;
		return availableFormats.front();
	};

	{
		auto surface_format = chooseSwapSurfaceFormat(details.formats);
		createInfo.imageFormat = (r.format = surface_format.format);
		createInfo.imageColorSpace = surface_format.colorSpace;
	}

	auto chooseSwapExtent = [](int width, int height, VkSurfaceCapabilitiesKHR const& capabilities) {
		if (capabilities.currentExtent.width != 0xFFFFFFFF)
			return capabilities.currentExtent;

		VkExtent2D extent = {(uint32_t)width, (uint32_t)height};
		extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
		return extent;
	};
	createInfo.imageExtent = (r.extent = chooseSwapExtent(width, height, details.capabilities));

	auto chooseSwapPresentMode = [](std::vector<VkPresentModeKHR> const& availablePresentModes, VkPresentModeKHR desiredMode) {
		for (auto const& availablePresentMode : availablePresentModes)
			if (availablePresentMode == desiredMode)
				return availablePresentMode;
		return VK_PRESENT_MODE_FIFO_KHR;
	};
	createInfo.presentMode = chooseSwapPresentMode(details.presentModes, pm);
	engine.graphics.sc_present_mode = createInfo.presentMode;

	uint32_t imageCount = details.capabilities.minImageCount + 1;
	if (details.capabilities.maxImageCount > 0 && imageCount > details.capabilities.maxImageCount) {
		imageCount = details.capabilities.maxImageCount;
	}
	createInfo.minImageCount = imageCount;

	r.result = vkCreateSwapchainKHR(device, &createInfo, nullptr, swap_chain);
	return r;
}

__FISSION_BEGIN__

bool Graphics::create(Graphics_Create_Info* info)
{
	{
		SCOPED_TRACE("vkCreateInstance");
		VkInstanceCreateInfo info{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};

		VkApplicationInfo appInfo{VK_STRUCTURE_TYPE_APPLICATION_INFO};
		appInfo.apiVersion = VK_API_VERSION_1_3;
		appInfo.pApplicationName = "How did you find this?";
		appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 0, 1, 69);
		appInfo.pEngineName = "Fission";
		appInfo.engineVersion = VK_MAKE_API_VERSION(0, FISSION_VERSION_MAJ, FISSION_VERSION_MIN, FISSION_VERSION_PAT);
		info.pApplicationInfo = &appInfo;

		const char* Extensions[] = {
			VK_KHR_SURFACE_EXTENSION_NAME,
#if defined(FISSION_PLATFORM_WINDOWS)
			VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
#ifdef FISSION_DEBUG
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
		};

		info.enabledExtensionCount = count32(Extensions);
		info.ppEnabledExtensionNames = Extensions;

#ifdef FISSION_DEBUG
		const char* Layers[] = {
			"VK_LAYER_KHRONOS_validation",
		};
		info.enabledLayerCount = count32(Layers);
		info.ppEnabledLayerNames = Layers;

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
		debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugCreateInfo.pfnUserCallback = on_graphics_vaidation_error;
		debugCreateInfo.pUserData;
		info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		check_result(vkCreateInstance(&info, nullptr, &instance), "Vulkan SDK must be installed to enable validation layer [vkCreateInstance]");
#else
		check_result(vkCreateInstance(&info, nullptr, &instance), "Failed to create instance");
#endif
	}

#ifdef FISSION_DEBUG
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo{VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = on_graphics_vaidation_error;
		createInfo.pUserData;

		check_result(CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger), "Failed to create debug messenger");
	}
#endif

	{
#if defined(FISSION_PLATFORM_WINDOWS)
		SCOPED_TRACE("vkCreateWin32SurfaceKHR");
		VkWin32SurfaceCreateInfoKHR surfaceInfo{VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
		surfaceInfo.hwnd = info->window->_handle;
		surfaceInfo.hinstance = GetModuleHandleW(nullptr);
		check_result(vkCreateWin32SurfaceKHR(instance, &surfaceInfo, nullptr, &surface), "Failed to create Win32 surface");
#endif
	}

	{
		SCOPED_TRACE("vkCreateWin32SurfaceKHR");
		u32 deviceCount;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

		if (deviceCount == 0) {
			display_fatal_graphics_error("Unable to find graphics device with Vulkan support!");
			return true;
		}

		std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());

		physical_device = physicalDevices[0];
#if 1 // Need to actually choose GPU we want to use
		for (auto&& dev : physicalDevices) {
			VkPhysicalDeviceProperties props;
			vkGetPhysicalDeviceProperties(dev, &props);
			
			VkPhysicalDeviceFeatures deviceFeatures;
			vkGetPhysicalDeviceFeatures(dev, &deviceFeatures);

			auto dt = [](VkPhysicalDeviceType t) {
				switch (t)
				{
				case VK_PHYSICAL_DEVICE_TYPE_OTHER:          return "Other";
				case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return "Integrated GPU";
				case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:   return "Descrete GPU";
				case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:    return "Virtual GPU";
				case VK_PHYSICAL_DEVICE_TYPE_CPU:            return "CPU";
				default:                                     return "Unknown";
				}
			};

			auto gpu_info = std::format("GPU: {}\n\ttype: {}\n", props.deviceName, dt(props.deviceType));

 			console::print(FS_str_std(gpu_info));
		}
#endif
	}

	const auto queue_families = findDeviceQueueFamilies(physical_device, surface);
	{
		SCOPED_TRACE("vkCreateDevice");
		if (!queue_families.graphics) {
			display_fatal_graphics_error("Unable to find queue family that supports GRAPHICS_BIT!");
			return true;
		}
		if (!queue_families.present) {
			display_fatal_graphics_error("Unable to find queue family that supports presentation for our surface!");
			return true;
		}

		float queuePriority = 1.0f;

		VkDeviceQueueCreateInfo queue_create_infos[2] = {
			{.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, .queueFamilyIndex = *queue_families.graphics, .queueCount = 1, .pQueuePriorities = &queuePriority},
			{.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, .queueFamilyIndex = *queue_families.present,  .queueCount = 1, .pQueuePriorities = &queuePriority},
		};

		uint32_t number_of_unique_queues = (*queue_families.graphics == *queue_families.present)? 1 : 2;

		VkDeviceCreateInfo deviceInfo{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
		deviceInfo.pQueueCreateInfos = queue_create_infos;
		deviceInfo.queueCreateInfoCount = number_of_unique_queues;

		const std::vector<const char*> deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		//	VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME,
		};
		deviceInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		deviceInfo.ppEnabledExtensionNames = deviceExtensions.data();

#ifdef FISSION_DEBUG
		/*
* 		  https://vulkan-tutorial.com/en/Drawing_a_triangle/Setup/Logical_device_and_queues
		  "Previous implementations of Vulkan made a distinction between instance
		  and device specific validation layers, but this is no longer the case.
		  That means that the enabledLayerCount and ppEnabledLayerNames fields
		  of VkDeviceCreateInfo are ignored by up-to-date implementations.
		  However, it is still a good idea to set them anyway to be compatible
		  with older implementations"
		*/
		const char* Layers[] = { "VK_LAYER_KHRONOS_validation" };
		deviceInfo.enabledLayerCount = count32(Layers);
		deviceInfo.ppEnabledLayerNames = Layers;
#endif

		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.fillModeNonSolid = VK_TRUE;
		deviceFeatures.sampleRateShading = VK_TRUE;
		deviceInfo.pEnabledFeatures = &deviceFeatures;

		check_result(vkCreateDevice(physical_device, &deviceInfo, nullptr, &device), "Failed to create device");

		vkGetDeviceQueue(device, *queue_families.graphics, 0, &graphics_queue);
		vkGetDeviceQueue(device, *queue_families.present , 0, &present_queue);
	}

	{
		SCOPED_TRACE("createSwapChain");
		auto&& [result, extent, format] = createSwapChain(
			physical_device,
			device,
			surface,
			VK_PRESENT_MODE_FIFO_RELAXED_KHR,
			info->window->width,
			info->window->height,
			VK_NULL_HANDLE,
			&swap_chain
		);
		check_result(result, "Failed to create swap chain");
		sc_extent = extent;
		sc_format = format;
	}
	
	{
		VkImage swap_chain_images[Graphics::max_sc_images];

		vkGetSwapchainImagesKHR(device, swap_chain, &sc_image_count, nullptr);
		
		if(sc_image_count > Graphics::max_sc_images) {
			display_fatal_graphics_error(
				std::format("image count is {}, but expected to be less than or equal to {}", sc_image_count, Graphics::max_sc_images)
				.c_str());
			return true;
		}
		vkGetSwapchainImagesKHR(device, swap_chain, &sc_image_count, swap_chain_images);

		{
			auto createInfo = vk::image_view_2d(VK_NULL_HANDLE, sc_format);
			foru32(sc_image_count) {
				createInfo.image = swap_chain_images[i];
				check_result(vkCreateImageView(device, &createInfo, nullptr, sc_image_views + i), "Failed to create swap chain ImageView");
			}
		}

		// This is seriously aweful, remove ASAP
		VkRenderPass temp_render_pass;
		{
			VkAttachmentDescription colorAttachment{};
			colorAttachment.format = sc_format;
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			VkAttachmentReference colorAttachmentRef{};
			colorAttachmentRef.attachment = 0;
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &colorAttachmentRef;

			VkSubpassDependency dependency{};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			VkRenderPassCreateInfo renderPassInfo{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
			renderPassInfo.attachmentCount = 1;
			renderPassInfo.pAttachments = &colorAttachment;
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.dependencyCount = 1;
			renderPassInfo.pDependencies = &dependency;

			check_result(vkCreateRenderPass(device, &renderPassInfo, nullptr, &temp_render_pass), "Failed to create render pass");
		}

		{
			VkFramebufferCreateInfo framebufferInfo{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
			framebufferInfo.renderPass = temp_render_pass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.width  = sc_extent.width;
			framebufferInfo.height = sc_extent.height;
			framebufferInfo.layers = 1;

			foru32(sc_image_count) {
				framebufferInfo.pAttachments = sc_image_views + i;
				check_result(vkCreateFramebuffer(device, &framebufferInfo, nullptr, sc_framebuffers + i), "Failed to create Framebuffer");
			}
		}

		vkDestroyRenderPass(device, temp_render_pass, nullptr);
	}

	{
		VkCommandPoolCreateInfo poolInfo{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = *queue_families.graphics;
		check_result(vkCreateCommandPool(device, &poolInfo, nullptr, &command_pool), "Failed to create command pool");
	}
	//VkCommandPool transfer_command_pool;
	//{
	//	VkCommandPoolCreateInfo poolInfo{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
	//	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	//	poolInfo.queueFamilyIndex = *queue_families.transfer;
	//	check_result(vkCreateCommandPool(device, &poolInfo, nullptr, &transfer_command_pool), "Failed to create command pool");
	//}

	{
		VkCommandBufferAllocateInfo allocInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
		allocInfo.commandPool = command_pool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = count32(command_buffers);
		check_result(vkAllocateCommandBuffers(device, &allocInfo, command_buffers), "Failed to allocate command buffers");
	}

	{
		VkSemaphoreCreateInfo semaphoreInfo{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
		foru32(2) {
			check_result(vkCreateSemaphore(device, &semaphoreInfo, nullptr,  sc_image_read_semaphore + i), "Failed to create semaphore");
			check_result(vkCreateSemaphore(device, &semaphoreInfo, nullptr, sc_image_write_semaphore + i), "Failed to create semaphore");
		}
	}

	{
		VkFenceCreateInfo fenceInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		foru32(2)
		check_result(vkCreateFence(device, &fenceInfo, nullptr, cb_fences + i), "Failed to create fence");
	}

	{
		SCOPED_TRACE("vmaCreateAllocator");
		VmaAllocatorCreateInfo allocatorCreateInfo = {
			.flags = VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT,
			.physicalDevice = physical_device,
			.device = device,
			.instance = instance,
		};
		check_result(vmaCreateAllocator(&allocatorCreateInfo, &allocator), "Failed to create Vulkan Memory Allocator");
	}

	_graphics_queue_family_index = *queue_families.graphics;

	return false;
}

void Graphics::recreate_swap_chain(Window* wnd, VkPresentModeKHR pm) {
	if (engine.window.is_minimized()) {
		std::unique_lock lock(engine._mutex);
		engine._event.wait(lock);
	}
	vkDeviceWaitIdle(device);
	foru32(sc_image_count) {
		vkDestroyImageView(device, sc_image_views[i], nullptr);
		vkDestroyFramebuffer(device, sc_framebuffers[i], nullptr);
	}
	vkDestroySwapchainKHR(device, swap_chain, nullptr);
//	VkSwapchainKHR old = swap_chain;
	auto&& [r, extent, format]
		= createSwapChain(physical_device, device, surface, pm, wnd->width, wnd->height, VK_NULL_HANDLE, &swap_chain);
//	vkDestroySwapchainKHR(device, old, nullptr);

	if (r) display_fatal_graphics_error("this is not supposed to happen");
#ifdef FISSION_DEBUG
	OutputDebugStringA("recreated swap chain\n");
#endif

	sc_extent = extent;
	sc_format = format;

	// rip clean-up of these
	VkImageView   swap_chain_images_views[Graphics::max_sc_images];
	VkFramebuffer swap_chain_framebuffers[Graphics::max_sc_images];
	u32 swap_chain_image_count;
	{
		VkImage swap_chain_images[Graphics::max_sc_images];

		vkGetSwapchainImagesKHR(device, swap_chain, &swap_chain_image_count, nullptr);
		vkGetSwapchainImagesKHR(device, swap_chain, &swap_chain_image_count, swap_chain_images);

		{
			auto createInfo = vk::image_view_2d(VK_NULL_HANDLE, sc_format);
			foru32(swap_chain_image_count) {
				createInfo.image = swap_chain_images[i];
				vkCreateImageView(device, &createInfo, nullptr, swap_chain_images_views + i);
			}
		}

		VkRenderPass temp_render_pass;
		{
			VkAttachmentDescription colorAttachment{};
			colorAttachment.format = format;
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			VkAttachmentReference colorAttachmentRef{};
			colorAttachmentRef.attachment = 0;
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &colorAttachmentRef;

			VkSubpassDependency dependency{};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			VkRenderPassCreateInfo renderPassInfo{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
			renderPassInfo.attachmentCount = 1;
			renderPassInfo.pAttachments = &colorAttachment;
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.dependencyCount = 1;
			renderPassInfo.pDependencies = &dependency;

			vkCreateRenderPass(device, &renderPassInfo, nullptr, &temp_render_pass);
		}

		{
			VkFramebufferCreateInfo framebufferInfo{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
			framebufferInfo.renderPass = temp_render_pass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.width = extent.width;
			framebufferInfo.height = extent.height;
			framebufferInfo.layers = 1;

			foru32(swap_chain_image_count) {
				framebufferInfo.pAttachments = swap_chain_images_views + i;
				vkCreateFramebuffer(device, &framebufferInfo, nullptr, swap_chain_framebuffers + i);
			}
		}

		vkDestroyRenderPass(device, temp_render_pass, nullptr);
	}

	memcpy(this->sc_image_views, swap_chain_images_views, sizeof(swap_chain_images_views));
	memcpy(this->sc_framebuffers, swap_chain_framebuffers, sizeof(swap_chain_framebuffers));
	sc_image_count = swap_chain_image_count;
}

void Graphics::upload_buffer(VkBuffer dstBuffer, void const* inData, VkDeviceSize inSize)
{
	VkBuffer      stagingBuffer{};
	VmaAllocation stagingAllocation{};

	VmaAllocationCreateInfo allocInfo {
		.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
		.usage = VMA_MEMORY_USAGE_AUTO,
	};
	VkBufferCreateInfo bufferInfo {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = inSize,
		.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	};
	::vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &stagingBuffer, &stagingAllocation, nullptr);

	{
		void* dst;
		::vmaMapMemory(allocator, stagingAllocation, &dst);
		memcpy(dst, inData, inSize);
		vmaUnmapMemory(allocator, stagingAllocation);
		vmaFlushAllocation(allocator, stagingAllocation, 0, VK_WHOLE_SIZE);
	}

	// TODO: Terrible code, who the fuck wrote this??
	VkCommandPool cmdPool;
	VkCommandPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
	poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	poolInfo.queueFamilyIndex = _graphics_queue_family_index;
	vkCreateCommandPool(device, &poolInfo, nullptr, &cmdPool);

	VkCommandBuffer cmd;
	VkCommandBufferAllocateInfo commandBufferInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
	commandBufferInfo.commandBufferCount = 1;
	commandBufferInfo.commandPool = cmdPool;
	commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	vkAllocateCommandBuffers(device, &commandBufferInfo, &cmd);

	VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(cmd, &beginInfo);
	VkBufferCopy region{
		.srcOffset = 0,
		.dstOffset = 0,
		.size = inSize,
	};
	vkCmdCopyBuffer(cmd, stagingBuffer, dstBuffer, 1, &region);
	vkEndCommandBuffer(cmd);

	VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmd;
	::vkQueueSubmit(graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);

	::vkQueueWaitIdle(graphics_queue);
	vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);
	vkDestroyCommandPool(device, cmdPool, nullptr);
}

void Graphics::upload_image(VkImage dstImage, void* inData, VkExtent3D extent) {
	VkDeviceSize  data_size = extent.width * extent.height * extent.depth * 4;
	VkBuffer      stagingBuffer{};
	VmaAllocation stagingAllocation{};

	VmaAllocationCreateInfo allocInfo{
		.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
		.usage = VMA_MEMORY_USAGE_AUTO,
	};
	VkBufferCreateInfo bufferInfo{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = data_size,
		.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	};
	::vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &stagingBuffer, &stagingAllocation, nullptr);

	{
		void* dst;
		::vmaMapMemory(allocator, stagingAllocation, &dst);
		memcpy(dst, inData, data_size);
		vmaUnmapMemory(allocator, stagingAllocation);
		vmaFlushAllocation(allocator, stagingAllocation, 0, VK_WHOLE_SIZE);
	}

	VkCommandPool cmdPool;
	VkCommandPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
	poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	poolInfo.queueFamilyIndex = _graphics_queue_family_index;
	vkCreateCommandPool(device, &poolInfo, nullptr, &cmdPool);

	VkCommandBuffer cmd;
	VkCommandBufferAllocateInfo commandBufferInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	commandBufferInfo.commandBufferCount = 1;
	commandBufferInfo.commandPool = cmdPool;
	commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	vkAllocateCommandBuffers(device, &commandBufferInfo, &cmd);

	VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(cmd, &beginInfo);

	VkImageSubresourceRange range;
	range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	range.baseMipLevel = 0;
	range.levelCount = 1;
	range.baseArrayLayer = 0;
	range.layerCount = 1;
	VkImageMemoryBarrier imageBarrier_toTransfer = {};
	imageBarrier_toTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageBarrier_toTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageBarrier_toTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imageBarrier_toTransfer.image = dstImage;
	imageBarrier_toTransfer.subresourceRange = range;
	imageBarrier_toTransfer.srcAccessMask = 0;
	imageBarrier_toTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toTransfer);

	VkBufferImageCopy copyRegion{};
	copyRegion.bufferOffset = 0;
	copyRegion.bufferRowLength = 0;
	copyRegion.bufferImageHeight = 0;
	copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copyRegion.imageSubresource.mipLevel = 0;
	copyRegion.imageSubresource.baseArrayLayer = 0;
	copyRegion.imageSubresource.layerCount = 1;
	copyRegion.imageExtent = extent;
	vkCmdCopyBufferToImage(cmd, stagingBuffer, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

	VkImageMemoryBarrier imageBarrier_toReadable{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
	imageBarrier_toReadable.image = dstImage;
	imageBarrier_toReadable.subresourceRange = range;
	imageBarrier_toReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imageBarrier_toReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageBarrier_toReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	imageBarrier_toReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toReadable);
	vkEndCommandBuffer(cmd);

	VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmd;
	::vkQueueSubmit(graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);

	::vkQueueWaitIdle(graphics_queue);
	vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);
	vkDestroyCommandPool(device, cmdPool, nullptr);
}

version Graphics::get_api_version() {
	uint32_t instanceVersion;
	
	auto enumerateInstanceVersion = (PFN_vkEnumerateInstanceVersion)vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceVersion");
	if (enumerateInstanceVersion == nullptr)
		return version(1,0,0);
	
	if (enumerateInstanceVersion(&instanceVersion) != VK_SUCCESS)
		return version();

	return {
		VK_API_VERSION_MAJOR(instanceVersion),
		VK_API_VERSION_MINOR(instanceVersion),
		VK_API_VERSION_PATCH(instanceVersion),
	};
}

Graphics::~Graphics() {
	if(allocator) vmaDestroyAllocator(allocator);

	if (command_pool) {
		foru32(2) {
			vkDestroySemaphore(device, sc_image_write_semaphore[i], nullptr);
			vkDestroySemaphore(device, sc_image_read_semaphore[i], nullptr);
			vkDestroyFence(device, cb_fences[i], nullptr);
		}
		vkDestroyCommandPool(device, command_pool, nullptr);
	}

	foru32(sc_image_count) {
		vkDestroyImageView(device, sc_image_views[i], nullptr);
		vkDestroyFramebuffer(device, sc_framebuffers[i], nullptr);
	}

	if (swap_chain) {
		vkDestroySwapchainKHR(device, swap_chain, nullptr);
	}

	if (device) {
		vkDestroyDevice(device, nullptr);
	}
#ifdef FISSION_DEBUG
	if (debugMessenger) {
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	}
#endif
	if (surface) {
		vkDestroySurfaceKHR(instance, surface, nullptr);
	}
	if (instance) {
		vkDestroyInstance(instance, nullptr);
	}
}

void Render_Pass::create(VkSampleCountFlagBits samples, bool clear)
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = engine.graphics.sc_format;
	colorAttachment.samples = samples;
	colorAttachment.loadOp = clear? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = clear? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	colorAttachment.finalLayout = (clear||samples != VK_SAMPLE_COUNT_1_BIT)? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentDescription colorAttachmentResolve{};
	colorAttachmentResolve.format = engine.graphics.sc_format;
	colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentResolveRef{};
	colorAttachmentResolveRef.attachment = 1;
	colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	if (samples != VK_SAMPLE_COUNT_1_BIT)
	subpass.pResolveAttachments = &colorAttachmentResolveRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	auto attachments = {colorAttachment, colorAttachmentResolve};

	VkRenderPassCreateInfo renderPassInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
	renderPassInfo.attachmentCount = (samples == VK_SAMPLE_COUNT_1_BIT)? 1:2;
	renderPassInfo.pAttachments = attachments.begin();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	vkCreateRenderPass(engine.graphics.device, &renderPassInfo, nullptr, &handle);
}
void Render_Pass::destroy() {
	vkDestroyRenderPass(engine.graphics.device, handle, nullptr);
}
void Render_Pass::begin(Render_Context* ctx, VkFramebuffer fb, color clear) {
	VkClearValue clear_value;
	clear_value.color = {clear.r, clear.g, clear.b, clear.a};
	VkRenderPassBeginInfo beginInfo{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
	beginInfo.framebuffer = fb;
	beginInfo.renderPass = handle;
	beginInfo.clearValueCount = 1;
	beginInfo.pClearValues = &clear_value;
	beginInfo.renderArea.extent = ctx->gfx->sc_extent;
	beginInfo.renderArea.offset = {0, 0};
	vkCmdBeginRenderPass(ctx->command_buffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}
void Render_Pass::begin(Render_Context* ctx, color clear) {
	begin(ctx, ctx->frame_buffer, clear);
}
void Render_Pass::begin(Render_Context* ctx) {
	VkRenderPassBeginInfo beginInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
	beginInfo.framebuffer = ctx->frame_buffer;
	beginInfo.renderPass = handle;
	beginInfo.clearValueCount = 0;
	beginInfo.renderArea.extent = ctx->gfx->sc_extent;
	beginInfo.renderArea.offset = { 0, 0 };
	vkCmdBeginRenderPass(ctx->command_buffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}
void Render_Pass::end(Render_Context* ctx) {
	vkCmdEndRenderPass(ctx->command_buffer);
}

__FISSION_END__