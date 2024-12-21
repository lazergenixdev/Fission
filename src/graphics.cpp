#include "internal.hpp"
#include <Fission/graphics/util.hpp>
#include <format.hpp>
#include <numeric>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_normalized_axis.hpp>
#if defined(FISSION_PLATFORM_LINUX) || defined(FISSION_PLATFORM_MACOS)
#include <GLFW/glfw3.h>
#endif

// TODO: only do portability stuff when on MACOS

using fmt::format;
using namespace fs;

#define DEBUG_UTILS_MESSAGE_SEVERITY_ALL            \
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT \
|   VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT    \
|   VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT \
|   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT

#define DEBUG_UTILS_MESSAGE_TYPE_ALL                \
    VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT     \
|   VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT  \
|   VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT \
|   VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT

inline constexpr char const*
severity_string(VkDebugUtilsMessageSeverityFlagBitsEXT severity)
{
	switch (severity)
	{
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: return "Verbose";
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:    return "Info";
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: return "Warning";
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:   return "Error";
	default:                                              return "Unknown";
	}
}

VKAPI_ATTR VkBool32 VKAPI_CALL debug_utils_callback(
	VkDebugUtilsMessageSeverityFlagBitsEXT severity,
	VkDebugUtilsMessageTypeFlagsEXT type,
	const VkDebugUtilsMessengerCallbackDataEXT* data, void* user)
{
	(void)type;
	(void)user;
	if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		auto level = severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
			? log::Warn
			: log::Error;
		os::log(level, std::string(data->pMessage));
	}

	if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
		auto body = format("{}\n\n{}", data->pMessageIdName, data->pMessage);
		os::show_error_dialog(
			format("Graphics Validation [{}]", severity_string(severity)), body
		);
	}

	return VK_FALSE;
}

#define check(FUNC, ERROR_MESSAGE) \
  if (FUNC != VK_SUCCESS) {        \
    log::error(ERROR_MESSAGE);     \
    return true;                   \
  }                                \
  (void)0

#ifdef assert
#undef assert
#endif
#define assert(EXPR, ERROR_MESSAGE) \
  if (!(EXPR)) {                    \
    log::error(ERROR_MESSAGE);      \
    return true;                    \
  }                                 \
  (void)0

void check_layers_and_extensions();

auto Graphics::create (Graphics_Create_Info const& info) -> bool
{
	log::verbose("Creating Graphics...");

    if (create_instance(info.debug))    return true;
    if (create_surface(info.window))    return true;
    if (pick_physical_device())         return true;
    if (pick_queue_families())          return true;
    if (create_device(info.debug))      return true;
    if (create_allocator())             return true;
    if (create_swap_chain(info.window)) return true;
    if (create_sc_image_views())        return true;
    if (create_command_buffers())       return true;
    if (create_sync_objects())          return true;

    return false;
}

void Graphics::destroy()
{
	log::verbose("Destroying Graphics...");

	if (device)
		vkDeviceWaitIdle(device);

	if (device) for_n (2) {
		vkDestroySemaphore(device, sc_image_read_semaphore[i], nullptr);
		vkDestroySemaphore(device, sc_image_write_semaphore[i], nullptr);
		vkDestroyFence(device, cb_fences[i], nullptr);
	}

	if (transfer_command_pool)
		vkDestroyCommandPool(device, transfer_command_pool, nullptr);

	if (command_pool)
		vkDestroyCommandPool(device, command_pool, nullptr);
	
	if (swap_chain) for_n (sc_image_count)
		vkDestroyImageView(device, sc_image_views[i], nullptr);

	if (swap_chain) vkDestroySwapchainKHR(device, swap_chain, nullptr);
	if (allocator) vmaDestroyAllocator(allocator);
	if (device) vkDestroyDevice(device, nullptr);
	if (surface) vkDestroySurfaceKHR(instance, surface, nullptr);

	if (debug_messenger) {
		auto vkDestroyDebugUtilsMessengerEXT =
			(PFN_vkDestroyDebugUtilsMessengerEXT)
			vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

		if (vkDestroyDebugUtilsMessengerEXT != nullptr) {
			vkDestroyDebugUtilsMessengerEXT(instance, debug_messenger, nullptr);
		}
		else log::error("Could not destroy debug messenger");
	}

	if (instance) vkDestroyInstance(instance, nullptr);

    // Set everything to null.. just in case
    memset(this, 0, sizeof(*this));
}

