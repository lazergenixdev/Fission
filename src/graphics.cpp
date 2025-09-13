#include "Fission/core.hpp"
#include "embed/draw2d.spv.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/rotate_normalized_axis.hpp"
#include <bit>
using std::popcount;
#define TAB "   "

//constexpr auto popcount(unsigned x) {
//    unsigned num{};
//    for (; x; ++num, x &= (x - 1));
//    return num;
//};

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

#define X_VK_SURFACE_TRANSFORMS(X)                                  \
    X(IDENTITY) X(ROTATE_90) X(ROTATE_180) X(ROTATE_270)            \
    X(HORIZONTAL_MIRROR) X(HORIZONTAL_MIRROR_ROTATE_90)             \
    X(HORIZONTAL_MIRROR_ROTATE_180) X(HORIZONTAL_MIRROR_ROTATE_270) \
    X(INHERIT)

void format_single(Arena& arena, VkSurfaceTransformFlagBitsKHR surface_transform_flags)
{
	bool found = false;
	#define FORMAT_ENUM(NAME) \
	if (surface_transform_flags&VK_SURFACE_TRANSFORM_##NAME##_BIT_KHR) \
		found? format_single(arena, "-"):(void)0, format_single(arena, #NAME), found = true;
	X_VK_SURFACE_TRANSFORMS(FORMAT_ENUM)
	#undef FORMAT_ENUM
}

#define X_VK_FORMATS(X) \
    X(UNDEFINED) X(R4G4_UNORM_PACK8) X(R4G4B4A4_UNORM_PACK16) X(B4G4R4A4_UNORM_PACK16) \
    X(R5G6B5_UNORM_PACK16) X(B5G6R5_UNORM_PACK16) X(R5G5B5A1_UNORM_PACK16) \
    X(B5G5R5A1_UNORM_PACK16) X(A1R5G5B5_UNORM_PACK16) X(R8_UNORM) X(R8_SNORM) \
    X(R8_USCALED) X(R8_SSCALED) X(R8_UINT) X(R8_SINT) X(R8_SRGB) X(R8G8_UNORM) \
    X(R8G8_SNORM) X(R8G8_USCALED) X(R8G8_SSCALED) X(R8G8_UINT) X(R8G8_SINT) X(R8G8_SRGB) \
	X(R8G8B8_UNORM) X(R8G8B8_SNORM) X(R8G8B8_USCALED) X(R8G8B8_SSCALED) X(R8G8B8_UINT) \
    X(R8G8B8_SINT) X(R8G8B8_SRGB) X(B8G8R8_UNORM) X(B8G8R8_SNORM) X(B8G8R8_USCALED) \
    X(B8G8R8_SSCALED) X(B8G8R8_UINT) X(B8G8R8_SINT) X(B8G8R8_SRGB) X(R8G8B8A8_UNORM) \
    X(R8G8B8A8_SNORM) X(R8G8B8A8_USCALED) X(R8G8B8A8_SSCALED) X(R8G8B8A8_UINT) X(R8G8B8A8_SINT) \
    X(R8G8B8A8_SRGB) X(B8G8R8A8_UNORM) X(B8G8R8A8_SNORM) X(B8G8R8A8_USCALED) X(B8G8R8A8_SSCALED) \
    X(B8G8R8A8_UINT) X(B8G8R8A8_SINT) X(B8G8R8A8_SRGB) X(A8B8G8R8_UNORM_PACK32) \
    X(A8B8G8R8_SNORM_PACK32) X(A8B8G8R8_USCALED_PACK32) X(A8B8G8R8_SSCALED_PACK32) X(A8B8G8R8_UINT_PACK32) \
	X(A8B8G8R8_SRGB_PACK32) X(A2R10G10B10_UNORM_PACK32) X(A2R10G10B10_SNORM_PACK32) \
	X(A2R10G10B10_USCALED_PACK32) X(A2R10G10B10_SSCALED_PACK32) X(A2R10G10B10_UINT_PACK32) \
	X(A2R10G10B10_SINT_PACK32) X(A2B10G10R10_UNORM_PACK32) X(A2B10G10R10_SNORM_PACK32) \
	X(A2B10G10R10_USCALED_PACK32) X(A2B10G10R10_SSCALED_PACK32) X(A2B10G10R10_UINT_PACK32) \
	X(A2B10G10R10_SINT_PACK32) X(R16_UNORM) X(R16_SNORM) X(R16_USCALED) X(R16_SSCALED) X(R16_UINT) \
	X(R16_SINT) X(R16_SFLOAT) X(R16G16_UNORM) X(R16G16_SNORM) X(R16G16_USCALED) X(R16G16_SSCALED) \
	X(R16G16_UINT) X(R16G16_SINT) X(R16G16_SFLOAT) X(R16G16B16_UNORM) X(R16G16B16_SNORM) \
	X(R16G16B16_USCALED) X(R16G16B16_SSCALED) X(R16G16B16_UINT) X(R16G16B16_SINT) X(R16G16B16_SFLOAT) \
	X(R16G16B16A16_UNORM) X(R16G16B16A16_SNORM) \
    X(R16G16B16A16_USCALED) X(R16G16B16A16_SSCALED) X(R16G16B16A16_UINT) X(R16G16B16A16_SINT) \
    X(R16G16B16A16_SFLOAT) X(R32_UINT) X(R32_SINT) X(R32_SFLOAT) X(R32G32_UINT) X(R32G32_SINT) \
    X(R32G32_SFLOAT) X(R32G32B32_UINT) X(R32G32B32_SINT) X(R32G32B32_SFLOAT) X(R32G32B32A32_UINT) \
    X(R32G32B32A32_SINT) X(R32G32B32A32_SFLOAT) X(R64_UINT) X(R64_SINT) X(R64_SFLOAT) \
    X(R64G64_UINT) X(R64G64_SINT) X(R64G64_SFLOAT) X(R64G64B64_UINT) X(R64G64B64_SINT) X(R64G64B64_SFLOAT) \
    X(R64G64B64A64_UINT) X(R64G64B64A64_SINT) X(R64G64B64A64_SFLOAT) X(B10G11R11_UFLOAT_PACK32) \
    X(E5B9G9R9_UFLOAT_PACK32) X(D16_UNORM) X(X8_D24_UNORM_PACK32) X(D32_SFLOAT) X(S8_UINT) \
    X(D16_UNORM_S8_UINT) X(D24_UNORM_S8_UINT) X(D32_SFLOAT_S8_UINT) X(BC1_RGB_UNORM_BLOCK) \
	X(BC1_RGB_SRGB_BLOCK) X(BC1_RGBA_UNORM_BLOCK) X(BC1_RGBA_SRGB_BLOCK) X(BC2_UNORM_BLOCK) \
    X(BC2_SRGB_BLOCK) X(BC3_UNORM_BLOCK) X(BC3_SRGB_BLOCK) X(BC4_UNORM_BLOCK) X(BC4_SNORM_BLOCK) \
    X(BC5_UNORM_BLOCK) X(BC5_SNORM_BLOCK) X(BC6H_UFLOAT_BLOCK) X(BC6H_SFLOAT_BLOCK) X(BC7_UNORM_BLOCK) \
    X(BC7_SRGB_BLOCK) X(ETC2_R8G8B8_UNORM_BLOCK) X(ETC2_R8G8B8_SRGB_BLOCK) X(ETC2_R8G8B8A1_UNORM_BLOCK) \
    X(ETC2_R8G8B8A1_SRGB_BLOCK) X(ETC2_R8G8B8A8_UNORM_BLOCK) X(ETC2_R8G8B8A8_SRGB_BLOCK) \
	X(EAC_R11_UNORM_BLOCK) X(EAC_R11_SNORM_BLOCK) X(EAC_R11G11_UNORM_BLOCK) X(EAC_R11G11_SNORM_BLOCK) \
    X(ASTC_4x4_UNORM_BLOCK) X(ASTC_4x4_SRGB_BLOCK) X(ASTC_5x4_UNORM_BLOCK) X(ASTC_5x4_SRGB_BLOCK) \
    X(ASTC_5x5_UNORM_BLOCK) X(ASTC_5x5_SRGB_BLOCK) X(ASTC_6x5_UNORM_BLOCK) X(ASTC_6x5_SRGB_BLOCK) \
	X(ASTC_6x6_UNORM_BLOCK) X(ASTC_6x6_SRGB_BLOCK) X(ASTC_8x5_UNORM_BLOCK) X(ASTC_8x5_SRGB_BLOCK) \
    X(ASTC_8x6_UNORM_BLOCK) X(ASTC_8x6_SRGB_BLOCK) X(ASTC_8x8_UNORM_BLOCK) X(ASTC_8x8_SRGB_BLOCK) \
    X(ASTC_10x5_UNORM_BLOCK) X(ASTC_10x5_SRGB_BLOCK) X(ASTC_10x6_UNORM_BLOCK) X(ASTC_10x6_SRGB_BLOCK) \
    X(ASTC_10x8_UNORM_BLOCK) X(ASTC_10x8_SRGB_BLOCK) X(ASTC_10x10_UNORM_BLOCK) X(ASTC_10x10_SRGB_BLOCK) \
    X(ASTC_12x10_UNORM_BLOCK) X(ASTC_12x10_SRGB_BLOCK) X(ASTC_12x12_UNORM_BLOCK) X(ASTC_12x12_SRGB_BLOCK) \
    X(G8B8G8R8_422_UNORM) X(B8G8R8G8_422_UNORM) X(G8_B8_R8_3PLANE_420_UNORM) X(G8_B8R8_2PLANE_420_UNORM) \
    X(G8_B8_R8_3PLANE_422_UNORM) X(G8_B8R8_2PLANE_422_UNORM) X(G8_B8_R8_3PLANE_444_UNORM) X(R10X6_UNORM_PACK16) \
    X(R10X6G10X6_UNORM_2PACK16) X(R10X6G10X6B10X6A10X6_UNORM_4PACK16) X(G10X6B10X6G10X6R10X6_422_UNORM_4PACK16) \
    X(B10X6G10X6R10X6G10X6_422_UNORM_4PACK16) X(G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16) \
    X(G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16) X(G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16) \
    X(G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16) X(G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16) \
    X(R12X4_UNORM_PACK16) X(R12X4G12X4_UNORM_2PACK16) X(R12X4G12X4B12X4A12X4_UNORM_4PACK16) \
    X(G12X4B12X4G12X4R12X4_422_UNORM_4PACK16) X(B12X4G12X4R12X4G12X4_422_UNORM_4PACK16) \
    X(G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16) X(G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16) \
    X(G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16) X(G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16) \
    X(G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16) X(G16B16G16R16_422_UNORM) X(B16G16R16G16_422_UNORM) \
    X(G16_B16_R16_3PLANE_420_UNORM) X(G16_B16R16_2PLANE_420_UNORM) X(G16_B16_R16_3PLANE_422_UNORM) \
    X(G16_B16R16_2PLANE_422_UNORM) X(G16_B16_R16_3PLANE_444_UNORM) X(G8_B8R8_2PLANE_444_UNORM) \
    X(G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16) X(G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16) \
    X(G16_B16R16_2PLANE_444_UNORM) X(A4R4G4B4_UNORM_PACK16) X(A4B4G4R4_UNORM_PACK16) \
    X(ASTC_4x4_SFLOAT_BLOCK) X(ASTC_5x4_SFLOAT_BLOCK) X(ASTC_5x5_SFLOAT_BLOCK) \
    X(ASTC_6x5_SFLOAT_BLOCK) X(ASTC_6x6_SFLOAT_BLOCK) X(ASTC_8x5_SFLOAT_BLOCK) \
    X(ASTC_8x6_SFLOAT_BLOCK) X(ASTC_8x8_SFLOAT_BLOCK) X(ASTC_10x5_SFLOAT_BLOCK) \
    X(ASTC_10x6_SFLOAT_BLOCK) X(ASTC_10x8_SFLOAT_BLOCK) X(ASTC_10x10_SFLOAT_BLOCK) \
    X(ASTC_12x10_SFLOAT_BLOCK) X(ASTC_12x12_SFLOAT_BLOCK) \
	X(PVRTC1_2BPP_UNORM_BLOCK_IMG) X(PVRTC1_4BPP_UNORM_BLOCK_IMG) \
    X(PVRTC2_2BPP_UNORM_BLOCK_IMG) X(PVRTC2_4BPP_UNORM_BLOCK_IMG) X(PVRTC1_2BPP_SRGB_BLOCK_IMG) \
	X(PVRTC1_4BPP_SRGB_BLOCK_IMG) X(PVRTC2_2BPP_SRGB_BLOCK_IMG) X(PVRTC2_4BPP_SRGB_BLOCK_IMG)

void format_single(Arena& arena, VkFormat iFormat)
{
	#define FORMAT_ENUM(NAME) case VK_FORMAT_##NAME: format_single(arena, #NAME); break;
	switch (iFormat)
	{
		X_VK_FORMATS(FORMAT_ENUM)
		default: format(arena, "[VkFormat ", (int)iFormat, "]"); break;
	}
	#undef FORMAT_ENUM
}

#define X_VK_COLOR_SPACES(X) \
    X(SRGB_NONLINEAR_KHR) X(DISPLAY_P3_NONLINEAR_EXT) X(EXTENDED_SRGB_LINEAR_EXT) \
	X(DISPLAY_P3_LINEAR_EXT) X(DCI_P3_NONLINEAR_EXT) X(BT709_LINEAR_EXT) \
    X(BT709_NONLINEAR_EXT) X(BT2020_LINEAR_EXT) X(HDR10_ST2084_EXT) \
    X(DOLBYVISION_EXT) X(HDR10_HLG_EXT) X(ADOBERGB_LINEAR_EXT) X(ADOBERGB_NONLINEAR_EXT) \
    X(PASS_THROUGH_EXT) X(EXTENDED_SRGB_NONLINEAR_EXT) X(DISPLAY_NATIVE_AMD)

void format_single(Arena& arena, VkColorSpaceKHR color_space)
{
	#define FORMAT_ENUM(NAME) case VK_COLOR_SPACE_##NAME: format_single(arena, #NAME); break;
	switch (color_space)
	{
		X_VK_COLOR_SPACES(FORMAT_ENUM)
		default: format(arena, "[VkColorSpaceKHR ", (int)color_space, "]"); break;
	}
	#undef FORMAT_ENUM
}

#define DEBUG_UTILS_MESSAGE_SEVERITY_ALL            \
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT \
|   VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT    \
|   VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT \
|   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT

#define DEBUG_UTILS_MESSAGE_TYPE_ALL                \
    VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT     \
|   VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT  \
|   VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT

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
    return Failed;                  \
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
	log::verbose("Graphics context created!");
    return Success;
}

void Graphics::destroy()
{
	log::info("Destroying Graphics...");

	if (device)
		vkDeviceWaitIdle(device);

	if (fences[0]) {
		forn (MAX_SWAP_CHAIN_IMAGES) {
			vkDestroySemaphore(device, present_ready_semaphore[i], nullptr);
		}
		forn (MAX_FRAMES_IN_FLIGHT) {
			vkDestroySemaphore(device, image_ready_semaphore[i], nullptr);
			vkDestroyFence(device, fences[i], nullptr);
		}
	}

	if (transfer_command_pool)
		vkDestroyCommandPool(device, transfer_command_pool, nullptr);

	if (command_pool)
		vkDestroyCommandPool(device, command_pool, nullptr);
	
	if (swap_chain)
	forn (image_count)
		vkDestroyImageView(device, image_views[i], nullptr);

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

	memset(this, 0, sizeof(*this));
}

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
		log::verbose(TAB, pad(layer_name, 36), " ", description);
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
		log::verbose(TAB, extension_name);
	}
    scratch_arena.reset();
}

