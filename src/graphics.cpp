#include "Fission/core.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/rotate_normalized_axis.hpp"

namespace vk
{
	inline constexpr VkImageViewCreateInfo image_view_2d(VkImage image, VkFormat format, VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT) {
		return {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.flags = 0,
			.image = image,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = format,
			.components = {}, // Identity
			.subresourceRange = {
				.aspectMask = aspect,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			}
		};
	}
}

BEGIN_NAMESPACE(fission)

void format_single(Arena& arena, VkSurfaceTransformFlagBitsKHR surface_transform_flags)
{
	bool found = false;
	if (surface_transform_flags&VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
		found? format_single(arena, "-"):(void)0, format_single(arena, "IDENTITY"), found = true;
	if (surface_transform_flags&VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR)
		found? format_single(arena, "-"):(void)0, format_single(arena, "ROTATE_90"), found = true;
	if (surface_transform_flags&VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR)
		found? format_single(arena, "-"):(void)0, format_single(arena, "ROTATE_180"), found = true;
	if (surface_transform_flags&VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR)
		found? format_single(arena, "-"):(void)0, format_single(arena, "ROTATE_270"), found = true;
	if (surface_transform_flags&VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR)
		found? format_single(arena, "-"):(void)0, format_single(arena, "HORIZONTAL_MIRROR"), found = true;
	if (surface_transform_flags&VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR)
		found? format_single(arena, "-"):(void)0, format_single(arena, "HORIZONTAL_MIRROR_ROTATE_90"), found = true;
	if (surface_transform_flags&VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR)
		found? format_single(arena, "-"):(void)0, format_single(arena, "HORIZONTAL_MIRROR_ROTATE_180"), found = true;
	if (surface_transform_flags&VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR)
		found? format_single(arena, "-"):(void)0, format_single(arena, "HORIZONTAL_MIRROR_ROTATE_270"), found = true;
	if (surface_transform_flags&VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR)
		found? format_single(arena, "-"):(void)0, format_single(arena, "INHERIT"), found = true;
}

// TODO: only do portability stuff when on MACOS

// TODO: find a solution to where to put this / is this needed
constexpr auto popcount(unsigned x) noexcept {
    unsigned num{};
    for (; x; ++num, x &= (x - 1));
    return num;
};

#define DEBUG_UTILS_MESSAGE_SEVERITY_ALL            \
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT \
|   VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT    \
|   VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT \
|   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT

#define DEBUG_UTILS_MESSAGE_TYPE_ALL                \
    VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT     \
|   VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT  \
|   VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT

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
    NOT_USED(user, type);

    auto level = severity <= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
        ? log::Warn
        : log::Error;

	if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    	log::log(level, data->pMessage);
	}
	return VK_FALSE;
}

#ifdef check
#undef check
#endif
#define check(FUNC, ERROR_MESSAGE) \
  if (FUNC < VK_SUCCESS) {         \
    log::error(ERROR_MESSAGE);     \
    return Failed;                 \
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

auto Graphics::create (Create_Info const& info) -> Result
{
	log::info("Creating Graphics Context...");
    if (create_instance(info.debug))    return Failed;
    if (create_surface(info.window))    return Failed;
    if (pick_physical_device())         return Failed;
    if (pick_queue_families())          return Failed;
    if (create_device(info.debug))      return Failed;
    if (create_allocator())             return Failed;
    if (create_swap_chain(info.window)) return Failed;
    if (create_sc_image_views())        return Failed;
    if (create_command_buffers())       return Failed;
    if (create_sync_objects())          return Failed;
    return Success;
}
#if 0
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
#endif

void log_layers_and_extensions()
{
    uint32_t layer_count {};
    if (vkEnumerateInstanceLayerProperties(&layer_count, nullptr) != VK_SUCCESS)
        return;

    log::verbose("Number of layers available: ", layer_count);
    auto layers = scratch_arena.alloc<VkLayerProperties>(layer_count);
    if (vkEnumerateInstanceLayerProperties(&layer_count, layers) != VK_SUCCESS)
        return;

	forn (layer_count) {
        using namespace formatting;
        const char* layer_name = layers[i].layerName;
        const char* description = layers[i].description;
		log::verbose(" - ", pad(layer_name, 36), " ", description);
	}
    scratch_arena.reset();

    uint32_t extension_count {};
    if (vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr) != VK_SUCCESS)
        return;

    auto extensions = scratch_arena.alloc<VkExtensionProperties>(extension_count);
    if (vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions) != VK_SUCCESS)
        return;

    log::verbose("Number of extensions available: ", extension_count);
	
	forn (extension_count) {
        using namespace formatting;
        const char* extension_name = extensions[i].extensionName;
		log::verbose(" - ", extension_name);
	}
    scratch_arena.reset();
}

