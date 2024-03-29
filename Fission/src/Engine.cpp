#include "Version.h"
#include "Platform/Common.h"
#include <Fission/Core/Engine.hh>
#include <Fission/Core/Console.hh>
#include <Fission/Base/Color.hpp>
#include <Fission/Base/Time.hpp>
#include <Fission/Base/Memory.hpp>
#include <filesystem>
#include <freetype/freetype.h>
// @TODO: maybe put in separate file?
#define STB_IMAGE_WRITE_IMPLEMENTATION 1
#include <stb_image_write.h>

struct Debug_Font {
#include <BinaryFonts/IBMPlexMono-Medium.inl>
};
struct Console_Font {
#include <BinaryFonts/JetBrainsMono-Regular.inl>
};

extern fs::Engine engine;

void display_fatal_error(const char* title, const char* what);
void display_fatal_graphics_error(VkResult result, const char* what);

inline VkBuffer      screenshot_buffer{};
inline VmaAllocation screenshot_allocation{};
#include <iostream>

// "Certified Chat-Gippty Classic"
void* convert_to_rgb(void* data, int pixel_count) {
	// Assuming input data is an array of BGRA values (4 bytes per pixel)
	unsigned char* input_data = static_cast<unsigned char*>(data);

	// Allocate memory for the output RGB data (3 bytes per pixel)
	unsigned char* output_data = new unsigned char[pixel_count * 3];

	for (int i = 0, j = 0; i < pixel_count * 4; i += 4, j += 3) {
		// Copy RGB values (skipping the alpha channel)
		output_data[j] = input_data[i + 2];  // Blue
		output_data[j + 1] = input_data[i + 1];  // Green
		output_data[j + 2] = input_data[i];      // Red
	}

	return static_cast<void*>(output_data);
}

__FISSION_BEGIN__

Scene_Key cmdline_to_scene_key(platform::Instance const&);
void enumerate_displays(std::vector<struct Display>& out);
void add_engine_console_commands();

string Engine::get_version_string() {
	return FS_str(FISSION_VERSION_STRV);
}

void* talloc(u64 size) {
	auto ptr = (byte*)engine._ts_base + engine._ts_allocated;
	engine._ts_allocated += u32(size);
#if defined(FISSION_DEBUG)
	if (ptr > ((byte*)engine._ts_base + engine._ts_size)) {
		display_fatal_error("Error", "Allocated past end of temparary storage!");
		return nullptr;
	}
#endif
	return ptr;
}

#if FS_INCLUDE_EASTER_EGGS
#include "/dev/easter_eggs.hpp"
#endif

int Engine::create(platform::Instance const& instance, Defaults const& defaults) {
	flags |= fRunning;

	struct _defer {
		~_defer() { engine.flags |= fWindow_Destroy_Enable; }
	} defer;

	_ts_size = FS_KILOBYTES(64);
	_ts_base = FISSION_DEFAULT_ALLOC(_ts_size);
	// TODO: might want to check every malloc in case we run out of memory,
	//       but that is such a damn edge case, not worth thinking about for now.

	// setup the console early so we can use it as soon as possible
	console_layer.setup_console_api();

#if FS_INCLUDE_EASTER_EGGS
#include "/dev/easter_eggs_setup.inl"
#endif

	add_engine_console_commands();

	enumerate_displays(displays);

	{
		Window_Create_Info info;
		info.width         = defaults.window_width;
		info.height        = defaults.window_height;
		info.title         = defaults.window_title;
		info.mode          = defaults.window_mode;
		info.display_index = defaults.display_index;
		window.create(&info);
		if (!window.exists()) return 1;
	}

	{
		Graphics_Create_Info info;
		info.window = &window;
		if (graphics.create(&info)) return 1;
	}

	{
		VmaAllocationCreateInfo allocInfo{
			.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
			.usage = VMA_MEMORY_USAGE_AUTO,
		};
		VkBufferCreateInfo bufferInfo{
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = 1920 * 1920 * sizeof(fs::rgba8),
			.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		};
		vmaCreateBuffer(graphics.allocator, &bufferInfo, &allocInfo, &screenshot_buffer, &screenshot_allocation, nullptr);
	}

	if (create_layers()) {
		destroy();
		return 1;
	}

	{
		next_scene_key = cmdline_to_scene_key(instance);
		current_scene = on_create_scene(next_scene_key);

		if (current_scene == nullptr) {
			display_fatal_error("Undefined Scene ID", "Provided Scene ID does not exist.");
			destroy();
			return 1;
		}
	}

	return 0;
}

