#include "Fission/core/engine.hpp"
#include "Fission/core/console.hpp"
#include "freetype/freetype.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/rotate_normalized_axis.hpp"

using fmt::format;
using namespace fs;

void add_engine_console_commands();

struct Debug_Font {
#	include "../resources/BinaryFonts/IBMPlexMono-Medium.inl"
};
struct Console_Font {
#	include "../resources/BinaryFonts/JetBrainsMono-Regular.inl"
};

fs::Engine engine {
    .version = {
        version_major,
        version_minor,
        version_patch
    }
};

auto Engine::create(Defaults const& defaults) -> bool
{
    log::info("Creating Fission Engine...");

	// setup the console early so we can use it as soon as possible
	console_layer.setup_console_api();
	add_engine_console_commands();

    if (window.create({
        .title = defaults.window_title,
		.width = defaults.window_width,
		.height = defaults.window_height,
    })) return true;

	if (graphics.create({
		.window = &window,
        .debug = true,
	})) return true;

    window.show();

	if (create_screenshot_buffer()) return true;
    if (create_layers())            return true;

	engine.current_scene = on_create_scene({});
    engine.flags |= Engine::Running;

    log::debug("Creating render thread...");
	if (os_thread_start(fs::render_main, nullptr, &engine.render_thread)) {
		log::error("Failed to start render thread!");
		return true;
	}

    debug_layer.flags |= layer::show;

    return false;
}

#define check(VK, MSG) if (VK < VK_SUCCESS) { log::error(MSG); return true; } (void)0