auto Graphics::create_instance(bool debug) -> bool
{
    log::debug("Creating Vulkan instance...");
	if (debug) check_layers_and_extensions();

	VkApplicationInfo application_info {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "How did you find this?",
		.applicationVersion = VK_MAKE_API_VERSION(1, 0, 0, 69),
		.pEngineName = "Fission",
		.engineVersion = vk::make_api_version<1,version_major,version_minor,version_patch>,
		.apiVersion = VK_API_VERSION_1_3,
	};

    log::info(format(
		"Vulkan Platform Extensions: {}",
		MACRO_STRING_EXPAND(FISSION_PLATFORM_VULKAN_EXTENSION_NAMES)
	));

	const char* extension_names[] = {
        VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
		VK_KHR_SURFACE_EXTENSION_NAME,
		FISSION_PLATFORM_VULKAN_EXTENSION_NAMES,
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
	};

	const char* layer_names[] = {
		"VK_LAYER_KHRONOS_validation",
	};

	VkDebugUtilsMessengerCreateInfoEXT debug_utils_info {
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.messageSeverity = DEBUG_UTILS_MESSAGE_SEVERITY_ALL,
            .messageType = DEBUG_UTILS_MESSAGE_TYPE_ALL,
		.pfnUserCallback = debug_utils_callback,
		.pUserData = nullptr,
	};
		
	VkInstanceCreateInfo instance_info {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext = debug? &debug_utils_info:nullptr,
        .flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
		.pApplicationInfo = &application_info,
		.enabledLayerCount = debug? 1u:0u,
		.ppEnabledLayerNames = layer_names,
		.enabledExtensionCount = vk::count(extension_names) - (debug? 0u:1u),
		.ppEnabledExtensionNames =  extension_names,
	};

    check(vkCreateInstance(&instance_info, nullptr, &instance),
          "Failed to create instance!");

	if (debug) {
		auto vkCreateDebugUtilsMessengerEXT = reinterpret_cast<
			PFN_vkCreateDebugUtilsMessengerEXT
		>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));

		if (vkCreateDebugUtilsMessengerEXT == nullptr) {
			log::error("Could not load function \"vkCreateDebugUtilsMessengerEXT\"");
		}
		else 
		check(vkCreateDebugUtilsMessengerEXT(instance, &debug_utils_info, nullptr, &debug_messenger),
			  "Failed to create debug messenger");
	}

    return false;
}

auto Graphics::create_surface(Window* window) -> bool
{
    log::debug("Creating Vulkan surface...");

#if   defined(FISSION_PLATFORM_WINDOWS)

	VkWin32SurfaceCreateInfoKHR surface_info {
		.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		.hwnd = window->_handle,
    };

    check(vkCreateWin32SurfaceKHR(instance, &surface_info, nullptr, &surface),
          "Failed to create surface!");

#elif defined(FISSION_PLATFORM_ANDROID)

    VkAndroidSurfaceCreateInfoKHR surface_info {
        .sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
        .window = window->_native,
    };

    check(vkCreateAndroidSurfaceKHR(instance, &surface_info, nullptr, &surface),
          "Failed to create surface!");

#elif defined(FISSION_PLATFORM_LINUX) || defined(FISSION_PLATFORM_MACOS)
    
    check(glfwCreateWindowSurface(instance, window->_glfw_window, nullptr, &surface),
          "Failed to create surface!");

#endif

    return false;
}

