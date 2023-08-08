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

#undef assert
#define assert(R) if(!(R)) return 1

extern fs::Engine engine;

__FISSION_BEGIN__

Scene_Key cmdline_to_scene_key(platform::Instance);
extern s64 timestamp();
extern double seconds_elasped_and_reset(s64& last);

string Engine::get_version_string() {
	return FS_str(FISSION_VERSION_STRV);
}

#define FS_INCLUDE_EASTER_EGGS 1

#if FS_INCLUDE_EASTER_EGGS
#include "/dev/easter_eggs.hpp"
#endif

int Engine::create(platform::Instance const& instance, Defaults const& defaults) {
	running   = true;
	minimized = false;

	struct _defer {
		~_defer() { engine.flags |= fWindow_Destroy_Enable; }
	} defer;

//	auto a = std::filesystem::current_path();
//	next_scene_key = cmdline_to_scene_key(instance);
//	MessageBoxW(0, GetCommandLineW(), L"Command Line", MB_CANCELTRYCONTINUE);
//	return 1;

	_temp_memory_size = FS_KILOBYTES(64);
	_temp_memory_base = _aligned_malloc(_temp_memory_size, 32);
	assert(_temp_memory_base != nullptr);

	// I want my console messages!
	console_layer.create();

#if FS_INCLUDE_EASTER_EGGS
#include "/dev/easter_eggs_setup.inl"
#endif

	{
		Window_Create_Info info;
		info.width  = defaults.window_width;
		info.height = defaults.window_height;
		info.title  = defaults.window_title;
		info.mode   = defaults.window_mode;
		info.display_index = 0;
		info.engine = this;
		window.create(&info);
		assert(window.exists());
	}

	{
		Graphics_Create_Info info;
		info.window = &window;
		assert(graphics.create(&info) == false);
	}

	if (create_layers()) return 1;

	{
		current_scene = on_create_scene(next_scene_key);
		assert(current_scene != nullptr);
	}

	return 0;
}

