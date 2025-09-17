#include "Fission/core.hpp"
#include "MaxRectsBinPack.hpp"
#include "embed/NotoSansKR-Regular.ttf.hpp"
#include <freetype/freetype.h>

namespace os {
	auto init() -> fission::Result;
	auto info() -> const Info& { return _info; }
}

BEGIN_NAMESPACE(fission)

#define check(Result, ...) if ((result = (Result)) < VK_SUCCESS) { log::error(__VA_ARGS__); return stop(); } (void)0
#define for_(itr, N) for (decltype(N) itr = 0; itr < (N); ++itr)

auto decode_utf8(Arena& arena, string s) -> array<c32>
{
	c32* start = arena.next_ptr<c32>();
	size_t i = 0;

	next: while (i < s.count) {
		c8 c = s.data[i];
		c32 codepoint = 0;
		int extra_bytes = 0;

		if (c <= 0x7F) { // 1-byte (ASCII)
			codepoint = c;
			extra_bytes = 0;
		} else if ((c >> 5) == 0x6) { // 2-byte
			codepoint = c & 0x1F;
			extra_bytes = 1;
		} else if ((c >> 4) == 0xE) { // 3-byte
			codepoint = c & 0x0F;
			extra_bytes = 2;
		} else if ((c >> 3) == 0x1E) { // 4-byte
			codepoint = c & 0x07;
			extra_bytes = 3;
		} else {
			// Error: Invalid UTF-8 start byte
			{ i++; continue; }
		}

		// Error: Truncated UTF-8 sequence
		if (i + extra_bytes >= s.count)
			break;

		for (int j = 0; j < extra_bytes; ++j) {
			c8 cc = s.data[i + j + 1];
			// Error: Invalid UTF-8 continuation byte
			if ((cc >> 6) != 0x2)
				{ i++; goto next; }
			codepoint = (codepoint << 6) | (cc & 0x3F);
		}

		i += extra_bytes + 1;

		// Encode as UTF-32
		arena.push(codepoint);
	}

	return {size_t(arena.next_ptr<c32>() - start), start};
}

auto Font::create(Create_Info const& info) -> Result
{
	auto& graphics = engine.graphics;

	VkDescriptorSetAllocateInfo set_info {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = engine.descriptor_pool,
		.descriptorSetCount = 1,
		.pSetLayouts = &engine.descriptor_set_layout,
	};
	vkAllocateDescriptorSets(graphics.device, &set_info, &set);

	FT_Face face;
	FT_Long ttf_size = (FT_Long)(embedded::NotoSansKR_Regular_ttf_end - embedded::NotoSansKR_Regular_ttf_start);
	ASSERT(!FT_New_Memory_Face(engine.freetype_library, (FT_Byte const*)embedded::NotoSansKR_Regular_ttf_start, ttf_size, 0, &face));
	ASSERT(!FT_Set_Pixel_Sizes(face, 0, (FT_UInt)info.font_size));
	
	float yMax = float(face->size->metrics.ascender >> 6);
	auto _h = u32(face->size->metrics.height >> 6);
	v2u32 size = { _h * 12, _h * 12 };
	auto pixel_data = (rgba8*)calloc(sizeof(rgba8), size.x * size.y);
	auto pack = rbp::MaxRectsBinPack(size.x, size.y, false);

	auto generate_glyph = [&]() -> Glyph {
		Glyph g;

		auto offsetx = (float)(face->glyph->bitmap_left);
		auto offsety = (float)(-face->glyph->bitmap_top) + yMax;
		auto sizex = (float)(face->glyph->metrics.width >> 6);
		auto sizey = (float)(face->glyph->metrics.height >> 6);

		g.rc = rf32::from_topleft(offsetx, offsety, sizex, sizey);
		g.advance = (float)(face->glyph->metrics.horiAdvance >> 6);

		auto bitmap = face->glyph->bitmap;

		auto rect = pack.Insert(bitmap.width+1, bitmap.rows+1, rbp::MaxRectsBinPack::RectBestAreaFit);

		/* now, copy to our target surface */
		for_(y, bitmap.rows) {
			for_(x, bitmap.width) {
				pixel_data[(rect.y + y) * size.x + (rect.x + x)]
					= rgba8(255, 255, 255, bitmap.buffer[y * bitmap.width + x]);
			}
		}

		g.uv.x = { (float)rect.x, (float)(rect.x + bitmap.width) };
		g.uv.x /= (float)size.x;
		g.uv.y = { (float)rect.y, (float)(rect.y + bitmap.rows) };
		g.uv.y /= (float)size.y;
		return g;
	};

	// Set Fallback glyph
	ASSERT(!FT_Load_Glyph(face, 0, FT_LOAD_RENDER));
	fallback = generate_glyph();

	for (auto c: info.codepoints)
	{
		ASSERT(!FT_Load_Char(face, c, FT_LOAD_RENDER));
		//! TODO: don't look for repeats
		if (glyph_map.contains(c)) continue;
		glyph_map.emplace(c, generate_glyph());
	}

	{
		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
		VkImageCreateInfo imageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.arrayLayers = 1;
		imageInfo.extent = { .width = size.x, .height = size.y, .depth = 1 };
		imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.mipLevels = 1;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		vmaCreateImage(graphics.allocator, &imageInfo, &allocInfo, &image, &image_allocation, nullptr);
		graphics.upload(image, pixel_data, imageInfo.extent, VK_FORMAT_R8G8B8A8_UNORM);
	}

	VkImageViewCreateInfo image_view_info {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = image,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = VK_FORMAT_R8G8B8A8_UNORM,
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1,
		}
	};
	vkCreateImageView(graphics.device, &image_view_info, nullptr, &image_view);

	VkDescriptorImageInfo imageInfo;
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = image_view;
	VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
	write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	write.descriptorCount = 1;
	write.dstBinding = 0;
	write.dstSet = set;
	write.pImageInfo = &imageInfo;
	vkUpdateDescriptorSets(engine.graphics.device, 1, &write, 0, nullptr);

	return Success;
}

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
	logger.minimum_level = log::Debug;