auto Graphics::pick_physical_device() -> bool
{
    log::debug("Picking Vulkan physical device...");
	
	u32 count;
    check(vkEnumeratePhysicalDevices(instance, &count, nullptr),
          "Failed to enumerate physical devices");

    assert(count != 0, "Unable to find physical device with Vulkan support!");

	std::vector<VkPhysicalDevice> physical_devices{count};
	check(vkEnumeratePhysicalDevices(instance, &count, physical_devices.data()),
          "Failed to enumerate physical devices");

	int max_score = -1;

	// Dumb selection, realisticly the user should decide what to use
	for (auto const& d: physical_devices) {
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(d, &properties);
		
		int score = [](VkPhysicalDeviceType t) {switch (t){
			case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:   return 500;
			case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return 300;
			case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:    return 270;
			case VK_PHYSICAL_DEVICE_TYPE_CPU:            return 100;
			default:                                     return 0;
		}} (properties.deviceType);
		
		VkPhysicalDeviceFeatures features;
		vkGetPhysicalDeviceFeatures(d, &features);

		score += std::accumulate(reinterpret_cast<u32*>(&features), reinterpret_cast<u32*>(&features+1), 0);

		if (score > max_score) {
			physical_device = d;
			max_score = score;
		}
	}
    physical_device = physical_devices[0];

	for (auto&& [i, d]: enumerate(physical_devices)) {
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(d, &properties);

		auto dt = [](VkPhysicalDeviceType t) {switch (t){
			case VK_PHYSICAL_DEVICE_TYPE_OTHER:          return "(Other)";
			case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return "(Integrated GPU)";
			case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:   return "(Descrete GPU)";
			case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:    return "(Virtual GPU)";
			case VK_PHYSICAL_DEVICE_TYPE_CPU:            return "(CPU)";
			default:                                     return "(Unknown)";
		}};

		log::info(format(" - GPU {}: {} {:16} {}", i, (d == physical_device)? ">>":"  ",
			dt(properties.deviceType), properties.deviceName));
	}

    return false;
}

bool Graphics::pick_queue_families()
{
	log::debug("Picking Vulkan queue families...");

	extra.queue_family.graphics = ~0u;
	extra.queue_family.transfer = ~0u;
	extra.queue_family.present  = ~0u;

	uint32_t family_count {};
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &family_count, nullptr);

	std::vector<VkQueueFamilyProperties> families_properties(family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &family_count, families_properties.data());

	uint32_t transfer_flags {};
	for_n ((u32)families_properties.size()) {
		const auto flags = families_properties[i].queueFlags;

		if ((extra.queue_family.graphics == ~0u) && (flags & VK_QUEUE_GRAPHICS_BIT))
			extra.queue_family.graphics = i;

		if (flags & VK_QUEUE_TRANSFER_BIT) {
			if (extra.queue_family.transfer != ~0u) {
				int count = std::popcount(transfer_flags);
				int new_count = std::popcount(flags);
				
				// select family with the least bits
				// meaning we want the most "specialized" for transfer operations
				if (new_count < count) {
					extra.queue_family.transfer = i;
					transfer_flags = flags;
				}
			}
			else {
				extra.queue_family.transfer = i;
				transfer_flags = flags;
			}
		}

		VkBool32 supports_surface = false;
#if 1
		vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &supports_surface);
#else
        supports_surface = glfwGetPhysicalDevicePresentationSupport(instance, physical_device, i);
#endif
        log::verbose(format("supports {}", supports_surface));

		if ((extra.queue_family.present == ~0u) && supports_surface)
			extra.queue_family.present = i;

		// TODO: print verbose info here
	}

	auto const& q = extra.queue_family;
	if ((q.graphics | q.transfer | q.present) == ~0u) {
		log::error("Failed to find suitable queue families");
		return true;
	}

    log::verbose(format("Graphics: {}", q.graphics));
    log::verbose(format("Transfer: {}", q.transfer));
    log::verbose(format("Present: {}", q.present));

    VkBool32 supported = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, q.present, surface, &supported);
    log::verbose(format("present supported? {}", (bool)supported));

	return false;
}