// TODO: need error handling here pls
int Engine::create_layers() {
	overlay_render_pass.create(VK_SAMPLE_COUNT_1_BIT, false);
	texture_layout.create(graphics);
	transform_2d.layout.create(graphics);

	{
		VkFramebufferCreateInfo framebufferInfo{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
		framebufferInfo.renderPass = overlay_render_pass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.width  = graphics.sc_extent.width;
		framebufferInfo.height = graphics.sc_extent.height;
		framebufferInfo.layers = 1;

		FS_FOR(graphics.sc_image_count) {
			framebufferInfo.pAttachments = graphics.sc_image_views + i;
			vkCreateFramebuffer(graphics.device, &framebufferInfo, nullptr, framebuffers + i);
		}
	}

	// 0 = transform_2d, 1 = debug font, 2 = console font
	VkDescriptorSet sets[3] = {};
	{
		VkDescriptorPoolSize pool_sizes[] = {
			{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         32},
			{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 64},
		};
		VkDescriptorPoolCreateInfo descPoolInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
		descPoolInfo.maxSets = 32 + 64;
		descPoolInfo.poolSizeCount = (fs::u32)std::size(pool_sizes);
		descPoolInfo.pPoolSizes = pool_sizes;
		vkCreateDescriptorPool(graphics.device, &descPoolInfo, nullptr, &descriptor_pool);

		VkDescriptorSetLayout layouts[3] = { transform_2d.layout, texture_layout, texture_layout };
		VkDescriptorSetAllocateInfo descSetAllocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		descSetAllocInfo.descriptorPool = descriptor_pool;
		descSetAllocInfo.pSetLayouts = layouts;
		descSetAllocInfo.descriptorSetCount = 3;
		vkAllocateDescriptorSets(graphics.device, &descSetAllocInfo, sets);
	}

	// Create buffer for the 2d transform
	{
		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
		VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		bufferInfo.size = sizeof(fs::Transform_2D_Data);
		bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		vmaCreateBuffer(graphics.allocator, &bufferInfo, &allocInfo, &transform_2d.buffer, &transform_2d.allocation, nullptr);

		fs::Transform_2D_Data transform;
		transform.offset = { -1.0f,-1.0f };
		transform.scale = { 2.0f / (float)graphics.sc_extent.width, 2.0f / (float)graphics.sc_extent.height };
		graphics.upload_buffer(transform_2d.buffer, &transform, sizeof(fs::Transform_2D_Data));
	}

	// Create Descriptor Set for the transform
	{
		transform_2d.set = sets[0];
		VkDescriptorBufferInfo bufferInfo;
		bufferInfo.buffer = transform_2d.buffer;
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(fs::Transform_2D_Data);
		VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write.descriptorCount = 1;
		write.dstBinding = 0;
		write.dstSet = transform_2d.set;
		write.pBufferInfo = &bufferInfo;
		vkUpdateDescriptorSets(graphics.device, 1, &write, 0, nullptr);
	}
	{
		auto samplerInfo = vk::sampler(VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
		vkCreateSampler(graphics.device, &samplerInfo, nullptr, &fonts.sampler);
	}

	FT_Init_FreeType(&fonts.library);
	fonts.debug  .create(  Debug_Font::data,   Debug_Font::size, 18.0f, sets[1], fonts.sampler);
	fonts.console.create(Console_Font::data, Console_Font::size, 16.0f, sets[2], fonts.sampler);

	renderer_2d         .create(&graphics, overlay_render_pass, transform_2d.layout);
	textured_renderer_2d.create(&graphics, overlay_render_pass, transform_2d.layout, texture_layout);

	debug_layer.create();

	// Initialize console position
	console_layer.position = -(fonts.console.height + 1);

	return 0;
}

// too lazy to do some destructor nice-ty, so this will have to suffice..
int Engine::destroy() {
	if (graphics.device) {
		vkDeviceWaitIdle(graphics.device);
	}
	vmaDestroyBuffer(graphics.allocator, screenshot_buffer, screenshot_allocation);
	delete current_scene;
	debug_layer.destroy();
	console_layer.destroy();
	renderer_2d.destroy();
	textured_renderer_2d.destroy();
	vkDestroySampler(graphics.device, fonts.sampler, nullptr);
	vmaDestroyBuffer(engine.graphics.allocator, transform_2d.buffer, transform_2d.allocation);
	fonts.debug.destroy();
	fonts.console.destroy();
	vkDestroyDescriptorPool(graphics.device, descriptor_pool, nullptr);
	vkDestroyDescriptorSetLayout(engine.graphics.device, transform_2d.layout, nullptr);
	vkDestroyDescriptorSetLayout(graphics.device, texture_layout, nullptr);
	FT_Done_FreeType(fonts.library);
	FS_FOR(graphics.sc_image_count) {
		vkDestroyFramebuffer(graphics.device, framebuffers[i], nullptr);
	}
	overlay_render_pass.destroy();
	return 0;
}

#define unlikely [[unlikely]] // pretty sure this does nothing, but it's a nice thought
#define vk_check(Result, What) if (Result) { display_fatal_graphics_error(vk_result, What); return; } (void)0

// bad
inline s64 fps_last;

void Engine::run() {
	VkSemaphore write_semaphore;
	VkSemaphore read_semaphore;
	VkFence     fence;

	auto last_timestamp = fs::timestamp();
	u64 frame_index = 0;

	std::vector<fs::Event> events;
	events.reserve(64);

	Render_Context render_context{.gfx = &graphics};
	double dt = 0.0;

	fps_last = timestamp();
#if defined(FISSION_PLATFORM_WINDOWS)
	auto timer = CreateWaitableTimerExW(NULL, NULL, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
#endif

	while (flags& fRunning) {
		render_context.frame = frame_index & 1;

		write_semaphore = graphics.sc_image_write_semaphore[render_context.frame];
		read_semaphore  = graphics.sc_image_read_semaphore [render_context.frame];
		fence           = graphics.cb_fences[render_context.frame];

		//-------------------------------------------------------------------------------------
		// Check for anything to do before processing the next frame
		unlikely if (window.is_minimized()) {
			window.sleep_until_not_minimized();
		}
		unlikely if (flags& fChange_Scene) {
			auto next_scene = on_create_scene(next_scene_key);
			vkDeviceWaitIdle(graphics.device);
			delete current_scene;
			current_scene = next_scene;
			flags &=~ fChange_Scene;
		}
		unlikely if (flags& fGraphics_Recreate_Swap_Chain) {
			resize();
			flags &=~ fGraphics_Recreate_Swap_Chain;
		}
		unlikely if (flags & fFPS_Limiter_Enable) {
#if defined(FISSION_PLATFORM_WINDOWS)
			auto time_between_frames = s64(1e7f / fps_limit);
			auto next   = fps_last + time_between_frames;
			auto now = timestamp();

			LARGE_INTEGER due_time;
			due_time.QuadPart = -((next - now) / 100);
			if (due_time.QuadPart < 0 && SetWaitableTimerEx(timer, &due_time, 0, NULL, NULL, NULL, 0)) {
				WaitForSingleObject(timer, INFINITE);
			}
			fps_last = next;
#endif
		}

		//-------------------------------------------------------------------------------------

		VkResult vk_result = VK_ERROR_UNKNOWN;
		while (vk_result != VK_SUCCESS) {
			vk_result = vkAcquireNextImageKHR(graphics.device, graphics.swap_chain, UINT64_MAX, write_semaphore, VK_NULL_HANDLE, &render_context.image_index);

			if (vk_result == VK_SUBOPTIMAL_KHR) break;
			else if (vk_result == VK_ERROR_OUT_OF_DATE_KHR) {
				resize();
				continue;
			}
			else if (vk_result != VK_SUCCESS) {
				display_fatal_graphics_error(vk_result, "Failed to get swap chain image [vkAcquireNextImageKHR]");
				return; // TODO: should try to recover here
			}
		}

		vk_check(vkWaitForFences(graphics.device, 1, &fence, VK_TRUE, UINT64_MAX), "[vkWaitForFences] failed");
		vk_check(vkResetFences(graphics.device, 1, &fence), "[vkResetFences] failed");

		//-------------------------------------------------------------------------------------

		auto cpu_start = timestamp();

		render_context.frame_buffer   = framebuffers[render_context.image_index];
		render_context.command_buffer = graphics.command_buffers[render_context.frame];

		VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		vkBeginCommandBuffer(render_context.command_buffer, &beginInfo);

		//-------------------------------------------------------------------------------------
		// Eat any events handled by debug and console layers
		window.event_queue.pop_all(events);
		debug_layer  .handle_events(events);
		console_layer.handle_events(events);
		//-------------------------------------------------------------------------------------

		current_scene->on_update(dt, events, &render_context);

		//-------------------------------------------------------------------------------------
		// Render console and debug overlay
		overlay_render_pass.begin(&render_context);
		{
			bind_font(render_context.command_buffer, &fonts.console);
			console_layer.on_update(dt, &render_context);

			bind_font(render_context.command_buffer, &fonts.debug);
			debug_layer.on_update(dt, &render_context);
		}
		overlay_render_pass.end(&render_context);
		//-------------------------------------------------------------------------------------

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

		vkEndCommandBuffer(render_context.command_buffer);

		renderer_2d         .end_render(&render_context);
		textured_renderer_2d.end_render(&render_context);

		//-------------------------------------------------------------------------------------

		debug_layer.cpu_time = (float)seconds_elasped_and_reset(cpu_start);

		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &write_semaphore;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &render_context.command_buffer;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &read_semaphore;
		vk_check(vkQueueSubmit(graphics.graphics_queue, 1, &submitInfo, fence), "[vkQueueSubmit] failed");

		//-------------------------------------------------------------------------------------

		VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &read_semaphore;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &graphics.swap_chain;
		presentInfo.pImageIndices = &render_context.image_index;
		vk_result = vkQueuePresentKHR(graphics.present_queue, &presentInfo);
		if (vk_result == VK_ERROR_OUT_OF_DATE_KHR) {
			if (!(flags& fRunning)) break; // ok, we head out
			resize();
		}
		else if (vk_result != VK_SUCCESS && vk_result != VK_SUBOPTIMAL_KHR) {
			display_fatal_graphics_error(vk_result, "[vkQueuePresentKHR] failed");
			return;
		}
        if (vk_result == VK_SUBOPTIMAL_KHR) resize();

		if (flags & fSave_Currect_Frame) {
			vkWaitForFences(graphics.device, 1, &fence, VK_TRUE, UINT64_MAX);
			void* gpu_memory = nullptr;
			vmaMapMemory(graphics.allocator, screenshot_allocation, &gpu_memory);
			void* data = convert_to_rgb(gpu_memory, graphics.sc_extent.width*graphics.sc_extent.height);
			vmaUnmapMemory(graphics.allocator, screenshot_allocation);

			time_t rawtime;
			time(&rawtime);
			auto timeinfo = localtime(&rawtime);

			char filename[48];
			"screenshot_%04d-%02d-%02d_%02d-%02d-%02d.png"_fmt(filename,
				timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday,
				timeinfo->tm_hour,      timeinfo->tm_min,   timeinfo->tm_sec
			);

			stbi_write_png(filename, graphics.sc_extent.width, graphics.sc_extent.height, 3, data, 3 * graphics.sc_extent.width);
			delete [] reinterpret_cast<byte*>(data);
			flags &=~ fSave_Currect_Frame;
		}

		//-------------------------------------------------------------------------------------
		// Calculate next frame time
		dt = fs::seconds_elasped_and_reset(last_timestamp);
		frame_index += 1;
	}

#if defined(FISSION_PLATFORM_WINDOWS)
	CloseHandle(timer);
#endif
}

void Engine::resize() {
    FS_debug_print("> inside resize()\n");
	if (window.is_minimized())
		window.sleep_until_not_minimized();

    FS_debug_print("> calling recreate_swap_chain()\n");
	graphics.recreate_swap_chain(&window);

	// Update 2D transform
    FS_debug_print("> updating graphics stuff\n");
	fs::Transform_2D_Data transform;
	transform.offset = {-1.0f,-1.0f};
	transform.scale = {2.0f / (float)graphics.sc_extent.width, 2.0f / (float)graphics.sc_extent.height};
	graphics.upload_buffer(transform_2d.buffer, &transform, sizeof(fs::Transform_2D_Data));
	
	// Recreate framebuffers
	FS_FOR(graphics.sc_image_count)
		vkDestroyFramebuffer(graphics.device, framebuffers[i], nullptr);
	
	{
		VkFramebufferCreateInfo framebufferInfo{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
		framebufferInfo.renderPass = overlay_render_pass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.width  = graphics.sc_extent.width;
		framebufferInfo.height = graphics.sc_extent.height;
		framebufferInfo.layers = 1;

		FS_FOR(graphics.sc_image_count) {
			framebufferInfo.pAttachments = graphics.sc_image_views + i;
			vkCreateFramebuffer(graphics.device, &framebufferInfo, nullptr, framebuffers + i);
		}
	}

    FS_debug_print("> calling resize scene\n");
	current_scene->on_resize();
    FS_debug_print("> done resizeing!\n");
}

#define ADD_COMMAND(Name, Body) console::register_command(FS_str(#Name), [](string args) Body)

void add_engine_console_commands() {
	ADD_COMMAND(vsync, {
		if (args == "on") {
			engine.graphics.sc_present_mode = VK_PRESENT_MODE_FIFO_KHR;
			engine.flags |= engine.fGraphics_Recreate_Swap_Chain;
			console::println(FS_str("vsync enabled"), fs::colors::Green);
		}
		else if (args == "off") {
			engine.graphics.sc_present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
			engine.flags |= engine.fGraphics_Recreate_Swap_Chain;
			console::println(FS_str("vsync disabled"), fs::colors::Red);
		}
	});

	ADD_COMMAND(fps_limit, {
		if (args == "on") {
			fps_last = timestamp();
			engine.flags |= engine.fFPS_Limiter_Enable;
			console::println(FS_str("fps limiter enabled"));
		}
		else if (args == "off") {
			engine.flags &=~ engine.fFPS_Limiter_Enable;
			console::println(FS_str("fps limiter disabled"));
		}
	});

	ADD_COMMAND(fps, {
		using namespace fs;
		args.data[args.count] = 0;
		float fps = strtof((char*)args.data, nullptr);
		if (fps != 0.0f) {
			engine.fps_limit = fps;
			char buffer[32];
			console::println("set fps to: %.1f"_fmt(buffer, fps));
		}
	});
}

__FISSION_END__
