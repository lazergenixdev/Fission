#include "Version.h"
#include "Platform/Common.h"
#include <Fission/Core/Engine.hh>
#include <Fission/Core/Console.hh>
#include <Fission/Base/Color.hpp>
#include <filesystem>
#include <freetype/freetype.h>

struct Debug_Font {
#include <BinaryFonts/IBMPlexMono-Medium.inl>
};
struct Console_Font {
#include <BinaryFonts/JetBrainsMono-Regular.inl>
};

extern fs::Engine engine;

void display_fatal_error(const char* title, const char* what);
void display_fatal_graphics_error(VkResult result, const char* what);

__FISSION_BEGIN__

Scene_Key cmdline_to_scene_key(platform::Instance);
extern s64 timestamp();
extern double seconds_elasped_and_reset(s64& last);
void enumerate_displays(std::vector<struct Display>& out);
void add_engine_console_commands();

string Engine::get_version_string() {
	return FS_str(FISSION_VERSION_STRV);
}

#if FS_INCLUDE_EASTER_EGGS
#include "/dev/easter_eggs.hpp"
#endif

int Engine::create(platform::Instance const& instance, Defaults const& defaults) {
	flags |= fRunning;

	struct _defer {
		~_defer() { engine.flags |= fWindow_Destroy_Enable; }
	} defer;

//	wchar_t buffer[128];
//	auto dwRet = GetEnvironmentVariableW(L"APPDATA", buffer, std::size(buffer));
//	OutputDebugStringW(L"APPDATA = ");
//	OutputDebugStringW(buffer);
//	OutputDebugStringW(L"\n");
//	return 1;

#if WIP
	_ts_size = FS_KILOBYTES(64);
	_ts_base = _aligned_malloc(_ts_size, 32);
	assert(_ts_base != nullptr);
#endif

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
			{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         16},
			{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,112},
		//	{VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       16},
		};
		VkDescriptorPoolCreateInfo descPoolInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
		descPoolInfo.maxSets = 128;
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
	fonts.debug  .create(  Debug_Font::data,   Debug_Font::size, 20.0f, sets[1], fonts.sampler);
	fonts.console.create(Console_Font::data, Console_Font::size, 16.0f, sets[2], fonts.sampler);

	renderer_2d         .create(&graphics, overlay_render_pass, transform_2d.layout);
	textured_renderer_2d.create(&graphics, overlay_render_pass, transform_2d.layout, texture_layout);

	debug_layer.create();
	console_layer.position = -(fonts.console.height + 1); // console_layer.create();

	return 0;
}

// too lazy to do some destructor nice-ty, so this will have to suffice..
int Engine::destroy() {
	if (graphics.device) {
		vkDeviceWaitIdle(graphics.device);
	}
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

void Engine::run() {
	VkSemaphore write_semaphore;
	VkSemaphore read_semaphore;
	VkFence     fence;

	auto last_timestamp = fs::timestamp();
	u64 frame_index = 0;
	uint32_t imageIndex;

	std::vector<fs::Event> events;
	events.reserve(64);

	Render_Context render_context{.gfx = &graphics};
	double dt = 0.0;

	_next = clock::now();

	VkFence _fence;
	VkFenceCreateInfo fenceInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
	vkCreateFence(graphics.device, &fenceInfo, nullptr, &_fence);

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
		// TODO: fix [fFPS_Limiter_Enable]
		// This is a WIP, don't know why this doesn't give good results
		unlikely if (flags & fFPS_Limiter_Enable) {
			std::chrono::nanoseconds offset{(long long)(1e9f / (fps_limit*2.0f))};
			_next += offset;

			if constexpr(0) {
				std::this_thread::sleep_until(_next);
			}
			else {
				// this gives better results than `sleep_until` SMH, fix your shit microsoft
				vkWaitForFences(graphics.device, 1, &_fence, VK_TRUE, offset.count());
			}
		}

		//-------------------------------------------------------------------------------------

		VkResult vk_result = VK_ERROR_UNKNOWN;
		while (vk_result != VK_SUCCESS) {
			vk_result = vkAcquireNextImageKHR(graphics.device, graphics.swap_chain, UINT64_MAX, write_semaphore, VK_NULL_HANDLE, &imageIndex);

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

		render_context.frame_buffer   = framebuffers[imageIndex];
		render_context.command_buffer = graphics.command_buffers[render_context.frame];
		render_context.image_index    = imageIndex;

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

		vkEndCommandBuffer(render_context.command_buffer);

		renderer_2d         .end_render(&render_context);
		textured_renderer_2d.end_render(&render_context);

		//-------------------------------------------------------------------------------------

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

		debug_layer.cpu_time = (float)seconds_elasped_and_reset(cpu_start);

		//-------------------------------------------------------------------------------------

		VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &read_semaphore;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &graphics.swap_chain;
		presentInfo.pImageIndices = &imageIndex;
		vk_result = vkQueuePresentKHR(graphics.present_queue, &presentInfo);
		if (vk_result == VK_ERROR_OUT_OF_DATE_KHR) {
			if (!(flags& fRunning)) break; // ok, we head out
			resize();
		}
		else if (vk_result != VK_SUCCESS && vk_result != VK_SUBOPTIMAL_KHR) {
			display_fatal_graphics_error(vk_result, "[vkQueuePresentKHR] failed");
			return;
		}

		//-------------------------------------------------------------------------------------
		// Calculate next frame time
		dt = fs::seconds_elasped_and_reset(last_timestamp);
		frame_index += 1;
	}

	vkDestroyFence(graphics.device, _fence, nullptr);
}

void Engine::resize() {
	if (window.is_minimized())
		window.sleep_until_not_minimized();

	graphics.recreate_swap_chain(&window);

	// Update 2D transform
	fs::Transform_2D_Data transform;
	transform.offset = {-1.0f,-1.0f};
	transform.scale = {2.0f / (float)graphics.sc_extent.width, 2.0f / (float)graphics.sc_extent.height};
	graphics.upload_buffer(transform_2d.buffer, &transform, sizeof(fs::Transform_2D_Data));
	
	// Recreate framebuffers
	FS_FOR(graphics.sc_image_count) {
		vkDestroyFramebuffer(graphics.device, framebuffers[i], nullptr);
	}
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

	current_scene->on_resize();
}

#define ADD_COMMAND(Name, Body) auto proc_##Name = [](string args) Body; console::register_command(FS_str(#Name), proc_##Name)

void add_engine_console_commands() {
	ADD_COMMAND(vsync, {
		if (args == "on") {
			engine.graphics.sc_present_mode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
			engine.flags |= engine.fGraphics_Recreate_Swap_Chain;
			console::println(FS_str("vsync enabled"));
		}
		else if (args == "off") {
			engine.graphics.sc_present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
			engine.flags |= engine.fGraphics_Recreate_Swap_Chain;
			console::println(FS_str("vsync disabled"));
		}
	});

	ADD_COMMAND(fps_limit, {
		if (args == "on") {
			engine._next = Engine::clock::now();
			engine.flags |= engine.fFPS_Limiter_Enable;
			console::println(FS_str("fps limiter enabled"));
		}
		else if (args == "off") {
			engine.flags &=~ engine.fFPS_Limiter_Enable;
			console::println(FS_str("fps limiter disabled"));
		}
	});

	ADD_COMMAND(fps, {
		args.data[args.count] = 0;
		float fps = strtof((char*)args.data, nullptr);
		if (fps != 0.0f) {
			engine.fps_limit = fps;
			string out;
			FS_FORMAT_TO_STRING(64, out, "set fps to: %.1f", fps);
			console::println(out);
		}
	});
}

__FISSION_END__