template <u32 max_size>
struct unique_queue_family_set {
	VkDeviceQueueCreateInfo data[max_size];
	u32 count = 0;
	float priority = 1.0f;

	void add(u32 index) {
		for (u32 i = 0; i < count; ++i)
			if (data[i].queueFamilyIndex == index) return;
		data[count++] = {.queueFamilyIndex = index};
	}

	VkDeviceQueueCreateInfo* get() {
		for (u32 i = 0; i < count; ++i) {
			data[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			data[i].pQueuePriorities = &priority;
			data[i].queueCount = 1;
		}
		return data;
	}
};

bool Graphics::create_device(bool debug)
{
	log::debug("Creating Vulkan device...");

    unique_queue_family_set<3> queue_family_set;
	queue_family_set.add(extra.queue_family.graphics);
    queue_family_set.add(extra.queue_family.transfer);
	queue_family_set.add(extra.queue_family.present);

    log::verbose(format("Count = {}", queue_family_set.count));
    for_n (queue_family_set.count) {
        log::verbose(format("Queue {}: index = {}", i, queue_family_set.data[i].queueFamilyIndex));
    }

	const char* device_extensions[] = {
        "VK_KHR_portability_subset",
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};

	VkPhysicalDeviceFeatures features {}; // This looks fun

/*
  https://vulkan-tutorial.com/en/Drawing_a_triangle/Setup/Logical_device_and_queues
	"Previous implementations of Vulkan made a distinction between instance
	and device specific validation layers, but this is no longer the case.
	That means that the enabledLayerCount and ppEnabledLayerNames fields
	of VkDeviceCreateInfo are ignored by up-to-date implementations.
	However, it is still a good idea to set them anyway to be compatible
	with older implementations"
*/
	const char* layer_names[] = { "VK_LAYER_KHRONOS_validation" };

    VkDeviceCreateInfo device_info {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = queue_family_set.count,
        .pQueueCreateInfos = queue_family_set.get(),
		.enabledLayerCount = debug? vk::count(layer_names):0,
		.ppEnabledLayerNames = layer_names,
	    .enabledExtensionCount = vk::count(device_extensions),
	    .ppEnabledExtensionNames = device_extensions,
        .pEnabledFeatures = &features,
    };

	VkResult result = vkCreateDevice(physical_device, &device_info, nullptr, &device);

    if (result != VK_SUCCESS) {
        log::error(format("Failed to create device (vkCreateDevice => {})", (int)result));
        return true;
    }

	// TODO: maybe more than 1 transfer queue for loading?
	vkGetDeviceQueue(device, extra.queue_family.graphics, 0, &graphics_queue);
	vkGetDeviceQueue(device, extra.queue_family.present , 0, &present_queue);
	vkGetDeviceQueue(device, extra.queue_family.transfer, 0, &transfer_queue);
    
    return false;
}

bool Graphics::create_allocator()
{
	log::debug("Creating Vulkan memory allocator (VMA) ...");

	VmaAllocatorCreateInfo allocator_info {
		.flags = VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT,
		.physicalDevice = physical_device,
		.device         = device,
		.instance       = instance,
	};

	check(vmaCreateAllocator(&allocator_info, &allocator),
		  "Failed to create Vulkan Memory Allocator");

	return false;
}

size_t pick_surface_format(std::vector<VkSurfaceFormatKHR> const& formats) {
	size_t index = 0;
	int max_score = 0;
	for_n (formats.size()) {
		int score = [](VkSurfaceFormatKHR const& sf) {switch (sf.format) {
			case VK_FORMAT_B8G8R8A8_SRGB: return 2;
			case VK_FORMAT_R8G8B8A8_SRGB: return 1;
			default: return 0;
		}} (formats[i]);

		if (score > max_score) {
			index = i;
			max_score = score;
		}
	}
	return index;
}

bool Graphics::create_swap_chain(Window* window)
{
	log::debug("Creating Vulkan swap chain...");

	VkSurfaceCapabilitiesKHR capabilities;
	check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities),
		  "Failed to get Vulkan surface capabilities");

    log::verbose(format("min, max = {}, {}", capabilities.minImageCount, capabilities.maxImageCount));

    u32 surface_format_count;
    check(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &surface_format_count, nullptr),
          "Failed to enumerate surface formats");

    assert(surface_format_count != 0, "Unable to find any surface formats!");

    std::vector<VkSurfaceFormatKHR> surface_formats {surface_format_count};
    check(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &surface_format_count, surface_formats.data()),
          "Failed to enumerate surface formats");

	size_t surface_format_index = pick_surface_format(surface_formats);

    for (auto&& [i,sf]: enumerate(surface_formats)) {
        log::debug(format(" - {:3}: {} {:32} {}", i, (i == surface_format_index)? ">>":"  ",
			vk::name(sf.colorSpace), vk::name(sf.format)));
    }

	sc_format = surface_formats[surface_format_index].format;
	sc_extent = capabilities.currentExtent;
    sc_transform = capabilities.currentTransform;
    log::debug(format(" - Surface Transform: {}", vk::name(sc_transform)));

    if (sc_transform & VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR
	||  sc_transform & VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR)
    {
        // Pre-rotation: always use native orientation
		// i.e. if rotated, use width and height of identity transform
        std::swap(sc_extent.width, sc_extent.height);
    }

