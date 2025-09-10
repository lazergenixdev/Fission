#include "Fission/core.hpp"

namespace os {
	auto init() -> fission::Result;
	auto info() -> const Info& { return _info; }
}

BEGIN_NAMESPACE(fission)

#define check(Result, ...) if ((result = (Result)) < VK_SUCCESS) { log::error(__VA_ARGS__); return stop(); } (void)0

Window::~Window() {
	//! NOTE: Exiting application, no need to do anything
	log::info("YOU ARE TERMINATED");
}

Logging_Timestamp Logging_Timestamp::now()
{
    using namespace std::chrono;
    auto now = system_clock::now();
    auto ms = duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;
    std::time_t t = system_clock::to_time_t(now);
    std::tm local {};
#if defined(OS_WINDOWS)
    localtime_s(&local, &t);
#else
    localtime_r(&t, &local);
#endif
    return {
        .date = ((u32(local.tm_year) + 1900) << 16)
              | ((u32(local.tm_mon) + 1) << 8)
              | (u32(local.tm_mday)),
        .time = (u32(local.tm_hour) << 16)
              | (u32(local.tm_min) << 8)
              | (u32(local.tm_sec)),
        .milliseconds = u32(ms),
    };
}

void log::write_log_from_logger(int level)
{
    static constexpr const char * level_colors [] {
        "\x1b[90m", "\x1b[96m", "\x1b[0m", "\x1b[93m", "\x1b[91m",
    };
	fwrite(logger.arena.start, 1, logger.arena.allocated, logger.backing_file);
	fflush(logger.backing_file);
#if defined(OS_WINDOWS)
	if (auto handle = os::output_console()) {
        WORD attr = FOREGROUND_INTENSITY;
        if (level == Info)  attr |= FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
        if (level == Debug) attr  = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        if (level == Warn)  attr  = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        if (level == Error) attr  = FOREGROUND_RED | FOREGROUND_INTENSITY;
        SetConsoleTextAttribute(handle, attr);

		DWORD offset = 0;
		DWORD count = DWORD(logger.arena.allocated);
		DWORD written;
		while (WriteConsoleA(handle, (byte*)logger.arena.start + offset, count, &written, NULL))
		{
			offset += written;
			count -= written;
			if (offset >= logger.arena.allocated) break;
		}
	}
	else {
		fputs(level_colors[level], stdout);
		fwrite(logger.arena.start, 1, logger.arena.allocated, stdout);
	}
#else
	fputs(level_colors[level], stdout);
    fwrite(logger.arena.start, 1, logger.arena.allocated, stdout);
#endif
#if !defined(OS_WINDOWS)
    if (level != Info) fputs("\x1b[0m", stdout);
#endif
	fflush(stdout);
#if defined(OS_WINDOWS) // Also output to debugger (if available)
	ASSERT(strlen((char*)logger.arena.start) < logger.arena.capacity);
    OutputDebugStringA((char*)logger.arena.start);
#endif
}

bool stop() {
    engine.flags &= ~Engine::Running;
    return false;
}

namespace vk {
	struct Render_Pass_Creator {
		std::vector<VkAttachmentDescription> attachments;
		std::vector<VkAttachmentReference> attachment_references;
		std::vector<VkSubpassDescription> subpasses;
		std::vector<VkSubpassDependency> subpass_dependencies;

		Render_Pass_Creator(size_t attachment_reference_count) {
			attachment_references.reserve(attachment_reference_count);
		}

		Render_Pass_Creator& add_external_subpass_dependency(uint32_t subpass) {
			VkSubpassDependency dependency{};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = subpass;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			subpass_dependencies.emplace_back(dependency);
			return *this;
		}

		VkPipelineStageFlags pick_stage_mask_from_access_mask(VkAccessFlags access) {
			switch (access)
			{
			case VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT:         return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			case VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT: return VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			case VK_ACCESS_SHADER_READ_BIT:                    return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			default:                                           return 0;
			}
		}