auto Graphics::create_instance(bool debug) -> Result
{
    log::verbose("Creating Vulkan instance...");
    log::verbose("Graphics debugging enabled: ", debug);
	if (debug) log_layers_and_extensions();

	VkApplicationInfo application_info {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "How did you find this?",
		.applicationVersion = VK_MAKE_API_VERSION(1, 0, 0, 69),
		.pEngineName = "Fission",
	//	.engineVersion = vk::make_api_version<1,version_major,version_minor,version_patch>,
		.apiVersion = VK_API_VERSION_1_3,
	};

	const char* extension_names[] = {
    #if defined(OS_MACOS)
        VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
    #endif
		VK_KHR_SURFACE_EXTENSION_NAME,
    #if defined(OS_MACOS)
        "VK_EXT_metal_surface",
    #endif
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
	};
		
	VkInstanceCreateInfo instance_info {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext = debug? &debug_utils_info:nullptr,
    #if defined(OS_MACOS)
        .flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
    #else
        .flags = 0,
    #endif
		.pApplicationInfo = &application_info,
		.enabledLayerCount = debug? 1u:0u,
		.ppEnabledLayerNames = layer_names,
		.enabledExtensionCount = array_count(extension_names) - (debug? 0u:1u),
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
    return Success;
}

auto Graphics::create_surface(Window* window) -> Result
{
    log::verbose("Creating Vulkan surface...");
#if defined(OS_WINDOWS)
	VkWin32SurfaceCreateInfoKHR surface_info {
		.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		.hwnd = window->_handle,
    };

    check(vkCreateWin32SurfaceKHR(instance, &surface_info, nullptr, &surface),
          "Failed to create surface!");
#elif defined(OS_ANDROID)
    VkAndroidSurfaceCreateInfoKHR surface_info {
        .sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
        .window = window->_native,
    };

    check(vkCreateAndroidSurfaceKHR(instance, &surface_info, nullptr, &surface),
          "Failed to create surface!");
#elif defined(OS_LINUX) || defined(OS_MACOS)
    check(glfwCreateWindowSurface(instance, window->_window, nullptr, &surface),
          "Failed to create surface!");
#endif
    return Success;
}

auto Graphics::pick_physical_device() -> Result
{
    log::verbose("Picking Vulkan physical device...");
	
	u32 count {};
    check(vkEnumeratePhysicalDevices(instance, &count, nullptr),
          "Failed to enumerate physical devices");

    if (count == 0) {
        log::error("Unable to find physical device with Vulkan support!");
        return Failed;
    }

    auto physical_devices = scratch_arena.alloc<VkPhysicalDevice>(count);
	check(vkEnumeratePhysicalDevices(instance, &count, physical_devices),
          "Failed to enumerate physical devices");

	int max_score = -1;

	// Dumb selection, realisticly the user should decide what to use
	for (auto const& d: array{count, physical_devices}) {
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

        forn (sizeof(features)/sizeof(VkBool32)) {
            auto p = reinterpret_cast<VkBool32*>(&features);
            score += int(p[i]);
        }

		if (score > max_score) {
			physical_device = d;
			max_score = score;
		}
	}

    scratch_arena.reset();
    physical_device = physical_devices[0];

    int i = 0;
	for (auto const& d: array{count, physical_devices}) {
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(d, &properties);

		auto dt = [](VkPhysicalDeviceType t) {switch (t){
			case VK_PHYSICAL_DEVICE_TYPE_OTHER:          return "(Other)           ";
			case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return "(Integrated GPU)  ";
			case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:   return "(Descrete GPU)    ";
			case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:    return "(Virtual GPU)     ";
			case VK_PHYSICAL_DEVICE_TYPE_CPU:            return "(CPU)             ";
			default:                                     return "(Unknown)         ";
		}};

		log::info(" - GPU ", i++, ": ", (d == physical_device)? ">> ":"   ",
            dt(properties.deviceType), properties.deviceName);
	}
    return Success;
}

auto Graphics::pick_queue_families() -> Result
{
	log::verbose("Picking Vulkan queue families...");

	queue_family.graphics = ~u32(0);
	queue_family.transfer = ~u32(0);
	queue_family.present  = ~u32(0);

	uint32_t family_count {};
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &family_count, nullptr);

    auto families_properties = scratch_arena.alloc<VkQueueFamilyProperties>(family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &family_count, families_properties);

	uint32_t transfer_flags {};
	forn (family_count) {
		const auto flags = families_properties[i].queueFlags;

		if ((queue_family.graphics == ~0u) && (flags & VK_QUEUE_GRAPHICS_BIT))
			queue_family.graphics = i;

		if (flags & VK_QUEUE_TRANSFER_BIT) {
			if (queue_family.transfer != ~0u) {
				int count = popcount(transfer_flags);
				int new_count = popcount(flags);
				
				// select family with the least bits
				// meaning we want the most "specialized" for transfer operations
				if (new_count < count) {
					queue_family.transfer = i;
					transfer_flags = flags;
				}
			}
			else {
				queue_family.transfer = i;
				transfer_flags = flags;
			}
		}

		VkBool32 supports_surface = false;
#if 1
		vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &supports_surface);
#else
        supports_surface = glfwGetPhysicalDevicePresentationSupport(instance, physical_device, i);
#endif

		if ((queue_family.present == ~0u) && supports_surface)
			queue_family.present = i;

		//! TODO: print verbose info here
	}
    scratch_arena.reset();

	auto const& q = queue_family;
	if ((q.graphics | q.transfer | q.present) == ~u32(0)) {
		log::error("Failed to find suitable queue families");
		return Failed;
	}

    log::verbose("Graphics: ", q.graphics);
    log::verbose("Transfer: ", q.transfer);
    log::verbose("Present: ", q.present);

    VkBool32 supported = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, q.present, surface, &supported);
    log::verbose("Surface supports present: ", (bool)supported);

	return Success;
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