#ifdef FISSION_PLATFORM_LINUX
    glfwGetFramebufferSize(window->_glfw_window, (int*)&sc_extent.width, (int*)&sc_extent.height);
#endif
    log::debug(format(" - size: {}x{}", sc_extent.width, sc_extent.height));

	log::debug(format(" - present mode: {}", vk::name(sc_present_mode)));

	VkSwapchainCreateInfoKHR swap_chain_info {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = surface,
		.minImageCount = capabilities.minImageCount + 1,
		.imageFormat = sc_format,
		.imageColorSpace = surface_formats[surface_format_index].colorSpace,
		.imageExtent = sc_extent,
		.imageArrayLayers = 1, /* For non-stereoscopic-3D applications, this value is 1 */
		.imageUsage = FISSION_DEFAULT_SWAP_CHAIN_USAGE,
		.preTransform = sc_transform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, // <-- TODO: fix this
		.presentMode = sc_present_mode, // TODO: this needs to be configurable
		.clipped = VK_TRUE, /* "... allows more efficient presentation methods to be used on some platforms." */
	};

	check(vkCreateSwapchainKHR(device, &swap_chain_info, nullptr, &swap_chain),
		  "Failed to create swap chain");

	return false;
}

bool Graphics::create_sc_image_views()
{
	log::debug("Creating Vulkan swap chain image views...");

	u32 old_image_count = sc_image_count;

	check(vkGetSwapchainImagesKHR(device, swap_chain, &sc_image_count, nullptr),
		  "vkGetSwapchainImagesKHR failed");

	log::debug(format(" - Number of swap chain images: {}", sc_image_count));

	// Allocate space for swap chain images
	if (sc_image_count > old_image_count) {
		if (sc_images) FISSION_DEFAULT_FREE(sc_images);

		bump_allocator bump {sc_image_count * size_of<VkImage,VkImageView>};
	
		sc_images      = bump.alloc<VkImage>    (sc_image_count);
		sc_image_views = bump.alloc<VkImageView>(sc_image_count);

		bump.release(); // we will track the memory
	}

	check(vkGetSwapchainImagesKHR(device, swap_chain, &sc_image_count, sc_images),
		  "vkGetSwapchainImagesKHR failed");
	
	auto view_info = vk::image_view_2d(VK_NULL_HANDLE, sc_format);
	for (u32 i = 0; i < sc_image_count; ++i) {
		view_info.image = sc_images[i];
		check(vkCreateImageView(device, &view_info, nullptr, sc_image_views + i),
			  "Failed to create swap chain image view");
	}

	return false;
}