// TODO: need error handling here pls
int Engine::create_layers() {
	overlay_render_pass.create(VK_SAMPLE_COUNT_1_BIT, false);
	texture_layout.create(graphics);
	transform_2d.layout.create(graphics);

	// 0 = transform_2d, 1 = debug font, 2 = console font
	VkDescriptorSet sets[3] = {};
	{
		VkDescriptorPoolSize pool_sizes[] = {
			{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         16},
			{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,112},
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
	overlay_render_pass.destroy();
	return 0;
}

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

	while (running) {
		render_context.frame = frame_index & 1;

		write_semaphore = graphics.sc_image_write_semaphore[render_context.frame];
		read_semaphore  = graphics.sc_image_read_semaphore [render_context.frame];
		fence           = graphics.cb_fences[render_context.frame];

		auto cpu_start = timestamp();

		if (minimized) {
		//	OutputDebugStringA("************************\nWAITING\n************************\n");
			std::unique_lock lock(_mutex);
			_event.wait(lock, []() { return !engine.minimized; });
		}
		if (flags& fScene_Change) {
			auto next_scene = on_create_scene(next_scene_key);
			vkDeviceWaitIdle(graphics.device);
			delete current_scene;
			current_scene = next_scene;
			flags &=~ fScene_Change;
		}
		vkWaitForFences(graphics.device, 1, &fence, VK_TRUE, UINT64_MAX);
		
		if (flags& fGraphics_Recreate_Swap_Chain) {
			graphics.recreate_swap_chain(&window, swap_chain_info.present_mode);
			current_scene->on_resize();
			flags &=~ fGraphics_Recreate_Swap_Chain;
		}

		VkResult vkr = VK_ERROR_UNKNOWN;
		while (vkr != VK_SUCCESS) {
			vkr = vkAcquireNextImageKHR(graphics.device, graphics.swap_chain, UINT64_MAX, write_semaphore, VK_NULL_HANDLE, &imageIndex);

			if (vkr == VK_SUBOPTIMAL_KHR) break;
			else if (vkr == VK_ERROR_OUT_OF_DATE_KHR) {
				graphics.recreate_swap_chain(&window, graphics.sc_present_mode);
				current_scene->on_resize();
				continue;
			}
			else if (vkr != VK_SUCCESS) {
				throw std::runtime_error("failed to acquire swap chain image!"); // please no exceptions
			}
		}
		vkResetFences(graphics.device, 1, &fence);

		render_context.frame_buffer   = graphics.sc_framebuffers[imageIndex];
		render_context.command_buffer = graphics.command_buffers[render_context.frame];
		render_context.image_index    = imageIndex;

		VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		vkBeginCommandBuffer(render_context.command_buffer, &beginInfo);

		window.event_queue.pop_all(events);
		debug_layer  .handle_events(events);
		console_layer.handle_events(events);
		
		current_scene->on_update(dt, events, &render_context);

		overlay_render_pass.begin(&render_context);
		VkDescriptorSet sets[] = { transform_2d.set, fonts.console.texture };
		VK_GFX_BIND_DESCRIPTOR_SETS(render_context.command_buffer, textured_renderer_2d.pipeline_layout, 2, sets);
		console_layer.on_update(dt, &render_context);
		sets[1] = fonts.debug.texture;
		VK_GFX_BIND_DESCRIPTOR_SETS(render_context.command_buffer, textured_renderer_2d.pipeline_layout, 2, sets);
		debug_layer.on_update(dt, &render_context);
		overlay_render_pass.end(&render_context);

		vkEndCommandBuffer(render_context.command_buffer);

		renderer_2d         .end_render(&render_context);
		textured_renderer_2d.end_render(&render_context);

		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &write_semaphore;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &render_context.command_buffer;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &read_semaphore;
		vkQueueSubmit(graphics.graphics_queue, 1, &submitInfo, fence);

		debug_layer.cpu_time = (float)seconds_elasped_and_reset(cpu_start);

		VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &read_semaphore;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &graphics.swap_chain;
		presentInfo.pImageIndices = &imageIndex;
		vkr = vkQueuePresentKHR(graphics.present_queue, &presentInfo);

		if (vkr == VK_ERROR_OUT_OF_DATE_KHR) {
			if (!running) return; // ok, we head out
			graphics.recreate_swap_chain(&window, graphics.sc_present_mode);
			current_scene->on_resize();
		}
		else if (vkr != VK_SUCCESS && vkr != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!"); // please no exceptions
		}

		dt = fs::seconds_elasped_and_reset(last_timestamp);
		frame_index += 1;
	}
}

void Engine::set_window_mode(struct Display* display, Window_Mode mode) {
	(void)display;
}

void skip_working_directory(LPWSTR& s) {
	if (*s != L'"') return;
	++s;
	while (*s != L'"') ++s;
	s += 2;
}

u64 wstrlen(LPWSTR s) {
	u64 size = 0;
	while (true) {
		if (s[size] == L'\0') break;
		++size;
	}
	return size;
}

string parse_next(LPWSTR cursor, LPWSTR const end, std::vector<u8>& temp) {
	string next;
	temp.clear();
	while (cursor != end) {
		if (*cursor == L'"') {

		}
		else if (*cursor == L' ') ++cursor;
		else {
			auto start = cursor;
			while (cursor != end) {
				if (*cursor == L' ') break;
			}
			auto out = FS_str_std(temp);
		//	convert_utf16_to_utf8(&out, )
		}
	}
	return next;
}

// windows only :(
Scene_Key cmdline_to_scene_key(platform::Instance) {
	Scene_Key key;
	auto lpCmdLine = GetCommandLineW();
	auto end = lpCmdLine + wstrlen(lpCmdLine);

	skip_working_directory(lpCmdLine);

	auto cursor = lpCmdLine;
	std::vector<u8> temp;
	temp.reserve(64);

	key.id = parse_next(cursor, end, temp);
	for (auto&& s : key.arguments) {
		s = parse_next(cursor, end, temp);
	}

	return key;
}

__FISSION_END__