auto Graphics::create_device(bool debug) -> Result
{
	log::verbose("Creating Vulkan device...");

    unique_queue_family_set<3> queue_family_set;
	queue_family_set.add(queue_family.graphics);
    queue_family_set.add(queue_family.transfer);
	queue_family_set.add(queue_family.present);
    log::verbose("Number of unique queue families = ", queue_family_set.count);

	const char* device_extensions[] = {
    #if defined(OS_MACOS)
        "VK_KHR_portability_subset",
    #endif
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};

	VkPhysicalDeviceFeatures features {}; // This looks fun
	//features.fillModeNonSolid = VK_TRUE;

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
		.enabledLayerCount = debug? array_count(layer_names):0,
		.ppEnabledLayerNames = layer_names,
	    .enabledExtensionCount = array_count(device_extensions),
	    .ppEnabledExtensionNames = device_extensions,
        .pEnabledFeatures = &features,
    };

    check(vkCreateDevice(physical_device, &device_info, nullptr, &device),
        "Failed to create device");

	//! TODO: use more than 1 transfer queue for loading
	vkGetDeviceQueue(device, queue_family.graphics, 0, &graphics_queue);
	vkGetDeviceQueue(device, queue_family.present , 0, &present_queue);
	vkGetDeviceQueue(device, queue_family.transfer, 0, &transfer_queue);
    
    return Success;
}

auto Graphics::create_allocator() -> Result
{
	log::verbose("Creating Vulkan memory allocator (VMA) ...");

	VmaAllocatorCreateInfo allocator_info {
		.flags = VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT,
		.physicalDevice = physical_device,
		.device         = device,
		.instance       = instance,
	};

	check(vmaCreateAllocator(&allocator_info, &allocator),
		  "Failed to create Vulkan Memory Allocator");

	return Success;
}

size_t pick_surface_format(std::vector<VkSurfaceFormatKHR> const& formats) {
	size_t index = 0;
	int max_score = 0;
	forn (formats.size()) {
		int score = [](VkSurfaceFormatKHR const& sf) {switch (sf.format) {
            case VK_FORMAT_B8G8R8A8_UNORM: return 4;
            case VK_FORMAT_R8G8B8A8_UNORM: return 3;
			case VK_FORMAT_B8G8R8A8_SRGB:  return 2;
			case VK_FORMAT_R8G8B8A8_SRGB:  return 1;
			default: return 0;
		}} (formats[i]);

		if (score > max_score) {
			index = i;
			max_score = score;
		}
	}
	return index;
}