		Render_Pass_Creator& add_dependency(uint32_t src_subpass, uint32_t dst_subpass, VkAccessFlags src_access, VkAccessFlags dst_access) {
			VkSubpassDependency dependency{};
			dependency.srcSubpass = src_subpass;
			dependency.dstSubpass = dst_subpass;
			dependency.srcStageMask = pick_stage_mask_from_access_mask(src_access);
			dependency.srcAccessMask = src_access;
			dependency.dstStageMask = pick_stage_mask_from_access_mask(dst_access);
			dependency.dstAccessMask = dst_access;
			subpass_dependencies.emplace_back(dependency);
			return *this;
		}

		Render_Pass_Creator& add_subpass(std::initializer_list<VkAttachmentReference> const& refs) {
			VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = (u32)refs.size();
			subpass.pColorAttachments = attachment_references.data() + attachment_references.size();
			for (auto&& ref : refs) attachment_references.emplace_back(ref);
			subpasses.emplace_back(subpass);
			return *this;
		}
		Render_Pass_Creator& add_subpass_input(std::initializer_list<VkAttachmentReference> const& refs, std::initializer_list<VkAttachmentReference> const& input_refs) {
			VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = (u32)refs.size();
			subpass.inputAttachmentCount = (u32)input_refs.size();
			subpass.pColorAttachments = attachment_references.data() + attachment_references.size();
			for (auto&& ref : refs) attachment_references.emplace_back(ref);
			subpass.pInputAttachments = attachment_references.data() + attachment_references.size();
			for (auto&& ref : input_refs) attachment_references.emplace_back(ref);
			subpasses.emplace_back(subpass);
			return *this;
		}

		Render_Pass_Creator& add_subpass(std::initializer_list<VkAttachmentReference> const& refs, VkAttachmentReference depth_ref) {
			VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = (u32)refs.size();
			subpass.pColorAttachments = attachment_references.data() + attachment_references.size();
			//	subpass.pResolveAttachments = &colorAttachmentResolveRef; // MSAA
			for (auto&& ref : refs) attachment_references.emplace_back(ref);
			subpass.pDepthStencilAttachment = attachment_references.data() + attachment_references.size();
			attachment_references.emplace_back(depth_ref);
			subpasses.emplace_back(subpass);
			return *this;
		}
		Render_Pass_Creator& add_subpass(std::initializer_list<VkAttachmentReference> const& refs, VkAttachmentReference depth_ref, VkAttachmentReference resolve_ref) {
			VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = (u32)refs.size();
			subpass.pColorAttachments = attachment_references.data() + attachment_references.size();
			for (auto&& ref : refs) attachment_references.emplace_back(ref);
			subpass.pDepthStencilAttachment = attachment_references.data() + attachment_references.size();
			attachment_references.emplace_back(depth_ref);
			subpass.pResolveAttachments = attachment_references.data() + attachment_references.size(); // MSAA
			attachment_references.emplace_back(resolve_ref);
			subpasses.emplace_back(subpass);
			return *this;
		}

		Render_Pass_Creator& add_subpass_with_input_attachment(std::initializer_list<VkAttachmentReference> const& refs, VkAttachmentReference input_ref) {
			VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = (u32)refs.size();
			subpass.pColorAttachments = attachment_references.data() + attachment_references.size();
			//	subpass.pResolveAttachments = &colorAttachmentResolveRef; // MSAA
			for (auto&& ref : refs) attachment_references.emplace_back(ref);
			subpass.pInputAttachments = attachment_references.data() + attachment_references.size();
			subpass.inputAttachmentCount = 1;
			attachment_references.emplace_back(input_ref);
			subpasses.emplace_back(subpass);
			return *this;
		}