auto Graphics::create_instance(bool debug) -> Result
{
    log::verbose("Creating Vulkan instance...");
    log::verbose("Graphics debugging enabled: ", debug);
	if (debug) log_layers_and_extensions();

	debug = false;
	VkApplicationInfo application_info {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "How did you find this?",
		.applicationVersion = VK_MAKE_API_VERSION(1, 0, 0, 69),
		.pEngineName = "Fission",
		//! TODO: engine version
	//	.engineVersion = vk::make_api_version<1,version_major,version_minor,version_patch>,
		.apiVersion = VK_API_VERSION_1_3,
	};

	const char* extension_names[] = {
    #if defined(OS_MACOS)
        VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
    #endif
		VK_KHR_SURFACE_EXTENSION_NAME,
	#if defined(OS_WINDOWS)
		"VK_KHR_win32_surface",
	#elif defined(OS_ANDROID)
		"VK_KHR_android_surface",
    #elif defined(OS_MACOS)
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
		else if (vkCreateDebugUtilsMessengerEXT(instance, &debug_utils_info, nullptr, &debug_messenger) < VK_SUCCESS)
			log::error("Failed to create debug messenger");
	}
    return Success;
}

auto Graphics::create_surface(Window* window) -> Result
{
	scoped_set(logger.prefix, OS_NAME);
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

    if (count == 0)
        return log::error("Unable to find physical device with Vulkan support!"), Failed;

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

		const char* device_name = properties.deviceName;

		log::info((d == physical_device)? TAB">> ":TAB"   ", "GPU ", i++, ": ",
            dt(properties.deviceType), device_name);
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
#if 1 // ? need GLFW here ?
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

	VkPhysicalDeviceFeatures features {};
	//! TODO: look at features

/*
  https://vulkan-tutorial.com/en/Drawing_a_triangle/Setup/Logical_device_and_queues
	"Previous implementations of Vulkan made a distinction between instance
	and device specific validation layers, but this is no longer the case.
	That means that the enabledLayerCount and ppEnabledLayerNames fields
	of VkDeviceCreateInfo are ignored by up-to-date implementations.
	However, it is still a good idea to set them anyway to be compatible
	with older implementations"
*/
	//! TODO: only do this on debug builds
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

size_t pick_surface_format(Arena::Temp_Array<VkSurfaceFormatKHR> const& formats) {
	size_t index = 0;
	int max_score = 0;
	forn (formats.count) {
		int score = [](VkSurfaceFormatKHR const& sf) {switch (sf.format) {
			case VK_FORMAT_B8G8R8A8_SRGB:  return 4;
			case VK_FORMAT_R8G8B8A8_SRGB:  return 3;
            case VK_FORMAT_B8G8R8A8_UNORM: return 2;
            case VK_FORMAT_R8G8B8A8_UNORM: return 1;
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

	log::verbose("min image count = ", capabilities.minImageCount);
    log::verbose("max image count = ", capabilities.maxImageCount);
	log::verbose("supported composite alpha = ", (int)capabilities.supportedCompositeAlpha);
	
	VkCompositeAlphaFlagBitsKHR composite_alpha = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
	if (capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR)
		composite_alpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
	if (capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
		composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	// Pick surface format
	{
		u32 surface_format_count {};
		check(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &surface_format_count, nullptr),
			"Failed to enumerate surface formats");

		if (surface_format_count == 0)
			return log::error("Unable to find any surface formats!"), Failed;

		auto surface_formats = scratch_arena.temp_array<VkSurfaceFormatKHR>(surface_format_count);
		check(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &surface_format_count, surface_formats.data),
			"Failed to enumerate surface formats");

    	log::verbose("Number of surface formats: ", surface_formats.count);
		size_t picked = pick_surface_format(surface_formats);
		forn (surface_formats.count) {
			using namespace formatting;
		    log::verbose((i == picked)? TAB">> ":TAB"   ", pad(surface_formats[i].format, 32), surface_formats[i].colorSpace);
		}
		format = surface_formats[picked].format;
		color_space = surface_formats[picked].colorSpace;
	}

	extent = capabilities.currentExtent;
    transform = capabilities.currentTransform;
    log::verbose("Surface transform: ", transform);

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
    log::verbose("Swap chain size: ", extent.width, "x", extent.height);
	//! TODO: present mode formatter
	log::verbose("Swap chain present mode: ", int(present_mode));

	VkSwapchainCreateInfoKHR swap_chain_info {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = surface,
		.minImageCount = capabilities.minImageCount,// + 1,
		.imageFormat = format,
		.imageColorSpace = color_space,
		.imageExtent = extent,
		.imageArrayLayers = 1, /* For non-stereoscopic-3D applications, this value is 1 */
		.imageUsage = image_usage,
		.preTransform = transform,
		.compositeAlpha = composite_alpha, // <-- TODO: fix this (ANDROID)
		.presentMode = present_mode, // TODO: this needs to be configurable
		.clipped = VK_TRUE, /* "... allows more efficient presentation methods to be used on some platforms." */
	};

	check(vkCreateSwapchainKHR(device, &swap_chain_info, nullptr, &swap_chain),
		  "Failed to create swap chain");
	return Success;
}
auto Graphics::create_sc_image_views() -> Result
{
	log::verbose("Creating swap chain image views...");

	check(vkGetSwapchainImagesKHR(device, swap_chain, &image_count, nullptr),
		  "vkGetSwapchainImagesKHR failed");

	log::verbose("Number of swap chain images: ", image_count);

	assert(image_count <= 8, "Image count must be less than 8");
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
	log::verbose("Creating command buffers...");

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
	
	forn (MAX_SWAP_CHAIN_IMAGES) {
		check(vkCreateSemaphore(device, &semaphore_info, nullptr, &present_ready_semaphore[i]),
			  "Failed to create semaphore");
	}

	VkFenceCreateInfo fence_info {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT,
	};

	forn (MAX_FRAMES_IN_FLIGHT) {
		check(vkCreateSemaphore(device, &semaphore_info, nullptr, &image_ready_semaphore[i]),
			  "Failed to create semaphore");
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
#endif

// --------------------------------------------------------------------------------
// Render Pass Creator

VkPipelineStageFlags Render_Pass_Creator::pick_stage_mask_from_access_mask(VkAccessFlags access)
{
	switch (access)
	{
	case VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT:         return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	case VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT: return VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	case VK_ACCESS_SHADER_READ_BIT:                    return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	default:                                           return 0;
	}
}

VkImageLayout Render_Pass_Creator::pick_final_image_layout_for_format(VkFormat format)
{
	switch (format)
	{
	case VK_FORMAT_D16_UNORM:
	case VK_FORMAT_X8_D24_UNORM_PACK32:
	case VK_FORMAT_D32_SFLOAT:
	case VK_FORMAT_D16_UNORM_S8_UINT:
	case VK_FORMAT_D24_UNORM_S8_UINT:
	case VK_FORMAT_D32_SFLOAT_S8_UINT:   return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	default:                             return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}
}

Render_Pass_Creator::Render_Pass_Creator(Arena& iArena)
: arena(iArena)
{
	// Allocate space for VkAttachmentReference because they are small
	attachment_references.data = arena.alloc<VkAttachmentReference>(max_attachment_reference_count);
}

Render_Pass_Creator& Render_Pass_Creator::add_external_subpass_dependency(uint32_t subpass)
{
	auto ptr = arena.push<VkSubpassDependency>({
		.srcSubpass = VK_SUBPASS_EXTERNAL,
		.dstSubpass = subpass,
		.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.srcAccessMask = 0,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
	});
	if (subpass_dependencies.data == nullptr)
		subpass_dependencies.data = ptr;
	subpass_dependencies.count += 1;
	return *this;
}

Render_Pass_Creator& Render_Pass_Creator::add_dependency(uint32_t src_subpass, uint32_t dst_subpass, VkAccessFlags src_access, VkAccessFlags dst_access)
{
	auto ptr = arena.push<VkSubpassDependency>({
		.srcSubpass = src_subpass,
		.dstSubpass = dst_subpass,
		.srcStageMask = pick_stage_mask_from_access_mask(src_access),
		.dstStageMask = pick_stage_mask_from_access_mask(dst_access),
		.srcAccessMask = src_access,
		.dstAccessMask = dst_access,
	});
	if (subpass_dependencies.data == nullptr)
		subpass_dependencies.data = ptr;
	subpass_dependencies.count += 1;
	return *this;
}

Render_Pass_Creator& Render_Pass_Creator::add_subpass(std::initializer_list<VkAttachmentReference> const& refs)
{
	ASSERT(attachments.count > 0 && subpass_dependencies.count == 0);
	ASSERT(attachment_references.count + refs.size() < max_attachment_reference_count);
	VkSubpassDescription subpass = {
		.flags = 0,
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = (u32)refs.size(),
		.pColorAttachments = attachment_references.data + attachment_references.count,
	};
	memcpy(attachment_references.data + attachment_references.count, refs.begin(), refs.size() * sizeof(VkAttachmentReference));
	attachment_references.count += subpass.colorAttachmentCount;
	auto ptr = arena.push(subpass);
	if (subpasses.data == nullptr)
		subpasses.data = ptr;
	subpasses.count += 1;
	return *this;
}

Render_Pass_Creator& Render_Pass_Creator::add_attachment(VkFormat format, Attachment_Preset preset, VkSampleCountFlagBits sample_count)
{
	ASSERT(subpasses.count == 0);
	VkAttachmentDescription attachment = {
		.flags = 0,
		.format = format,
		.samples = sample_count,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	};
	switch (preset) {
		break; case Attachment_Preset_New_Image_Present:
			attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		break; case Attachment_Preset_New_Image:
			ASSERT(false);
		break; case Attachment_Preset_Cumulative_Image:
			ASSERT(false);
		break;
	}
	auto ptr = arena.push(attachment);
	if (attachments.data == nullptr)
		attachments.data = ptr;
	attachments.count += 1;
	return *this;
}

VkResult Render_Pass_Creator::create(VkRenderPass* pRenderPass) {
	VkRenderPassCreateInfo info {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = (u32)attachments.count,
		.pAttachments    = attachments.data,
		.subpassCount    = (u32)subpasses.count,
		.pSubpasses      = subpasses.data,
		.dependencyCount = (u32)subpass_dependencies.count,
		.pDependencies   = subpass_dependencies.data,
	};
	return vkCreateRenderPass(engine.graphics.device, &info, nullptr, pRenderPass);
}

// --------------------------------------------------------------------------------
// Pipeline Creator

Pipeline_Creator& Pipeline_Creator::add_dynamic_state(VkDynamicState state)
{
	dynamic_states.emplace_back(state);
	return *this;
}

VkShaderModule create_shader(size_t size, void const* data)
{
	VkShaderModuleCreateInfo shader_info {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = size,
		.pCode = reinterpret_cast<const uint32_t*>(data),
	};
	VkShaderModule shader;
	vkCreateShaderModule(engine.graphics.device, &shader_info, nullptr, &shader);
	return shader;
}

Pipeline_Creator& Pipeline_Creator::add_shader(VkShaderStageFlagBits stage, VkShaderModule shader)
{
	VkPipelineShaderStageCreateInfo info {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = stage,
		.module = shader,
	};
	switch (stage) {
		case VK_SHADER_STAGE_COMPUTE_BIT:  info.pName = "computeMain"; break;
		case VK_SHADER_STAGE_VERTEX_BIT:   info.pName = "vertexMain"; break;
		case VK_SHADER_STAGE_FRAGMENT_BIT: info.pName = "fragmentMain"; break;
		default: info.pName = "main"; break;
	}
	shaders.emplace_back(info);
	return *this;
}

VkResult Pipeline_Creator::create(VkPipeline* pPipeline, VkPipelineLayout layout, VkRenderPass render_pass)
{
	dynamic_state.dynamicStateCount = (u32)dynamic_states.size();
	dynamic_state.pDynamicStates = dynamic_states.data();
	color_blend_state.pAttachments = &blend_attachment;

	VkGraphicsPipelineCreateInfo info {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount          = (u32)shaders.size(),
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
		.subpass             = 0,
	};
	return vkCreateGraphicsPipelines(engine.graphics.device, 0, 1, &info, nullptr, pPipeline);
}

void Graphics::set_default_viewport(VkCommandBuffer cmd) {
    VkViewport viewport {
        .x = 0.0f,
        .y = 0.0f,
        .width  = static_cast<float>(extent.width),
        .height = static_cast<float>(extent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };
    vkCmdSetViewport(cmd, 0, 1, &viewport);
}

void Graphics::set_default_scissor(VkCommandBuffer cmd) {
    VkRect2D scissor {
        .offset = { 0, 0 },
        .extent = extent,
    };
    vkCmdSetScissor(cmd, 0, 1, &scissor);
}

void set_viewport_and_scissor(VkCommandBuffer cmd, rf32 rect) {
	VkRect2D scissor {
		.offset = {},
		.extent = { .width = (u32)rect.width(), .height = (u32)rect.height() },
	};
	vkCmdSetScissor(cmd, 0, 1, &scissor);

	VkViewport viewport {
		.x = 0.0f,
		.y = 0.0f,
		.width = rect.width(),
		.height = rect.height(),
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};
	vkCmdSetViewport(cmd, 0, 1, &viewport);
}

// --------------------------------------------------------------------------------
// Renderer_2d

void Renderer_2d::create(VkRenderPass render_pass, VkPipelineLayout pipeline_layout, Draw_Data_2d* ref_draw_data, Options options)
{
	draw_data = ref_draw_data;

	VkShaderModule shader_module;
	VkShaderModuleCreateInfo info {
    	.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    	.codeSize = size_t(embedded::draw2d_spv_end - embedded::draw2d_spv_start),
    	.pCode = (u32*)embedded::draw2d_spv_start,
	};
	vkCreateShaderModule(engine.graphics.device, &info, nullptr, &shader_module);

	auto vertex_layout = Draw_Data_2d::vertex::Layout{};
	auto pc = Pipeline_Creator{}
		.vertex_layout(vertex_layout)
		.add_dynamic_state(VK_DYNAMIC_STATE_VIEWPORT)
		.add_dynamic_state(VK_DYNAMIC_STATE_SCISSOR)
		.add_shader(VK_SHADER_STAGE_VERTEX_BIT, shader_module)
		.add_shader(VK_SHADER_STAGE_FRAGMENT_BIT, shader_module);
    pc.input_assembly_state.topology = options.topology;
	pc.create(&pipeline, pipeline_layout, render_pass);

	vkDestroyShaderModule(engine.graphics.device, shader_module, nullptr);
}

void Renderer_2d::draw(Render_Context const& ctx)
{
	vkCmdBindPipeline(ctx.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	draw_data->flush_batch(ctx.command_buffer, ctx.frame);
}

END_NAMESPACE()