//	scoped_set(logger.minimum_level, log::Verbose);
	if (os::init()) return Failed;
    //! TODO: place log file in same directory as the executable
	logger.backing_file = os::open_file("fission.log", os::Write);
    os_mutex_create(&logger.mutex);
    log::info("Creating Fission Engine...");
	engine.temp_arena.create(16_MiB);
	engine.frame_arena.create(8_MiB);

	FT_Init_FreeType(&freetype_library);
	FT_Int x, y, z;
	FT_Library_Version(freetype_library, &x, &y, &z);
	log::info("Using FreeType version ", x, ".", y, ".", z);

	//! TODO: setup in-game console here

	Window::Create_Info window_info {
		.title = defaults.window_title,
        .width = defaults.window_width,
        .height = defaults.window_height,
    };
    if (window.create(window_info)) return Failed;
    
    Graphics::Create_Info graphics_info {
		.window = &window,
	//! TODO: need to download validation layers for Android
	#if defined(OS_ANDROID)
        .debug = false,
	#else
        .debug = true,
	#endif
    };
	if (graphics.create(graphics_info)) return Failed;

	log::verbose("Creating overlay render pass...");
    Render_Pass_Creator{}
        .add_attachment(graphics.format, Attachment_Preset_Clear_Image_Present)
        .add_subpass({ {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL} })
        .add_external_subpass_dependency(0)
        .create(&overlay_render_pass);
		
	log::verbose("Creating render pass...");
    Render_Pass_Creator{}
        .add_attachment(VK_FORMAT_R32G32B32A32_SFLOAT, Attachment_Preset_Clear_Image)
        .add_subpass({ {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL} })
        .add_external_subpass_dependency(0)
        .create(&render_pass);

	render_image.create({
		.width = graphics.extent.width,
		.height = graphics.extent.height,
		.format = VK_FORMAT_R32G32B32A32_SFLOAT,
		.render_pass = render_pass,
		.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
	});

	log::verbose("Creating Pipeline layout...");
	{
		auto make_sampler_info = [](VkFilter filter, VkSamplerAddressMode address_mode,
			VkBorderColor border_color = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE) {
			return VkSamplerCreateInfo {
				.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
				.magFilter = filter,
				.minFilter = filter,
				.addressModeU = address_mode,
				.addressModeV = address_mode,
				.addressModeW = address_mode,
				.borderColor = border_color,
			};
		};
		VkSamplerCreateInfo sampler_info = make_sampler_info(VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
		vkCreateSampler(graphics.device, &sampler_info, nullptr, &sampler);

		VkDescriptorSetLayoutBinding binding {
			.binding = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
			.pImmutableSamplers = &sampler,
		};
		VkDescriptorSetLayoutCreateInfo descriptor_set_layout_info {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.bindingCount = 1,
			.pBindings = &binding,
		};
		vkCreateDescriptorSetLayout(graphics.device, &descriptor_set_layout_info, nullptr, &descriptor_set_layout);

		VkDescriptorPoolSize pool_size {
			.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = 64,
		};
		VkDescriptorPoolCreateInfo descriptor_pool_info {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.maxSets = 64,
			.poolSizeCount = 1,
			.pPoolSizes = &pool_size,
		};
		vkCreateDescriptorPool(graphics.device, &descriptor_pool_info, nullptr, &descriptor_pool);
	
		VkPushConstantRange push {
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
			.offset = 0,
			.size = sizeof(vec4),
		};
		VkPipelineLayoutCreateInfo pipelineLayoutInfo {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = 1,
			.pSetLayouts = &descriptor_set_layout,
			.pushConstantRangeCount = 1,
			.pPushConstantRanges = &push,
		};
		vkCreatePipelineLayout(graphics.device, &pipelineLayoutInfo, nullptr, &pipeline_layout);
	}
	log::verbose("Creating framebuffers...");
	create_frame_buffers();
	log::verbose("Creating draw data...");
	draw_data.create(graphics);
	log::verbose("Creating renderer...");
	renderer.create(render_pass, pipeline_layout, &draw_data);
	log::verbose("Creating line renderer...");
	line_renderer.create(render_pass, pipeline_layout, &draw_data, {.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST});

	blur_post.create({
		.render_pass = overlay_render_pass,
		.source_attachment = render_image.image_view,
	});
	
	{
		VkDescriptorSetAllocateInfo set_info {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = engine.descriptor_pool,
			.descriptorSetCount = 1,
			.pSetLayouts = &blur_post.descriptor_set_layout,
		};
		vkAllocateDescriptorSets(graphics.device, &set_info, &render_image_set);
		
		VkDescriptorImageInfo imageInfo;
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = render_image.image_view;
		VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write.descriptorCount = 1;
		write.dstBinding = 0;
		write.dstSet = render_image_set;
		write.pImageInfo = &imageInfo;
		vkUpdateDescriptorSets(graphics.device, 1, &write, 0, nullptr);
	}

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
	FT_Done_FreeType(freetype_library);
	graphics.destroy();
}

