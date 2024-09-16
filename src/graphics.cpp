#define VMA_IMPLEMENTATION
#include <internal.hpp>

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

VKAPI_ATTR VkBool32 VKAPI_CALL debug_utils_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT,
    const VkDebugUtilsMessengerCallbackDataEXT*, void*);

using std::format;
using namespace fs;

//#define LOG_FUNCTION(MESSAGE) log::debug("(" __FUNCTION__ "): " MESSAGE)
#define LOG_FUNCTION(MESSAGE) log::debug(MESSAGE)

#define check(FUNC, ERROR_MESSAGE) \
  if (FUNC != VK_SUCCESS) {        \
    log::error(ERROR_MESSAGE);     \
    return true;                   \
  }                                \
  (void)0

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

    if (create_instance(info.debug)) return true;
    if (create_surface(info.window)) return true;
    if (pick_physical_device())      return true;
    if (pick_queue_families())       return true;
    if (create_device())             return true;
    if (create_allocator())          return true;
    if (create_swap_chain())         return true;
    if (create_sc_image_views())     return true;
    if (create_command_buffers())    return true;
    if (create_sync_objects())       return true;

    return false;
}

Graphics::~Graphics()
{
	log::verbose("Graphics destructor...");

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
	if (instance) vkDestroyInstance(instance, nullptr);
}

auto Graphics::create_instance(bool debug) -> bool
{
    LOG_FUNCTION("Creating Vulkan instance...");
	check_layers_and_extensions();

	VkApplicationInfo application_info {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "How did you find this?",
		.applicationVersion = VK_MAKE_API_VERSION(1, 0, 0, 69),
		.pEngineName = "Fission",
		.engineVersion = vk::make_api_version<1,
			version_major, version_minor, version_patch>,
		.apiVersion = VK_API_VERSION_1_3,
	};

	const char* extension_names[] = {
		VK_KHR_SURFACE_EXTENSION_NAME,
		FISSION_PLATFORM_VULKAN_EXTENSION_NAMES,
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
	};

    log::verbose(format(
		"Platform Extensions: {}",
		MACRO_STRING_EXPAND(FISSION_PLATFORM_VULKAN_EXTENSION_NAMES)
	));

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
		.pApplicationInfo = &application_info,
		.enabledLayerCount = debug? 1u:0u,
		.ppEnabledLayerNames = layer_names,
		.enabledExtensionCount = vk::count(extension_names) - (debug? 0u:1u),
		.ppEnabledExtensionNames =  extension_names,
	};

    check(vkCreateInstance(&instance_info, nullptr, &instance),
          "Failed to create instance!");

#if 0 // Do I need this?
	if (debug) {
		auto vkCreateDebugUtilsMessengerEXT =
			(PFN_vkCreateDebugUtilsMessengerEXT)
			vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

		if (vkCreateDebugUtilsMessengerEXT == nullptr) {
			log::error(
				"Could not load function \"vkCreateDebugUtilsMessengerEXT\"");
		}
		else 
		check(vkCreateDebugUtilsMessengerEXT(instance, &debug_utils_info, nullptr, &debug_messenger),
			  "Failed to create debug messenger");
	}
#endif

    return false;
}

auto Graphics::create_surface(Window* window) -> bool
{
    LOG_FUNCTION("Creating Vulkan surface...");

#if   defined(FISSION_PLATFORM_WINDOWS)

	VkWin32SurfaceCreateInfoKHR surface_info {
		.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		.hwnd = window->_handle,
    };

    check(vkCreateWin32SurfaceKHR(instance, &surface_info, nullptr, &surface),
          "Failed to create surface!");

#elif defined(FISSION_PLATFORM_LINUX)

#endif

    return false;
}

auto Graphics::pick_physical_device() -> bool
{
    LOG_FUNCTION("Picking Vulkan physical device...");
	
	u32 count;
    check(vkEnumeratePhysicalDevices(instance, &count, nullptr),
          "Failed to enumerate physical devices");

    assert(count != 0, "Unable to find physical device with Vulkan support!");

	std::vector<VkPhysicalDevice> physical_devices{count};
    check(vkEnumeratePhysicalDevices(instance, &count, physical_devices.data()),
          "Failed to enumerate physical devices");

	for (auto&& [i,dev]: enumerate(physical_devices)) {
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(dev, &props);
		
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(dev, &deviceFeatures);

		auto dt = [](VkPhysicalDeviceType t) {
			switch (t)
			{
			case VK_PHYSICAL_DEVICE_TYPE_OTHER:          return "(Other)";
			case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return "(Integrated GPU)";
			case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:   return "(Descrete GPU)";
			case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:    return "(Virtual GPU)";
			case VK_PHYSICAL_DEVICE_TYPE_CPU:            return "(CPU)";
			default:                                     return "(Unknown)";
			}
		};

		log::debug(format(
			"   GPU {:2}: {:14} {}",
			i, dt(props.deviceType), props.deviceName)
		);
	}

    physical_device = physical_devices[0];

    return false;
}

