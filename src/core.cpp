#include "Fission/core.hpp"

namespace os {
	auto init() -> fission::Result;
	auto info() -> const Info& { return _info; }
}

BEGIN_NAMESPACE(fission)

#define check(Result, ...) if ((result = (Result)) < VK_SUCCESS) { log::error(__VA_ARGS__); return stop(); } (void)0

auto Window::pop_all_events(Arena& arena) -> array<Event>
{
	auto data = arena.next_ptr<Event>();
	u32 count = 0;
	u32 tail = event_tail;
	while (event_head != tail)
	{
		arena.push(event_queue[event_head]);
		count += 1;
		event_head = (event_head + 1) % array_count(event_queue);
	}
	return {count, data};
}

Window::~Window() {
	//! NOTE: Exiting application, no need to do anything
	log::info("YOU ARE TERMINATED");
}

Arena& temp_arena() { return engine.temp_arena; }

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

bool stop() {
    engine.flags &= ~Engine::Running;
    return false;
}

auto Engine::create(Defaults const& defaults) -> Result
{
	//scoped_set(logger.minimum_level, log::Verbose);
	if (os::init()) return Failed;
    //! TODO: log to same directory as the executable
	logger.backing_file = os::open_file("fission.log", os::Write);
    os_mutex_create(&logger.mutex);
    log::info("Creating Fission Engine...");
	engine.temp_arena.create(16_MiB);
	engine.frame_arena.create(8_MiB);

	//! TODO: setup in-game console here

	Window::Create_Info window_info {
		.title = defaults.window_title,
        .width = defaults.window_width,
        .height = defaults.window_height,
    };
    if (window.create(window_info)) return Failed;
    
    Graphics::Create_Info graphics_info {
		.window = &window,
        .debug = true,
    };
	if (graphics.create(graphics_info)) return Failed;

	log::verbose("Creating overlay render pass...");
    Render_Pass_Creator{}
        .add_attachment(graphics.format, Attachment_Preset_New_Image_Present)
        .add_subpass({ {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL} })
        .add_external_subpass_dependency(0)
        .create(&overlay_render_pass);

	log::verbose("Creating Pipeline layout...");
	{
		VkPushConstantRange push {
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
			.offset = 0,
			.size = sizeof(vec4),
		};
		VkPipelineLayoutCreateInfo pipelineLayoutInfo {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.pushConstantRangeCount = 1,
			.pPushConstantRanges = &push,
		};
		vkCreatePipelineLayout(engine.graphics.device, &pipelineLayoutInfo, nullptr, &pipeline_layout);
	}
	log::verbose("Creating framebuffers...");
	create_frame_buffers();
	log::verbose("Creating draw data...");
	draw_data.create(graphics);
	log::verbose("Creating renderer...");
	renderer.create(overlay_render_pass, pipeline_layout, &draw_data);
	log::verbose("Creating line renderer...");
	line_renderer.create(overlay_render_pass, pipeline_layout, &draw_data, {.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST});

    engine.flags |= Engine::Running;
    log::info("Starting render thread...");
	if (os_thread_start(render_main, nullptr, &engine.render_thread))
		return log::error("Failed to start render thread!"), Failed;
	return Success;
}

void Engine::destroy()
{
    log::verbose(__PRETTY_FUNCTION__);
	os_thread_join(render_thread);
//	graphics.destroy();
}

auto Engine::setup() -> Result
{
	log::verbose("Setting up render thread...");
	last_ticks = ticks();
    return Success;
}

//! TODO: move this out
namespace vk
{
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
	VkResult result {VK_SUCCESS};
	frame_arena.reset();
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
		flags &=~ Graphics_Recreate_Swap_Chain;
	}
	
	Render_Context render_context { .frame = frame_count & 1 };
	VkFence fence = graphics.fences[render_context.frame];
	VkSemaphore image_ready_semaphore = graphics.image_ready_semaphore[render_context.frame];
	
    check(vkWaitForFences(graphics.device, 1, &fence, true, UINT64_MAX), "[vkWaitForFences] failed");
    check(vkResetFences(graphics.device, 1, &fence), "[vkResetFences] failed");

	result = VK_ERROR_UNKNOWN;
	while (result != VK_SUCCESS) {
		result = vkAcquireNextImageKHR(graphics.device, graphics.swap_chain,
			UINT64_MAX, image_ready_semaphore, VK_NULL_HANDLE, &render_context.image_index);

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
	
	render_context.command_buffer = graphics.command_buffers[render_context.frame];
	render_context.frame_buffer = frame_buffers[render_context.image_index];

	begin(render_context.command_buffer);

	vk::begin(render_context.command_buffer, overlay_render_pass, render_context.frame_buffer, {{}});
	graphics.set_default_scissor(render_context.command_buffer);
	graphics.set_default_viewport(render_context.command_buffer);

	f32 aspect = f32(graphics.extent.height) / f32(graphics.extent.width);
	vec4 scale {aspect, 1.0f, 1.0f, 1.0f};
	vkCmdPushConstants(render_context.command_buffer, pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(vec4), &scale);

	f64 dt = seconds_elapsed_and_reset(last_ticks);
	array<Event> events = window.pop_all_events(frame_arena);
	on_update(dt, events, render_context);
	
#ifdef FRAMETIMES
    {
		const u32 n = 256;
		local_persist f32 frame_times[n] = {};
		u32 pos = frame_count % n;
		frame_times[pos] = dt;
        u32 v = draw_data.current.vertex_count;
        
		forn (n-1) {
			draw_data.push_index(v+i);
			draw_data.push_index(v+i+1);
		}

		forn (n) {
			f32 x = (f32(i)/f32(n-1))*2.0f - 1.0f;
			f32 y = frame_times[i] / 0.100f - 0.166f;
			s32 k = (pos - i) % n;
			if (k < 0) k = -k;
        	draw_data.push_vertex({{x, y}, {}, rgba8(255,u8(k),u8(k))});
		}
        line_renderer.draw(render_context);
    }
#endif
	draw_data.send(graphics, render_context.frame);

	vkCmdEndRenderPass(render_context.command_buffer);
	vkEndCommandBuffer(render_context.command_buffer);

	VkSemaphore present_ready_semaphore = graphics.present_ready_semaphore[render_context.image_index];
	VkPipelineStageFlags wait_mask { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSubmitInfo submit_info {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &image_ready_semaphore,
		.pWaitDstStageMask = &wait_mask,
		.commandBufferCount = 1,
		.pCommandBuffers = &render_context.command_buffer,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &present_ready_semaphore,
	};
	check(vkQueueSubmit(graphics.graphics_queue, 1, &submit_info, fence), "[vkQueueSubmit] failed");

	VkPresentInfoKHR present_info {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &present_ready_semaphore,
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

	frame_count += 1;

	return bool(flags & Running);
}

auto Engine::create_frame_buffers() -> Result
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

    // Create
    g.create_swap_chain(&window);
    g.create_sc_image_views();
	create_frame_buffers();
}

void Engine::shutdown()
{
	log::verbose("Exiting render thread...");
//	window.close();
}

END_NAMESPACE()