bool Graphics::create_command_buffers()
{
	log::debug("Creating Vulkan command buffers...");

	VkCommandPoolCreateInfo graphics_pool_info {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = extra.queue_family.graphics,
	};

	check(vkCreateCommandPool(device, &graphics_pool_info, nullptr, &command_pool),
		  "Failed to create command pool");

	VkCommandPoolCreateInfo transfer_pool_info {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
		.queueFamilyIndex = extra.queue_family.transfer,
	};

	check(vkCreateCommandPool(device, &transfer_pool_info, nullptr, &transfer_command_pool),
		  "Failed to create command pool");

	VkCommandBufferAllocateInfo allocate_info {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = command_pool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = vk::count(command_buffers),
	};

	check(vkAllocateCommandBuffers(device, &allocate_info, command_buffers),
		  "Failed to allocate command buffers");
	
	return false;
}

bool Graphics::create_sync_objects()
{
	VkSemaphoreCreateInfo semaphore_info {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
	};
	
	for_n (2) {
		check(vkCreateSemaphore(device, &semaphore_info, nullptr, sc_image_read_semaphore + i),
			  "Failed to create semaphore");

		check(vkCreateSemaphore(device, &semaphore_info, nullptr, sc_image_write_semaphore + i),
			  "Failed to create semaphore");
	}

	VkFenceCreateInfo fence_info {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT,
	};

	for_n (2) {
		check(vkCreateFence(device, &fence_info, nullptr, cb_fences + i),
			  "Failed to create fence");
	}
	
	return false;
}

void check_layers_and_extensions()
{
	// note: assume these Vulkan functions never fail
    uint32_t layer_count {};
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

    std::vector<VkLayerProperties> layers {layer_count};
    vkEnumerateInstanceLayerProperties(&layer_count, layers.data());

    log::verbose(format("Number of layers available: {}", layer_count));

	for (auto&& [i, layer]: enumerate(layers)) {
		log::verbose(format(" - {:3}: \"{}\" ({})", i, layer.layerName, layer.description));
	}

    uint32_t extension_count;
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

    std::vector<VkExtensionProperties> extensions(extension_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());

    log::verbose(format("Number of extensions available: {}", extension_count));
	
	for (auto&& [i, ext]: enumerate(extensions)) {
		log::verbose(format(" - {:3}: \"{}\"", i, ext.extensionName));
	}
}

// TODO: errors? they exist right??
void Graphics::upload(VkBuffer dstBuffer, void const* inData, VkDeviceSize inSize)
{
	VkBuffer      stagingBuffer     {};
	VmaAllocation stagingAllocation {};

	VmaAllocationCreateInfo allocInfo {
		.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
		.usage = VMA_MEMORY_USAGE_AUTO,
	};
	VkBufferCreateInfo bufferInfo {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = inSize,
		.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	};
	vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &stagingBuffer, &stagingAllocation, nullptr);

	{
		void* dst;
		vmaMapMemory(allocator, stagingAllocation, &dst);
		memcpy(dst, inData, inSize);
		vmaUnmapMemory(allocator, stagingAllocation);
		vmaFlushAllocation(allocator, stagingAllocation, 0, VK_WHOLE_SIZE);
	}

	VkCommandBuffer cmd;
	VkCommandBufferAllocateInfo commandBufferInfo {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandBufferCount = 1,
		.commandPool = transfer_command_pool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
	};

	vkAllocateCommandBuffers(device, &commandBufferInfo, &cmd);
	{
		VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(cmd, &beginInfo);
		VkBufferCopy region {
			.srcOffset = 0,
			.dstOffset = 0,
			.size = inSize,
		};
		vkCmdCopyBuffer(cmd, stagingBuffer, dstBuffer, 1, &region);
		vkEndCommandBuffer(cmd);

		VkSubmitInfo submitInfo {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.commandBufferCount = 1,
			.pCommandBuffers = &cmd,
		};
		vkQueueSubmit(transfer_queue, 1, &submitInfo, VK_NULL_HANDLE);

		vkQueueWaitIdle(transfer_queue);
		vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);
	}
	vkFreeCommandBuffers(device, transfer_command_pool, 1, &cmd);
}