bool Graphics::pick_queue_families()
{
	LOG_FUNCTION("Picking Vulkan queue families...");
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

bool Graphics::create_device()
{
	LOG_FUNCTION("Creating Vulkan device...");

    unique_queue_family_set<3> queue_family_set;
	queue_family_set.add(extra.queue_family.graphics);
    queue_family_set.add(extra.queue_family.transfer);
	queue_family_set.add(extra.queue_family.present);

	const char* device_extensions[] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};

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

	VkPhysicalDeviceFeatures deviceFeatures {};
	deviceFeatures.fillModeNonSolid  = VK_TRUE;
	deviceFeatures.sampleRateShading = VK_TRUE;
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	deviceFeatures.geometryShader    = VK_TRUE;

    VkDeviceCreateInfo device_info {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = queue_family_set.count,
        .pQueueCreateInfos = queue_family_set.get(),
		.enabledLayerCount = vk::count(layer_names),
		.ppEnabledLayerNames = layer_names,
	    .enabledExtensionCount = vk::count(device_extensions),
	    .ppEnabledExtensionNames = device_extensions,
        .pEnabledFeatures = &deviceFeatures,
    };

	check(vkCreateDevice(physical_device, &device_info, nullptr, &device),
          "Failed to create device");

	vkGetDeviceQueue(device, extra.queue_family.graphics, 0, &graphics_queue);
	vkGetDeviceQueue(device, extra.queue_family.present , 0, &present_queue);
	vkGetDeviceQueue(device, extra.queue_family.transfer, 0, &transfer_queue);
    
    return false;
}

bool Graphics::create_allocator()
{
	log::debug("Createing Vulkan memory allocator (VMA) ...");

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

bool Graphics::create_swap_chain()
{
	LOG_FUNCTION("Creating Vulkan swap chain...");

	VkSurfaceCapabilitiesKHR capabilities;
	check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities),
		  "Failed to get Vulkan surface capabilities");

	sc_format = VK_FORMAT_B8G8R8A8_UNORM;
	sc_extent = capabilities.currentExtent;

	VkSwapchainCreateInfoKHR swap_chain_info{
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = surface,
		.minImageCount = capabilities.minImageCount + 1,
		.imageFormat = sc_format,
		.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
		.imageExtent = sc_extent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = VK_PRESENT_MODE_FIFO_KHR,
	};

	check(vkCreateSwapchainKHR(device, &swap_chain_info, nullptr, &swap_chain),
		  "Failed to create swap chain");

	return false;
}

bool Graphics::create_sc_image_views()
{
	LOG_FUNCTION("Creating Vulkan swap chain image views...");

	vkGetSwapchainImagesKHR(device, swap_chain, &sc_image_count, nullptr);
	if (sc_image_count > Graphics::max_sc_images) {
		log::error("ERROR");
		return true;
	}

	vkGetSwapchainImagesKHR(device, swap_chain, &sc_image_count, sc_images);
	{
		auto view_info = vk::image_view_2d(VK_NULL_HANDLE, sc_format);
		for (u32 i = 0; i < sc_image_count; ++i) {
			view_info.image = sc_images[i];
			check(vkCreateImageView(device, &view_info, nullptr, sc_image_views + i),
				  "Failed to create swap chain ImageView");
		}
	}

	return false;
}

bool Graphics::create_command_buffers()
{
	LOG_FUNCTION("Creating Vulkan command buffers...");

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
	
	for (auto i: rangeu32(2)) {
		check(vkCreateFence(device, &fence_info, nullptr, cb_fences + i),
			  "Failed to create fence");
	}
	
	return false;
}

void check_layers_and_extensions()
{
    char buffer[128];
    string message {buffer};

    uint32_t layer_count {};
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

    std::vector<VkLayerProperties> layers {layer_count};
    vkEnumerateInstanceLayerProperties(&layer_count, layers.data());

    log::verbose(format("Number of layers available: {}", layer_count));

	for (auto&& [i,layer]: enumerate(layers)) {
		log::verbose(format("   {:3}: \"{}\" ({})",
			i, layer.layerName, layer.description
		));
	}

    uint32_t extension_count;
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

    std::vector<VkExtensionProperties> extensions(extension_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());

    log::verbose(format("Number of extensions available: {}", extension_count));
	
	for (auto&& [i,ext]: enumerate(extensions)) {
		log::verbose(format("   {:3}: \"{}\"",
			i, ext.extensionName
		));
	}
}

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

#if 0 // kinda annoying tbh
    if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        auto body = format("{}\n\n{}", data->pMessageIdName, data->pMessage);
        os::show_error_dialog(
			format("Graphics Validation [{}]", severity_string(severity)), body
		);
    }
#endif

	return VK_FALSE;
}


////////////////////////////////////////////////////////////////////
// Shaders

VkShaderModule vk::create_shader(size_t size, void const* data) {
	VkShaderModule module;
	VkShaderModuleCreateInfo createInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
	createInfo.codeSize = size;
	createInfo.pCode = reinterpret_cast<const uint32_t*>(data);
	vkCreateShaderModule(engine.graphics.device, &createInfo, nullptr, &module);
	return module;
}



VkResult vk::Pipeline_Creator::create_and_destroy_shaders(VkPipeline * pipeline) {
	auto result = create(pipeline);
	for (auto&& [sType, pNext, flags, stage, module, pName, pSpecializationInfo] : shaders)
		vkDestroyShaderModule(engine.graphics.device, module, nullptr);
	return result;
}

VkResult vk::Pipeline_Creator::create(VkPipeline * pipeline) {
	dynamic_state.dynamicStateCount = (fs::u32)dynamic_states.size();
	dynamic_state.pDynamicStates = dynamic_states.data();
	color_blend_state.pAttachments = &blend_attachment;

	VkGraphicsPipelineCreateInfo pipelineInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
	pipelineInfo.stageCount = (fs::u32)shaders.size();
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

VkResult vk::Pipeline_Layout_Creator::create(VkPipelineLayout* pLayout) {
	VkPipelineLayoutCreateInfo ci{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	ci.pSetLayouts = layouts.data();
	ci.setLayoutCount = (fs::u32)layouts.size();
	ci.pPushConstantRanges = push_ranges.data();
	ci.pushConstantRangeCount = (fs::u32)push_ranges.size();
	return vkCreatePipelineLayout(engine.graphics.device, &ci, nullptr, pLayout);
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