auto Engine::create_layers() -> bool
{
    check(vk::Render_Pass_Creator{4}
        .add_attachment(graphics.sc_format, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
        .add_subpass({ {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL} })
        .add_external_subpass_dependency(0)
        .create(&overlay_render_pass),
        "Failed to create render pass");

	check(texture_layout.create(graphics), "Failed to create texture descriptor set layout");
	check(transform_2d.layout.create(graphics), "Failed to create 2d transform descriptor set layout");

	create_frame_buffers(0);

	// Descriptor Sets:
	// 0 = transform_2d, 1 = debug font, 2 = console font
	VkDescriptorSet sets[3] = {};
	{
		VkDescriptorPoolSize pool_sizes[] = {
			{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         32},
			{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 64},
		};
		VkDescriptorPoolCreateInfo descPoolInfo {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.maxSets = (32 + 64),
			.poolSizeCount = (u32)std::size(pool_sizes),
			.pPoolSizes = pool_sizes,
		};
		check(vkCreateDescriptorPool(graphics.device, &descPoolInfo, nullptr, &descriptor_pool),
			"Failed to create descriptor pool");

		VkDescriptorSetLayout layouts[3] = { transform_2d.layout, texture_layout, texture_layout };
		VkDescriptorSetAllocateInfo descSetAllocInfo {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = descriptor_pool,
			.descriptorSetCount = 3,
			.pSetLayouts = layouts,
		};
		check(vkAllocateDescriptorSets(graphics.device, &descSetAllocInfo, sets),
			"Failed to allocate descriptor sets");
	}

	// Create buffer for the 2d transform
	{
		VmaAllocationCreateInfo allocInfo { .usage = VMA_MEMORY_USAGE_AUTO };
		VkBufferCreateInfo bufferInfo {
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = sizeof(Transform_2D_Data),
			.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		};
		check(vmaCreateBuffer(graphics.allocator, &bufferInfo, &allocInfo, &transform_2d.buffer, &transform_2d.allocation, nullptr),
			"Failed to create buffer for 2d transform");

		using namespace glm;
		auto size = graphics.size();
		Transform_2D_Data transform {
			.transform = mat4(graphics.pre_rotation()) * (mat4x4 {
				{ 2.0f / (float)size.x, 0.0f, 0.0f, 0.0f },
				{ 0.0f, 2.0f / (float)size.y, 0.0f, 0.0f },
				{ 0.0f, 0.0f, 1.0f, 0.0f },
				{ -1.0f, -1.0f, 0.0f, 1.0f },
			})
		};
		graphics.upload(transform_2d.buffer, &transform, sizeof(transform));
	}

	// Create Descriptor Set for the transform
	{
		transform_2d.set = sets[0];
		VkDescriptorBufferInfo bufferInfo {
			.buffer = transform_2d.buffer,
			.offset = 0,
			.range = sizeof(Transform_2D_Data),
		};
		VkWriteDescriptorSet write {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = transform_2d.set,
			.dstBinding = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.pBufferInfo = &bufferInfo,
		};
		vkUpdateDescriptorSets(graphics.device, 1, &write, 0, nullptr);
	}

	{
		auto sampler_info = vk::sampler(VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
		check(vkCreateSampler(graphics.device, &sampler_info, nullptr, &font.sampler),
			"Failed to create sampler");
	}

	if (FT_Init_FreeType(&font.library) != 0) {
		log::error("Failed to create FreeType library");
		return true;
	}

	font.debug  .create(  Debug_Font::data,   Debug_Font::size, 18.0f, sets[1], font.sampler);
	font.console.create(Console_Font::data, Console_Font::size, 16.0f, sets[2], font.sampler);

	renderer_2d         .create(&graphics, overlay_render_pass, transform_2d.layout);
	textured_renderer_2d.create(&graphics, overlay_render_pass, transform_2d.layout, texture_layout);

	debug_layer.create();

	// Initialize console position (wtf is this???)
	console_layer.position = -(font.console.height + 1);

    return false;
}

auto Engine::create_frame_buffers(u32 old_count) -> bool
{
	// Allocate more space for frame buffers if image count changed
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

auto Engine::create_screenshot_buffer () -> bool
{
	VmaAllocationCreateInfo allocInfo {
		.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
		.usage = VMA_MEMORY_USAGE_AUTO,
	};
	VkBufferCreateInfo bufferInfo {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = 1920 * 1920 * sizeof(fs::rgba8), // TODO: fixed size is bad here. what else can we do?
		.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
	};
	check(vmaCreateBuffer(graphics.allocator, &bufferInfo, &allocInfo, &screenshot_buffer, &screenshot_allocation, nullptr),
		"Failed to create buffer for screenshots");

	return false;
}

#undef check

void Engine::destroy()
{
    log::info("Destroying Fission Engine...");

	// TODO:
	// Before doing anything that might crash the engine,
	//    save persistant data to files...

    engine.flags &= ~Running; // Ensure render thread will terminate
    if (engine.render_thread) os_thread_join(engine.render_thread);
	vkDeviceWaitIdle(graphics.device); // All graphics object must not be in use
	current_scene->~Scene();
	debug_layer.destroy();
	console_layer.destroy();
	renderer_2d.destroy();
	textured_renderer_2d.destroy();
	vkDestroySampler(graphics.device, font.sampler, nullptr);
	vmaDestroyBuffer(graphics.allocator, transform_2d.buffer, transform_2d.allocation);
	font.debug.destroy();
	font.console.destroy();
	FT_Done_FreeType(font.library);
	vkDestroyDescriptorPool(graphics.device, descriptor_pool, nullptr);
	vkDestroyDescriptorSetLayout(engine.graphics.device, transform_2d.layout, nullptr);
	vkDestroyDescriptorSetLayout(graphics.device, texture_layout, nullptr);
	vmaDestroyBuffer(engine.graphics.allocator, screenshot_buffer, screenshot_allocation);
	for_n (graphics.sc_image_count) vkDestroyFramebuffer(graphics.device, frame_buffers[i], nullptr);
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
	log::verbose("engine.shutdown()...");
	window.close();
}

bool stop() {
    engine.flags &= ~Engine::Running;
    return false;
}

#define check(Result, What) if (Result < VK_SUCCESS) { log::error(What); return stop(); } (void)0
#define checkf(Result, What, ...) if ((result = (Result)) < VK_SUCCESS) { log::error(format(What, __VA_ARGS__)); return stop(); } (void)0
#define unlikely [[unlikely]] // pretty sure this does nothing, but it's a nice thought

auto Engine::render_frame() -> bool
{
    VkResult       result {VK_SUCCESS};
	Render_Context render_context { .frame = frame_count & 1 };
	VkSemaphore    write_semaphore = graphics.sc_image_write_semaphore[render_context.frame];
	VkSemaphore    read_semaphore  = graphics.sc_image_read_semaphore[render_context.frame];
	VkFence        fence           = graphics.cb_fences[render_context.frame];

#if 0
	unlikely if (window.is_minimized()) {
		window.sleep_until_not_minimized();
	}
#endif
	unlikely if (flags & Change_Scene) {
		auto next_scene = on_create_scene(next_scene_key);
		vkDeviceWaitIdle(graphics.device);
		current_scene->~Scene(); // TODO: call delete?
		current_scene = next_scene;
		flags &= ~Change_Scene;
	}
	unlikely if (flags & Graphics_Recreate_Swap_Chain) {
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
	debug_layer  .handle_events(events);
	console_layer.handle_events(events);
	//-------------------------------------------------------------------------------------

	graphics.set_default_scissor(render_context.command_buffer);
	graphics.set_default_viewport(render_context.command_buffer);
	current_scene->on_update(delta_time, events, render_context);

	//-------------------------------------------------------------------------------------
	// Render console and debug overlay
	vk::begin(render_context.command_buffer, overlay_render_pass, render_context.frame_buffer, {});
	{
		graphics.set_default_scissor(render_context.command_buffer);
		graphics.set_default_viewport(render_context.command_buffer);

		bind_font(render_context.command_buffer, &font.console);
		console_layer.on_update(delta_time, &render_context);

		bind_font(render_context.command_buffer, &font.debug);
		debug_layer.on_update(delta_time, &render_context);
	}
	vkCmdEndRenderPass(render_context.command_buffer);
	//-------------------------------------------------------------------------------------

	if (flags & Save_Current_Frame) save_frame(render_context);

	vkEndCommandBuffer(render_context.command_buffer);

	renderer_2d         .end_render(render_context);
	textured_renderer_2d.end_render(render_context);

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

	if (flags & Save_Current_Frame) {
		// We must wait for the gpu to finish before we can read image data
		vkWaitForFences(graphics.device, 1, &fence, VK_TRUE, UINT64_MAX);
		write_frame();
	}

//=====================================================================================

	delta_time = fs::seconds_elasped_and_reset(last_timestamp);
	frame_count += 1;

	return flags & Running;
}

// TODO: error handling
void Engine::resize()
{
    auto& g = graphics;
    vkDeviceWaitIdle(g.device);

    // Destroy
    vkDestroySwapchainKHR(g.device, g.swap_chain, nullptr);
    for_n (g.sc_image_count) vkDestroyImageView(g.device, g.sc_image_views[i], nullptr);
    for_n (g.sc_image_count) vkDestroyFramebuffer(g.device, frame_buffers[i], nullptr);
	u32 old_image_count = g.sc_image_count;

    // Create
    g.create_swap_chain(&window);
    g.create_sc_image_views();

	create_frame_buffers(old_image_count);

	// Update screen transform
	using namespace glm;
	auto size = graphics.size();
	Transform_2D_Data transform {
		.transform = mat4(graphics.pre_rotation()) * (mat4x4 {
			{ 2.0f / (float)size.x, 0.0f, 0.0f, 0.0f },
			{ 0.0f, 2.0f / (float)size.y, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 1.0f, 0.0f },
			{ -1.0f, -1.0f, 0.0f, 1.0f },
		})
	};

	graphics.upload(transform_2d.buffer, &transform, sizeof(transform));

	current_scene->on_resize(old_image_count);
}

#define ADD_COMMAND(Name, Body) console::register_command(#Name, [](string args) Body)

void add_engine_console_commands()
{
	ADD_COMMAND(exit, { (void)args; stop(); });

	ADD_COMMAND(vsync, {
		if (args == "on") {
			engine.graphics.sc_present_mode = VK_PRESENT_MODE_FIFO_KHR;
			engine.flags |= Engine::Graphics_Recreate_Swap_Chain;
			console::println("vsync enabled", colors::green);
		}
		else if (args == "off") {
			engine.graphics.sc_present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
			engine.flags |= Engine::Graphics_Recreate_Swap_Chain;
			console::println("vsync disabled", colors::red);
		}
	});

#if 0 // TODO: get fps limiter working (Linux + Mac)
	ADD_COMMAND(fps_limit, {
		if (args == "on") {
			fps_last = timestamp();
			engine.flags |= Engine::FPS_Limiter_Enable;
			console::println("fps limiter enabled");
		}
		else if (args == "off") {
			engine.flags &=~ Engine::FPS_Limiter_Enable;
			console::println("fps limiter disabled");
		}
	});

	ADD_COMMAND(fps, {
		args.data[args.count] = 0;
		float fps = strtof((char*)args.data, nullptr);
		if (fps != 0.0f) {
			engine.fps_limit = fps;
			char buffer[32];
			console::println(format("set fps to: {:.1f}", fps));
		}
	});
#endif
}

// "Certified Chat-Gippty Classic" // BGRA -> RGB
void* convert_to_rgb(void* data, int pixel_count) {
	// Assuming input data is an array of BGRA values (4 bytes per pixel)
	unsigned char* input_data = static_cast<unsigned char*>(data);

	// Allocate memory for the output RGB data (3 bytes per pixel)
	unsigned char* output_data = new unsigned char[pixel_count * 3];

	for (int i = 0, j = 0; i < pixel_count * 4; i += 4, j += 3) {
		// Copy RGB values (skipping the alpha channel)
		output_data[j]     = input_data[i + 2]; // Blue
		output_data[j + 1] = input_data[i + 1]; // Green
		output_data[j + 2] = input_data[i];     // Red
	}

	return static_cast<void*>(output_data);
}

void Engine::save_frame(Render_Context& ctx)
{
	auto cmd = ctx.command_buffer;
	auto image = graphics.sc_images[ctx.image_index];
	auto range = vk::color_image_range();

	vk::image_barrier (
		cmd, image,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,      VK_ACCESS_NONE,               VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
		range
	);

	VkBufferImageCopy copy {
		.imageSubresource = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .layerCount = 1, },
		.imageExtent = { .width = graphics.sc_extent.width, .height = graphics.sc_extent.height, .depth = 1 },
	};
	vkCmdCopyImageToBuffer(cmd, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, screenshot_buffer, 1, &copy);

	vk::image_barrier (
		cmd, image,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,      VK_ACCESS_NONE,               VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		range
	);
}

void Engine::write_frame()
{
	void* gpu_memory = nullptr;
	vmaMapMemory(graphics.allocator, screenshot_allocation, &gpu_memory);
	void* data = convert_to_rgb(gpu_memory, graphics.sc_extent.width * graphics.sc_extent.height);
	vmaUnmapMemory(graphics.allocator, screenshot_allocation);

	time_t rawtime;
	time(&rawtime);
	struct tm timeinfo;
	localtime_s(&timeinfo, &rawtime);

	char filename[48];
	snprintf(filename, sizeof(filename),
		"screenshot_%04d-%02d-%02d_%02d-%02d-%02d.png",
		timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
		timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec
	);

	fs::log::info(format("Screenshot saved to \"{}\"", filename));

	stbi_write_png(filename, graphics.sc_extent.width, graphics.sc_extent.height, 3, data, 3 * graphics.sc_extent.width);
	delete[] reinterpret_cast<byte*>(data);
	flags &= ~Save_Current_Frame;
}