// TODO: errors? they exist right??
void Graphics::upload
(	VkImage       destination,
	void const*   image_data,
	VkExtent3D    extent,
	VkFormat      format,
	VkImageLayout final_layout,
	u32           layer
) {
    VkDeviceSize  data_size = extent.width * extent.height * extent.depth * vk::size_of(format);
    VkBuffer      buffer;
    VmaAllocation allocation;

    // 1. Create Staging Buffer
    VmaAllocationCreateInfo allocation_info {
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO,
    };
    VkBufferCreateInfo buffer_info {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = data_size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    };
    vmaCreateBuffer(allocator, &buffer_info, &allocation_info, &buffer, &allocation, nullptr);

    // 2. Copy Image Data to Staging Buffer
    {
        void* mapped;
        vmaMapMemory(allocator, allocation, &mapped);
        memcpy(mapped, image_data, data_size);
        vmaUnmapMemory(allocator, allocation);
        vmaFlushAllocation(allocator, allocation, 0, VK_WHOLE_SIZE);
    }

    // 3. Allocate a Command Buffer for Transfer operations
    VkCommandBuffer command_buffer;
    VkCommandBufferAllocateInfo command_buffer_info {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = transfer_command_pool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
	};
    vkAllocateCommandBuffers(device, &command_buffer_info, &command_buffer);

	vk::begin(command_buffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    {
        // 4. Set Image Layout for transfer
		VkImageSubresourceRange range {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = layer,
			.layerCount = 1,
		};
        VkImageMemoryBarrier to_transfer {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.srcAccessMask = 0,
			.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.image = destination,
			.subresourceRange = range,
		};
        vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &to_transfer);

        // 5. Copy from Staging Buffer to Destination Image
		VkBufferImageCopy region {
			.imageSubresource = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.mipLevel = 0,
				.baseArrayLayer = layer,
				.layerCount = 1,
			},
			.imageExtent = extent,
		};
		vkCmdCopyBufferToImage(command_buffer, buffer, destination,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        // 6. Set Image Layout to the Final Layout
        VkImageMemoryBarrier to_readable {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
			.dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.newLayout = final_layout,
			.image = destination,
			.subresourceRange = range,
		};
        vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &to_readable);
    }
    vk::end(command_buffer);

    // 7. Sumbit commands to GPU and wait for them to complete
	VkSubmitInfo submit_info {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &command_buffer,
	};
    vkQueueSubmit(transfer_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(transfer_queue);

	// 8. Cleanup Command Buffer and Staging Buffer
    vkFreeCommandBuffers(device, transfer_command_pool, 1, &command_buffer);
    vmaDestroyBuffer(allocator, buffer, allocation);
}