		VkImageLayout pick_final_image_layout_for_format(VkFormat format) {
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

		enum Attachment_Preset {
			// Use this for Depth and Color images that we want to write to
			Attachment_New_Image,        // load = CLEAR, store = STORE, stencil = DONT CARE, inital = UNDEFINED
			Attachment_Cumulative_Image, // load = LOAD , store = STORE, stencil = DONT CARE, inital = UNDEFINED
		};

		Render_Pass_Creator& add_attachment(VkFormat format, VkSampleCountFlagBits sample_count = VK_SAMPLE_COUNT_1_BIT) {
			VkAttachmentDescription attachment{};
			attachment.format = format;
			attachment.samples = sample_count;
			attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachment.finalLayout = pick_final_image_layout_for_format(format);
			attachments.emplace_back(attachment);
			return *this;
		}
		Render_Pass_Creator& add_attachment(VkFormat format, VkAttachmentLoadOp loadOp, VkImageLayout layout, VkSampleCountFlagBits sample_count = VK_SAMPLE_COUNT_1_BIT) {
			VkAttachmentDescription attachment{};
			attachment.format = format;
			attachment.samples = sample_count;
			attachment.loadOp = loadOp;
			attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachment.initialLayout = layout;
			attachment.finalLayout = layout;
			attachments.emplace_back(attachment);
			return *this;
		}

		Render_Pass_Creator& add_attachment(
			VkFormat format,
			VkImageLayout initial_layout,
			VkImageLayout final_layout,
			VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
			VkSampleCountFlagBits sample_count = VK_SAMPLE_COUNT_1_BIT
		) {
			attachments.emplace_back(VkAttachmentDescription {
				.format = format,
				.samples = sample_count,
				.loadOp = loadOp,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout = initial_layout,
				.finalLayout = final_layout,
			});
			return *this;
		}

		VkResult create(VkRenderPass* pRenderPass);
	};
}

auto Engine::create(Defaults const& defaults) -> Result
{
	if (os::init()) return Failed;
	logger.backing_file = os::open_file("log.txt", os::Write);
    os_mutex_create(&logger.mutex);
    log::info("Creating Fission Engine...");
	engine.temp_arena.create(4_MiB);

    /*
	// setup the console early so we can use it as soon as possible
	console_layer.setup_console_api();
	add_engine_console_commands();
    */
    Window::Create_Info window_info = {
		.title = defaults.window_title,
        .width = defaults.window_width,
        .height = defaults.window_height,
    };
    if (window.create(window_info)) return Failed;
    
    Graphics::Create_Info graphics_info = {
		.window = &window,
        .debug = true,
    };
	if (graphics.create(graphics_info)) return Failed;

    vk::Render_Pass_Creator{4}
        .add_attachment(graphics.format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_ATTACHMENT_LOAD_OP_CLEAR)
        .add_subpass({ {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL} })
        .add_external_subpass_dependency(0)
        .create(&overlay_render_pass);

	create_frame_buffers(0);

    /*
	if (create_screenshot_buffer()) return true;
    if (create_layers())            return true;

	engine.current_scene = on_create_scene({});
    */
    engine.flags |= Engine::Running;
    log::debug("Creating render thread...");
	if (os_thread_start(render_main, nullptr, &engine.render_thread))
		return log::error("Failed to start render thread!"), Failed;
	return Success;
}

void Engine::destroy()
{
#ifdef TEST_FMT
    log::verbose(fmt::format("{}", __PRETTY_FUNCTION__));
#else
    log::verbose(__PRETTY_FUNCTION__);
#endif
}

auto Engine::setup() -> Result
{
	log::verbose("Setting up render thread...");
    return Success;
}

//! TODO: move this out
namespace vk
{
	inline VkResult begin(VkCommandBuffer command_buffer, VkCommandBufferUsageFlags flags = 0)
	{
		VkCommandBufferBeginInfo begin_info {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = flags,
		};
		return vkBeginCommandBuffer(command_buffer, &begin_info);
	}
    void begin(VkCommandBuffer command_buffer, VkRenderPass render_pass, VkFramebuffer frame_buffer, VkClearColorValue color)
    {
        VkClearValue clear_color = { color };
        VkRenderPassBeginInfo begin_info {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = render_pass,
            .framebuffer = frame_buffer,
            .renderArea = { {0, 0}, engine.graphics.extent },
            .clearValueCount = 1,
            .pClearValues = &clear_color,
        };
        vkCmdBeginRenderPass(command_buffer, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
    }
}

auto Engine::render_frame() -> bool
{
    VkResult       result {VK_SUCCESS};
	Render_Context render_context { .frame = frame_count & 1 };
	VkSemaphore    write_semaphore = graphics.image_write_semaphore[render_context.frame];
	VkFence        fence           = graphics.fences[render_context.frame];

#if 0
	unlikely if (window.is_minimized()) {
		window.sleep_until_not_minimized();
	}
#endif
#if 0
	unlikely if (flags & Change_Scene) {
		auto next_scene = on_create_scene(next_scene_key);
		vkDeviceWaitIdle(graphics.device);
		current_scene->~Scene(); // TODO: call delete?
		current_scene = next_scene;
		flags &= ~Change_Scene;
	}
#endif
	if (flags & Graphics_Recreate_Swap_Chain) {
		resize();
		flags &= ~Graphics_Recreate_Swap_Chain;
	}
#if defined(FISSION_PLATFORM_WINDOWS) && false
	{
		auto timer = CreateWaitableTimerExW(NULL, NULL, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
		if (flags & fFPS_Limiter_Enable) {
			auto time_between_frames = s64(1e7f / fps_limit);
			auto next = fps_last + time_between_frames;
			auto now = timestamp();

			LARGE_INTEGER due_time;
			due_time.QuadPart = -((next - now) / 100);
			if (due_time.QuadPart < 0 && SetWaitableTimerEx(timer, &due_time, 0, NULL, NULL, NULL, 0)) {
				WaitForSingleObject(timer, INFINITE);
			}
			fps_last = next;
		}
		CloseHandle(timer);
	}
#endif

//=====================================================================================

	//	https://github.com/google/vulkan-pre-rotation-demo
    check(vkWaitForFences(graphics.device, 1, &fence, VK_TRUE, UINT64_MAX), "[vkWaitForFences] failed");
    check(vkResetFences(graphics.device, 1, &fence), "[vkResetFences] failed");

	result = VK_ERROR_UNKNOWN;
	while (result != VK_SUCCESS) {
		result = vkAcquireNextImageKHR(graphics.device, graphics.swap_chain,
			UINT64_MAX, write_semaphore, VK_NULL_HANDLE, &render_context.image_index);

		if (result == VK_SUBOPTIMAL_KHR) break;
		else if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			resize();
			continue;
		}
		else if (result != VK_SUCCESS) {
			log::error("Failed to get swap chain image with ", (int)result);
			return stop(); // should try to recover here?
		}
	}

//-------------------------------------------------------------------------------------

//	auto cpu_start = timestamp();

	render_context.frame_buffer   = frame_buffers[render_context.image_index];
	render_context.command_buffer = graphics.command_buffers[render_context.frame];

	check(vk::begin(render_context.command_buffer), "Failed to begin command buffer");

	//-------------------------------------------------------------------------------------
	// Eat any events handled by debug and console layers
	// window.event_queue.pop_all(events);
	// debug_layer  .handle_events(events);
	// console_layer.handle_events(events);
	//-------------------------------------------------------------------------------------

	 graphics.set_default_scissor(render_context.command_buffer);
	 graphics.set_default_viewport(render_context.command_buffer);
	// current_scene->on_update(delta_time, events, render_context);

	//-------------------------------------------------------------------------------------
	// Render console and debug overlay
	vk::begin(render_context.command_buffer, overlay_render_pass, render_context.frame_buffer, {});
	{
	//	graphics.set_default_scissor(render_context.command_buffer);
	//	graphics.set_default_viewport(render_context.command_buffer);
//
	//	bind_font(render_context.command_buffer, &font.console);
	//	console_layer.on_update(delta_time, &render_context);
//
	//	bind_font(render_context.command_buffer, &font.debug);
	//	debug_layer.on_update(delta_time, &render_context);
	}
	vkCmdEndRenderPass(render_context.command_buffer);
	//-------------------------------------------------------------------------------------

	//if (flags & Save_Current_Frame) save_frame(render_context);

	vkEndCommandBuffer(render_context.command_buffer);

	//renderer_2d         .end_render(render_context);
	//textured_renderer_2d.end_render(render_context);

	//-------------------------------------------------------------------------------------

	//debug_layer.cpu_time = (float)seconds_elasped_and_reset(cpu_start);

	VkSemaphore read_semaphore = graphics.image_read_semaphore[render_context.image_index];
	VkPipelineStageFlags wait_mask { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSubmitInfo submit_info {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &write_semaphore,
		.pWaitDstStageMask = &wait_mask,
		.commandBufferCount = 1,
		.pCommandBuffers = &render_context.command_buffer,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &read_semaphore,
	};
	check(vkQueueSubmit(graphics.graphics_queue, 1, &submit_info, fence), "[vkQueueSubmit] failed");

	//-------------------------------------------------------------------------------------

	VkPresentInfoKHR present_info {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &read_semaphore,
		.swapchainCount = 1,
		.pSwapchains = &graphics.swap_chain,
		.pImageIndices = &render_context.image_index,
	};
	result = vkQueuePresentKHR(graphics.present_queue, &present_info);

	if (result != VK_SUCCESS) {
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			if (!(flags & Running)) return stop(); // ok, we head out
			resize();
		}
		else {
			log::error("[vkQueuePresentKHR] failed with ", (u32)result);
			return stop();
		}
	}

	if (flags & Save_Current_Frame) {
		// We must wait for the gpu to finish before we can read image data
		vkWaitForFences(graphics.device, 1, &fence, VK_TRUE, UINT64_MAX);
	//	write_frame();
	}

//=====================================================================================

	//delta_time = fs::seconds_elasped_and_reset(last_timestamp);
	frame_count += 1;

	return bool(flags & Running);
}

auto Engine::create_frame_buffers(u32 old_count) -> Result
{
	VkFramebufferCreateInfo frame_buffer_info {
		.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		.renderPass = overlay_render_pass,
		.attachmentCount = 1,
		.width  = graphics.extent.width,
		.height = graphics.extent.height,
		.layers = 1,
	};

	forn (graphics.image_count) {
		frame_buffer_info.pAttachments = graphics.image_views + i;
		vkCreateFramebuffer(graphics.device, &frame_buffer_info, nullptr, frame_buffers + i);
	}

	return Success;
}

// TODO: error handling
void Engine::resize()
{
    auto& g = graphics;
    vkDeviceWaitIdle(g.device);

    // Destroy
    vkDestroySwapchainKHR(g.device, g.swap_chain, nullptr);
    forn (g.image_count) vkDestroyImageView(g.device, g.image_views[i], nullptr);
    forn (g.image_count) vkDestroyFramebuffer(g.device, frame_buffers[i], nullptr);
	u32 old_image_count = g.image_count;

    // Create
    g.create_swap_chain(&window);
    g.create_sc_image_views();

	create_frame_buffers(old_image_count);

	// Update screen transform
	//using namespace glm;
	//auto size = graphics.size();
	//Transform_2D_Data transform {
	//	.transform = mat4(graphics.pre_rotation()) * (mat4x4 {
	//		{ 2.0f / (float)size.x, 0.0f, 0.0f, 0.0f },
	//		{ 0.0f, 2.0f / (float)size.y, 0.0f, 0.0f },
	//		{ 0.0f, 0.0f, 1.0f, 0.0f },
	//		{ -1.0f, -1.0f, 0.0f, 1.0f },
	//	})
	//};

	//graphics.upload(transform_2d.buffer, &transform, sizeof(transform));

//	current_scene->on_resize(old_image_count);
}

void Engine::shutdown()
{
	log::verbose("Exiting render thread...");
//	window.close();
}

VkResult vk::Render_Pass_Creator::create(VkRenderPass* pRenderPass) {
	VkRenderPassCreateInfo render_pass_info {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = (u32)attachments.size(),
		.pAttachments    = attachments.data(),
		.subpassCount    = (u32)subpasses.size(),
		.pSubpasses      = subpasses.data(),
		.dependencyCount = (u32)subpass_dependencies.size(),
		.pDependencies   = subpass_dependencies.data(),
	};
	return vkCreateRenderPass(engine.graphics.device, &render_pass_info, nullptr, pRenderPass);
}

END_NAMESPACE()
