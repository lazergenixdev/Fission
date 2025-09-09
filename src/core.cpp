#include "Fission/core.hpp"

namespace os { auto init() -> fission::Result; }

BEGIN_NAMESPACE(fission)

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

void log::write_log_from_logging_arena(int level)
{
    static constexpr const char * level_colors [] {
        "\x1b[90m", "\x1b[96m", "\x1b[0m", "\x1b[93m", "\x1b[91m",
    };
	fwrite(logging_arena.start, 1, logging_arena.allocated, logging_file);
	fflush(logging_file);
#if defined(OS_WINDOWS)
	if (auto handle = os::output_console()) {
        WORD attr = FOREGROUND_INTENSITY;
        if (level == Info)  attr |= FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
        if (level == Debug) attr  = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        if (level == Warn)  attr  = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        if (level == Error) attr  = FOREGROUND_RED | FOREGROUND_INTENSITY;
        SetConsoleTextAttribute(handle, attr);

		DWORD offset = 0;
		DWORD count = DWORD(logging_arena.allocated);
		DWORD written;
		while (WriteConsoleA(handle, (byte*)logging_arena.start + offset, count, &written, NULL))
		{
			offset += written;
			count -= written;
			if (offset >= logging_arena.allocated) break;
		}
	}
	else {
		fputs(level_colors[level], stdout);
		fwrite(logging_arena.start, 1, logging_arena.allocated, stdout);
	}
#else
	fputs(level_colors[level], stdout);
    fwrite(logging_arena.start, 1, logging_arena.allocated, stdout);
#endif
#if !defined(OS_WINDOWS)
    if (level != Info) fputs("\x1b[0m", stdout);
#endif
	fflush(stdout);
#if defined(OS_WINDOWS) // Also output to debugger (if available)
	ASSERT(strlen((char*)logging_arena.start) < logging_arena.capacity);
    OutputDebugStringA((char*)logging_arena.start);
#endif
}

auto Engine::create(Defaults const& defaults) -> Result
{
	if (os::init()) return Failed;
    logging_arena.create(1_KiB);
	logging_file = fopen("log.txt", "wb");
    os_mutex_create(&logging_mutex);
    log::info("Creating Fission Engine...", 100);
    /*
	// setup the console early so we can use it as soon as possible
	console_layer.setup_console_api();
	add_engine_console_commands();
    */
    Window::Create_Info window_info = {
        .width = defaults.window_width,
        .height = defaults.window_height,
    };
    if (window.create(window_info)) return Failed;
    
    Graphics::Create_Info graphics_info = {
		.window = &window,
        .debug = true,
    };
	if (graphics.create(graphics_info)) return Failed;
    
    //window.show();

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

bool stop() {
    engine.flags &= ~Engine::Running;
    return false;
}

#define check(Result, ...) if ((result = (Result)) < VK_SUCCESS) { log::error(__VA_ARGS__); return stop(); } (void)0

auto Engine::render_frame() -> bool
{
	Sleep(100);
	#if 0
    VkResult       result {VK_SUCCESS};
	Render_Context render_context { .frame = frame_count & 1 };
	VkSemaphore    write_semaphore = graphics.image_write_semaphore[render_context.frame];
	VkSemaphore    read_semaphore  = graphics.image_read_semaphore[render_context.frame];
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
    check(vkWaitForFences(graphics.device, 1, &fence, VK_TRUE, UINT64_MAX), "[vkWaitForFences] failed with ", (int)result);
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

	// graphics.set_default_scissor(render_context.command_buffer);
	// graphics.set_default_viewport(render_context.command_buffer);
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
#endif
	return bool(flags & Running);
}

void Engine::shutdown()
{
	log::verbose("Exiting render thread...");
//	window.close();
}

void Engine::resize()
{
	log::verbose("Want resize...");
}

END_NAMESPACE()