auto Graphics::create_swap_chain(Window* window) -> Result
{
	log::verbose("Creating Vulkan swap chain...");

	VkSurfaceCapabilitiesKHR capabilities {};
	check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities),
		  "Failed to get Vulkan surface capabilities");

    log::verbose("max image count ", capabilities.minImageCount);
    log::verbose("min image count ", capabilities.maxImageCount);

    u32 surface_format_count {};
    check(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &surface_format_count, nullptr),
          "Failed to enumerate surface formats");

    if (surface_format_count == 0) {
        log::error("Unable to find any surface formats!");
        return Failed;
    }

    std::vector<VkSurfaceFormatKHR> surface_formats {surface_format_count};
    check(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &surface_format_count, surface_formats.data()),
          "Failed to enumerate surface formats");

	size_t surface_format_index = pick_surface_format(surface_formats);

	//! TODO: fix
    //for (auto&& [i,sf]: enumerate(surface_formats)) {
    //    log::debug(format(" - {:3}: {} {:32} {}", i, (i == surface_format_index)? ">>":"  ",
	//		vk::name(sf.colorSpace), vk::name(sf.format)));
    //}

	format = surface_formats[surface_format_index].format;
	extent = capabilities.currentExtent;
    transform = capabilities.currentTransform;
    log::debug(" - Surface Transform: ", transform);

    if (transform & VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR
	||  transform & VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR)
    {
        // Pre-rotation: always use native orientation
		// i.e. if rotated, use width and height of identity transform
    	std::swap(extent.width, extent.height);
    }

#if defined(OS_LINUX)
    glfwGetFramebufferSize(window->_window, (int*)&extent.width, (int*)&extent.height);
#else
	NOT_USED(window);
#endif
    log::debug(" - size: ", extent.width, "x", extent.height);
	//! TODO: present mode formatter
	log::debug(" - present mode: ", int(present_mode));

	VkSwapchainCreateInfoKHR swap_chain_info {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = surface,
		.minImageCount = capabilities.minImageCount + 1,
		.imageFormat = format,
		.imageColorSpace = surface_formats[surface_format_index].colorSpace,
		.imageExtent = extent,
		.imageArrayLayers = 1, /* For non-stereoscopic-3D applications, this value is 1 */
		.imageUsage = image_usage,
		.preTransform = transform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,//VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, // <-- TODO: fix this (ANDROID)
		.presentMode = present_mode, // TODO: this needs to be configurable
		.clipped = VK_TRUE, /* "... allows more efficient presentation methods to be used on some platforms." */
	};

	check(vkCreateSwapchainKHR(device, &swap_chain_info, nullptr, &swap_chain),
		  "Failed to create swap chain");
	return Success;
}
auto Graphics::create_sc_image_views() -> Result
{
	log::debug("Creating Vulkan swap chain image views...");

	u32 old_image_count = image_count;

	check(vkGetSwapchainImagesKHR(device, swap_chain, &image_count, nullptr),
		  "vkGetSwapchainImagesKHR failed");

	log::debug(" - Number of swap chain images: ", image_count);

	ASSERT(image_count <= 8);
	check(vkGetSwapchainImagesKHR(device, swap_chain, &image_count, images),
		  "vkGetSwapchainImagesKHR failed");
	
	auto view_info = ::vk::image_view_2d(VK_NULL_HANDLE, format);
	forn (image_count) {
		view_info.image = images[i];
		check(vkCreateImageView(device, &view_info, nullptr, image_views + i),
			  "Failed to create swap chain image view");
	}

	return Success;
}

auto Graphics::create_command_buffers() -> Result
{
	log::debug("Creating Vulkan command buffers...");

	VkCommandPoolCreateInfo graphics_pool_info {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = queue_family.graphics,
	};

	check(vkCreateCommandPool(device, &graphics_pool_info, nullptr, &command_pool),
		  "Failed to create command pool");

	VkCommandPoolCreateInfo transfer_pool_info {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
		.queueFamilyIndex = queue_family.transfer,
	};

	check(vkCreateCommandPool(device, &transfer_pool_info, nullptr, &transfer_command_pool),
		  "Failed to create command pool");

	VkCommandBufferAllocateInfo allocate_info {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = command_pool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = array_count(command_buffers),
	};

	check(vkAllocateCommandBuffers(device, &allocate_info, command_buffers),
		  "Failed to allocate command buffers");
	
	return Success;
}

auto Graphics::create_sync_objects() -> Result
{
	VkSemaphoreCreateInfo semaphore_info {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
	};
	
	forn (2) {
		check(vkCreateSemaphore(device, &semaphore_info, nullptr, image_read_semaphore + i),
			  "Failed to create semaphore");

		check(vkCreateSemaphore(device, &semaphore_info, nullptr, image_write_semaphore + i),
			  "Failed to create semaphore");
	}

	VkFenceCreateInfo fence_info {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT,
	};

	forn (2) {
		check(vkCreateFence(device, &fence_info, nullptr, fences + i),
			  "Failed to create fence");
	}
	
	return Success;
}
#if 0
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
void Graphics::upload (
	VkImage       destination,
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
		auto range = vk::color_image_range(layer);

        // 4. Set Image Layout for transfer
		vk::image_barrier (
			command_buffer, destination,
			VK_IMAGE_LAYOUT_UNDEFINED,            VK_ACCESS_NONE,               VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
			range
		);

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
		vk::image_barrier (
			command_buffer, destination,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
			final_layout,                         VK_ACCESS_SHADER_READ_BIT,    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			range
		);
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
	if (physical_device == VK_NULL_HANDLE) {
		fs::log::warn("Cannot call Graphics::api_version until graphics has been created");
		return {};
	}

	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(physical_device, &properties);

	return {
		VK_API_VERSION_MAJOR(properties.apiVersion),
		VK_API_VERSION_MINOR(properties.apiVersion),
		VK_API_VERSION_PATCH(properties.apiVersion),
	};
}

