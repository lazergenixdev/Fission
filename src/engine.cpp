#include "internal.hpp"
#include <Fission/core/engine.hpp>
#include <format.hpp>

using fmt::format;
using namespace fs;

fs::Engine engine {
    .version = {
        version_major,
        version_minor,
        version_patch
    }
};

auto Engine::create(Defaults const& defaults) -> bool
{
    (void)defaults;
    log::info("Creating Fission Engine...");

    if (window.create({
        .title = __TITLE__,
    })) return true;

	if (graphics.create({
		.window = &window,
        .debug = true,
	})) return true;

    window.show();

    if (create_layers()) return true;

	engine.current_scene = on_create_scene({});
    engine.flags |= Engine::Running;

    log::debug("Creating render thread...");
	if (os_thread_start(fs::render_main, nullptr, &engine.render_thread)) {
		log::error("Failed to start render thread!");
		return true;
	}

    log::verbose(format("logger is_open() => {}", FS_BTF(engine.logger.file.is_open())));

    return false;
}

#define check(VK, MSG) if (VK < VK_SUCCESS) { log::error(MSG); return true; } (void)0

auto Engine::create_layers() -> bool
{
    check(vk::Render_Pass_Creator{4}
        .add_attachment(graphics.sc_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
        .add_subpass({ {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL} })
        .add_external_subpass_dependency(0)
        .create(&overlay_render_pass),
        "Failed to create render pass");

	create_frame_buffers(0);

    return false;
}

auto Engine::create_frame_buffers(u32 old_count) -> bool
{
	if (graphics.sc_image_count > old_count) {
		bump_allocator {graphics.sc_image_count * sizeof(VkFramebuffer)}
			.alloc_to(frame_buffers, graphics.sc_image_count)
			.release();
	}

	VkFramebufferCreateInfo frame_buffer_info {
		.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		.renderPass = overlay_render_pass,
		.attachmentCount = 1,
		.width  = graphics.sc_extent.width,
		.height = graphics.sc_extent.height,
		.layers = 1,
	};

	for_n (graphics.sc_image_count) {
		frame_buffer_info.pAttachments = graphics.sc_image_views + i;
		vkCreateFramebuffer(graphics.device, &frame_buffer_info, nullptr, frame_buffers + i);
	}

	return false;
}

#undef check

void Engine::destroy()
{
    log::info("Destroying Fission Engine...");
    engine.flags &=~ Engine::Running; // Ensure render thread will terminate
    if (engine.render_thread) os_thread_join(engine.render_thread);
	vkDeviceWaitIdle(graphics.device);
	current_scene->~Scene();
	for_n (graphics.sc_image_count)
		vkDestroyFramebuffer(graphics.device, frame_buffers[i], nullptr);
	FISSION_DEFAULT_FREE(frame_buffers);
	vkDestroyRenderPass(graphics.device, overlay_render_pass, nullptr);
    graphics.destroy();
}

auto Engine::setup() -> bool
{
	last_timestamp = fs::timestamp();
	return false;
}

void Engine::shutdown()
{
	window.close();
}

bool stop() {
    engine.flags &=~ Engine::Running;
    return false;
}

#define check(Result, What) if (Result < VK_SUCCESS) { log::error(What); return stop(); } (void)0
#define checkf(Result, What, ...) if ((result = (Result)) < VK_SUCCESS) { log::error(format(What, __VA_ARGS__)); return stop(); } (void)0
#define unlikely [[unlikely]] // pretty sure this does nothing, but it's a nice thought

auto Engine::render_frame() -> bool
{
	std::vector<fs::Event> events;

    VkResult result { VK_SUCCESS };
	Render_Context render_context { .frame = frame_count & 1 };
	VkSemaphore write_semaphore = graphics.sc_image_write_semaphore[render_context.frame];
	VkSemaphore read_semaphore  = graphics.sc_image_read_semaphore[render_context.frame];
	VkFence     fence           = graphics.cb_fences[render_context.frame];

#if 0
	unlikely if (window.is_minimized()) {
		window.sleep_until_not_minimized();
	}
#endif
#if 0
	unlikely if (flags & fChange_Scene) {
		auto next_scene = on_create_scene(next_scene_key);
		vkDeviceWaitIdle(graphics.device);
		delete current_scene;
		current_scene = next_scene;
		flags &= ~fChange_Scene;
	}
#endif
	unlikely if (flags & Graphics_Recreate_Swap_Chain) {
		resize();
		flags &=~ Graphics_Recreate_Swap_Chain;
	}
#if 0
#if defined(FISSION_PLATFORM_WINDOWS)
		auto timer = CreateWaitableTimerExW(NULL, NULL, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
#endif
		if (flags & fFPS_Limiter_Enable) {
#if defined(FISSION_PLATFORM_WINDOWS)
			auto time_between_frames = s64(1e7f / fps_limit);
			auto next = fps_last + time_between_frames;
			auto now = timestamp();

			LARGE_INTEGER due_time;
			due_time.QuadPart = -((next - now) / 100);
			if (due_time.QuadPart < 0 && SetWaitableTimerEx(timer, &due_time, 0, NULL, NULL, NULL, 0)) {
				WaitForSingleObject(timer, INFINITE);
			}
			fps_last = next;
#endif
		}
#if defined(FISSION_PLATFORM_WINDOWS)
		CloseHandle(timer);
#endif
#endif

//=====================================================================================

	//	https://github.com/google/vulkan-pre-rotation-demo
    checkf(vkWaitForFences(graphics.device, 1, &fence, VK_TRUE, UINT64_MAX), "[vkWaitForFences] failed with {}", (int)result);
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
			log::error(format("Failed to get swap chain image [vkAcquireNextImageKHR => {}]", (int)result));
			return stop(); // should try to recover here?
		}
	}

//-------------------------------------------------------------------------------------

	auto cpu_start = timestamp();

	render_context.frame_buffer   = frame_buffers[render_context.image_index];
	render_context.command_buffer = graphics.command_buffers[render_context.frame];

	check(vk::begin(render_context.command_buffer), "Failed to begin command buffer");

	//-------------------------------------------------------------------------------------
	// Eat any events handled by debug and console layers
	window.event_queue.pop_all(events);
	//debug_layer.handle_events(events);
	//console_layer.handle_events(events);
	//-------------------------------------------------------------------------------------

	VkClearValue clear_color = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
	VkRenderPassBeginInfo begin_info {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = overlay_render_pass,
		.framebuffer = frame_buffers[render_context.image_index],
		.renderArea = {
			.offset = {0, 0},
			.extent = graphics.sc_extent,
		},
		.clearValueCount = 1,
		.pClearValues = &clear_color,
	};
	vkCmdBeginRenderPass(render_context.command_buffer, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
	current_scene->on_update(delta_time, events, render_context);
	vkCmdEndRenderPass(render_context.command_buffer);

	//-------------------------------------------------------------------------------------
	// Render console and debug overlay
#if 0
	vk::begin(render_context.command_buffer, overlay_render_pass);
	{
		bind_font(render_context.command_buffer, &fonts.console);
		console_layer.on_update(dt, &render_context);

		bind_font(render_context.command_buffer, &fonts.debug);
		debug_layer.on_update(dt, &render_context);
	}
	vkCmdEndRenderPass(render_context.command_buffer);
#endif
	//-------------------------------------------------------------------------------------

#if 0
	if (flags & fSave_Currect_Frame) {
		auto cmd = render_context.command_buffer;
		auto image = graphics.sc_images[render_context.image_index];

		VkImageSubresourceRange range;
		range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.baseMipLevel = 0;
		range.levelCount = 1;
		range.baseArrayLayer = 0;
		range.layerCount = 1;
		VkImageMemoryBarrier imageBarrier = {};
		imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageBarrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		imageBarrier.image = image;
		imageBarrier.subresourceRange = range;
		imageBarrier.srcAccessMask = 0;
		imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier);

		VkBufferImageCopy copy = {};
		copy.imageExtent = { .width = graphics.sc_extent.width, .height = graphics.sc_extent.height, .depth = 1 };
		copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copy.imageSubresource.layerCount = 1;
		vkCmdCopyImageToBuffer(cmd, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, screenshot_buffer, 1, &copy);

		VkImageMemoryBarrier imageBarrier_toReadable{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		imageBarrier_toReadable.image = image;
		imageBarrier_toReadable.subresourceRange = range;
		imageBarrier_toReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		imageBarrier_toReadable.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		imageBarrier_toReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageBarrier_toReadable.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toReadable);
	}
#endif

	vkEndCommandBuffer(render_context.command_buffer);

	//renderer_2d.end_render(&render_context);
	//textured_renderer_2d.end_render(&render_context);

	//-------------------------------------------------------------------------------------

	debug_layer.cpu_time = (float)seconds_elasped_and_reset(cpu_start);

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
			log::error(format("[vkQueuePresentKHR] failed with {}", (u32)result));
			return stop();
		}
	}