auto Engine::setup() -> Result
{
	log::verbose("Setting up render thread...");
	last_ticks = ticks();
    return Success;
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

	BeginCommandBuffer(render_context.command_buffer);

	CmdBeginRenderPass(render_context.command_buffer, render_pass, render_image.frame_buffer, {});
	graphics.set_default_scissor(render_context.command_buffer);
	graphics.set_default_viewport(render_context.command_buffer);

	vec4 scale {-1.0f, -1.0f, 2.0f / (float)graphics.extent.width, 2.0f / (float)graphics.extent.height};
	vkCmdPushConstants(render_context.command_buffer, pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(vec4), &scale);

	f64 dt = seconds_elapsed_and_reset(last_ticks);
	array<Event> events = window.pop_all_events(frame_arena);
	on_update(dt, events, render_context);
	
#if 1
    {
		const u32 n = 256;
		local_persist f32 frame_times[n] = {};
		u32 pos = frame_count % n;
		frame_times[pos] = f32(dt);
        u32 v = draw_data.current.vertex_count;
        
		forn (n-1) {
			draw_data.push_index(v+i);
			draw_data.push_index(v+i+1);
		}

		forn (n) {
			f32 x = (f32(i)/f32(n-1))*f32(graphics.extent.width);
			f32 y = (frame_times[i] / 0.016f) * 100.0f + f32(graphics.extent.height/2);
			s32 k = (pos - i) % n;
			if (k < 0) k = -k;
        	draw_data.push_vertex({{x, y}, vec2(-1.0f), vec4(1.0f,f32(k)/255.0f,f32(k)/255.0f,1.0f)});
		}
        line_renderer.draw(render_context);
    }
#endif
	vkCmdEndRenderPass(render_context.command_buffer);
	blur_post.process(render_context, render_image.image, render_image_set);
	draw_data.send(graphics, render_context.frame);

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