auto Graphics::pre_rotation() -> glm::mat2
{
	using namespace glm;
	switch (sc_transform)
	{
	default:
		return mat2(1.0f);
	case VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR:
		return rotate(mat4(1.0f), radians(90.0f), vec3(0.0f, 0.0f, 1.0f));
	case VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR:
		return rotate(mat4(1.0f), radians(180.0f), vec3(0.0f, 0.0f, 1.0f));
	case VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR:
		return rotate(mat4(1.0f), radians(270.0f), vec3(0.0f, 0.0f, 1.0f));
	}
}

void Graphics::set_default_viewport(VkCommandBuffer cmd) {
    VkViewport viewport {
        .x = 0.0f,
        .y = 0.0f,
        .width  = static_cast<float>(sc_extent.width),
        .height = static_cast<float>(sc_extent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };
    vkCmdSetViewport(cmd, 0, 1, &viewport);
}

void Graphics::set_default_scissor(VkCommandBuffer cmd) {
    VkRect2D scissor {
        .offset = { 0, 0 },
        .extent = sc_extent,
    };
    vkCmdSetScissor(cmd, 0, 1, &scissor);
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
		.renderArea = { {0, 0}, engine.graphics.sc_extent },
		.clearValueCount = 1,
		.pClearValues = &clear_color,
	};
	vkCmdBeginRenderPass(command_buffer, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

void vk::begin(VkCommandBuffer command_buffer, VkRenderPass render_pass, VkFramebuffer frame_buffer)
{
	VkRenderPassBeginInfo begin_info {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = render_pass,
		.framebuffer = frame_buffer,
		.renderArea = { {0, 0}, engine.graphics.sc_extent },
		.clearValueCount = 0,
		.pClearValues = nullptr,
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

VkResult vk::Pipeline_Creator::create_no_fragment(VkPipeline* pipeline) {
	dynamic_state.dynamicStateCount = (fs::u32)dynamic_states.size();
	dynamic_state.pDynamicStates = dynamic_states.data();
	color_blend_state.pAttachments = &blend_attachment;

	VkGraphicsPipelineCreateInfo pipelineInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
	pipelineInfo.stageCount = 1;
	pipelineInfo.pStages = shaders.data();
	pipelineInfo.pVertexInputState = vertex_input_state;
	pipelineInfo.pInputAssemblyState = &input_assembly_state;
	pipelineInfo.pViewportState = &viewport_state;
	pipelineInfo.pRasterizationState = &rasterization_state;
	pipelineInfo.pMultisampleState = &multisample_state;
	pipelineInfo.pDepthStencilState = &depth_stencil_state;
	pipelineInfo.pColorBlendState = &color_blend_state;
	pipelineInfo.pDynamicState = &dynamic_state;
	pipelineInfo.layout = layout;
	pipelineInfo.renderPass = render_pass;
	pipelineInfo.subpass = subpass;
	return vkCreateGraphicsPipelines(engine.graphics.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, pipeline);
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
		.pAttachments    = attachments.data(),
		.subpassCount    = (fs::u32)subpasses.size(),
		.pSubpasses      = subpasses.data(),
		.dependencyCount = (fs::u32)subpass_dependencies.size(),
		.pDependencies   = subpass_dependencies.data(),
	};
	return vkCreateRenderPass(engine.graphics.device, &render_pass_info, nullptr, pRenderPass);
}

void fs::set_viewport_and_scissor(VkCommandBuffer cmd, rf32 rect) {
	VkRect2D scissor {
		.offset = {},
		.extent = { .width = rect.width(), .height = rect.height() },
	};
	vkCmdSetScissor(cmd, 0, 1, &scissor);

	VkViewport viewport {
		.width = rect.width(),
		.height = rect.height(),
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
		.x = 0.0f,
		.y = 0.0f,
	};
	vkCmdSetViewport(cmd, 0, 1, &viewport);
}
#endif

END_NAMESPACE()