#if 0
	if (flags & fSave_Currect_Frame) {
		vkWaitForFences(graphics.device, 1, &fence, VK_TRUE, UINT64_MAX);
		void* gpu_memory = nullptr;
		vmaMapMemory(graphics.allocator, screenshot_allocation, &gpu_memory);
		void* data = convert_to_rgb(gpu_memory, graphics.sc_extent.width * graphics.sc_extent.height);
		vmaUnmapMemory(graphics.allocator, screenshot_allocation);

		time_t rawtime;
		time(&rawtime);
		auto timeinfo = localtime(&rawtime);

		char filename[48];
		"screenshot_%04d-%02d-%02d_%02d-%02d-%02d.png"_fmt(filename,
			timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday,
			timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec
		);

		stbi_write_png(filename, graphics.sc_extent.width, graphics.sc_extent.height, 3, data, 3 * graphics.sc_extent.width);
		delete[] reinterpret_cast<byte*>(data);
		flags &= ~fSave_Currect_Frame;
	}
#endif

//=====================================================================================

	delta_time = fs::seconds_elasped_and_reset(last_timestamp);
	frame_count += 1;

	return flags & Running;
}

// TODO: error handling
void Engine::resize() {
    auto& g = graphics;
    vkDeviceWaitIdle(g.device);

    // Destroy
    vkDestroySwapchainKHR(g.device, g.swap_chain, nullptr);
    for_n (g.sc_image_count) vkDestroyImageView(g.device, g.sc_image_views[i], nullptr);
    for_n (g.sc_image_count) vkDestroyFramebuffer(g.device, frame_buffers[i], nullptr);

    // Create
    g.create_swap_chain(&window);

	u32 old_image_count = g.sc_image_count;
    g.create_sc_image_views();

	create_frame_buffers(old_image_count);
}