version Graphics::api_version()
{
	uint32_t instanceVersion;
	
	// TODO: should I pass instance here?
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

auto Graphics::pre_rotation() -> glm::mat2
{
	using namespace glm;
	switch (sc_transform)
	{
	default:
		return glm::mat2(1.0f);
	case VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR:
		return rotate(mat4(1.0f), radians(90.0f), vec3(0.0f, 0.0f, 1.0f));
	case VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR:
		return rotate(mat4(1.0f), radians(180.0f), vec3(0.0f, 0.0f, 1.0f));
	case VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR:
		return rotate(mat4(1.0f), radians(270.0f), vec3(0.0f, 0.0f, 1.0f));
	}
}


////////////////////////////////////////////////////////////////////
// Shaders

VkShaderModule vk::create_shader(size_t size, void const* data) {
	VkShaderModule shader;
	VkShaderModuleCreateInfo shader_info {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = size,
		.pCode = reinterpret_cast<const uint32_t*>(data),
	};
	vkCreateShaderModule(engine.graphics.device, &shader_info, nullptr, &shader);
	return shader;
}


void vk::begin(VkCommandBuffer command_buffer, VkRenderPass render_pass, VkFramebuffer frame_buffer, VkClearColorValue color)
{
	VkClearValue clear_color = { color };
	VkRenderPassBeginInfo begin_info {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = render_pass,
		.framebuffer = frame_buffer,
		.renderArea = {
			.offset = {0, 0},
			.extent = engine.graphics.sc_extent,
		},
		.clearValueCount = 1,
		.pClearValues = &clear_color,
	};
	vkCmdBeginRenderPass(command_buffer, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
}


VkResult vk::Pipeline_Creator::create_and_destroy_shaders(VkPipeline * pipeline) {
	auto result = create(pipeline);
	for (auto&& shader_info: shaders)
		vkDestroyShaderModule(engine.graphics.device, shader_info.module, nullptr);
	return result;
}

VkResult vk::Pipeline_Creator::create(VkPipeline* p_pipeline) {
	dynamic_state.dynamicStateCount = (fs::u32)dynamic_states.size();
	dynamic_state.pDynamicStates = dynamic_states.data();
	color_blend_state.pAttachments = &blend_attachment;

	VkGraphicsPipelineCreateInfo pipeline_info {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount          = (fs::u32)shaders.size(),
		.pStages             = shaders.data(),
		.pVertexInputState   = vertex_input_state,
		.pInputAssemblyState = &input_assembly_state,
		.pViewportState      = &viewport_state,
		.pRasterizationState = &rasterization_state,
		.pMultisampleState   = &multisample_state,
		.pDepthStencilState  = &depth_stencil_state,
		.pColorBlendState    = &color_blend_state,
		.pDynamicState       = &dynamic_state,
		.layout              = layout,
		.renderPass          = render_pass,
		.subpass             = subpass,
	};
	return vkCreateGraphicsPipelines(engine.graphics.device, VK_NULL_HANDLE,
		1, &pipeline_info, nullptr, p_pipeline);
}

VkResult vk::Pipeline_Layout_Creator::create(VkPipelineLayout* p_pipeline_layout) {
	VkPipelineLayoutCreateInfo pipeline_layout_info{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = (fs::u32)layouts.size(),
		.pSetLayouts = layouts.data(),
		.pushConstantRangeCount = (fs::u32)push_ranges.size(),
		.pPushConstantRanges = push_ranges.data(),
	};
	return vkCreatePipelineLayout(engine.graphics.device, &pipeline_layout_info,
		nullptr, p_pipeline_layout);
}


////////////////////////////////////////////////////////////////////
// Render Pass Creator

VkResult vk::Render_Pass_Creator::create(VkRenderPass* pRenderPass) {
	VkRenderPassCreateInfo render_pass_info {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = (fs::u32)attachments.size(),
		.pAttachments = attachments.data(),
		.subpassCount = (fs::u32)subpasses.size(),
		.pSubpasses = subpasses.data(),
		.dependencyCount = (fs::u32)subpass_dependencies.size(),
		.pDependencies = subpass_dependencies.data(),
	};
	return vkCreateRenderPass(engine.graphics.device, &render_pass_info, nullptr, pRenderPass);
}
