#include <Fission/Platform.hpp>
#include <Fission/Core/Engine.hh>
#include <Fission/Core/Window.hh>
#include <Fission/Core/Graphics.hh>
#include <Fission/Core/Renderer_2D.hh>
#include <Fission/Base/Time.hpp>
#include <Fission/Base/Math/Matrix.hpp>
#include <Fission/Core/Scene.hh>
#include <Fission/Core/Input/Keys.hh>
#include <Fission/Core/Font.hh>
#include <Fission/Core/Console.hh>
#include "stdio.h"
#include <random>
#include <freetype/freetype.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define CELL_SIZE   24
#define GAME_WIDTH  10
#define GAME_HEIGHT 30

struct vert {
#include "../shaders/test.vert.inl"
};
struct frag {
#include "../shaders/test.frag.inl"
};

extern struct Tetris* tetris_init(int w, int h);
extern void tetris_update(Tetris* t, float dt, std::vector<fs::Event> const& events, fs::Renderer_2D& r);
extern void tetris_uninit(Tetris* t);

#define USE_MSAA 1

extern fs::Engine engine;

#define random() ((float)rand()/(float)RAND_MAX)

fs::v2f32 bounding_box(fs::Font* font, fs::string s) {
	fs::Glyph const* glyph = nullptr;
	fs::v2f32 pos = {0, font->height};
	float max_width = 0.0f;

	FS_FOR(s.count) {
		auto c = s.data[i];
		glyph = font->lookup(c);
		if (c == '\n') {
			max_width = std::max(max_width, pos.x);
			pos.x = 0;
			pos.y += font->height;
			continue;
		}
		if (glyph) {
			pos.x += glyph->advance;
		}
	}

	return {std::max(max_width, pos.x), pos.y};
}

namespace ui {
	using id = int;

	struct Global_State {
		id hot = 0;
		id active = 0;

		bool mouse_went_up   = false;
		bool mouse_went_down = false;
		fs::v2f32 mouse;
	};
	inline Global_State state;

	void set_hot(id _id) {
		if (state.active == 0) {
			state.hot = _id;
		}
	}

	bool Button(id _id, const char* text, fs::rf32 rect) {
		bool result = false;
		bool inside = rect[state.mouse];

		if (_id == state.active) {
			if (state.mouse_went_up) {
				if (_id == state.hot && inside)
					result = true;
				state.active = 0;
			}
		}
		else if (_id == state.hot) {
			if (state.mouse_went_down && inside) {
				state.active = _id;
			}
		}

		if (inside) {
			set_hot(_id);
		}

		static constexpr fs::color active_color = fs::colors::OrangeRed;
		static constexpr fs::color hot_color    = fs::colors::LightBlue;
		static constexpr fs::color def_color    = fs::colors::DimGray;

		fs::color c = def_color;
		if (_id == state.active) {
			c = active_color;
		}
		else if (inside) {
			c = hot_color;
		}

		c.a = 0.75f;
		engine.renderer_2d.add_rect(rect, c);
		auto s = FS_str_make(text, strlen(text));
		auto size = bounding_box(engine.textured_renderer_2d.current_font, s);
		engine.textured_renderer_2d.add_string(s, (rect.size() - size) * 0.5f + rect.topLeft(), fs::colors::White);

		return result;
	}
}
#if 0
struct Particle {
	fs::v2f32 position;
	float size;
	float rotation;
	float rv;
};

struct Particle_System {
	std::mt19937_64 rng;
	std::uniform_real_distribution<float> d{0.0f, 1.0f};
	Particle pool[264];

	Particle_System() {
		for (auto&& p: pool) {
			p.position = { d(rng)*1280.0f, d(rng)*720.0f };
		//	p.weight = d(rng) + 1.0f;
			p.size = d(rng)*10.0f;
			p.rotation = d(rng)*fs::math::tau;
			p.rv = d(rng) *5.0f - 2.5f;
		}
	}

	static void add_particle(fs::Renderer_2D& r, fs::v2f32 pos, float size, float rotation, fs::color col) {
		r.index_data[r.d.total_idx_count++] = r.d.vtx_count + 0;
		r.index_data[r.d.total_idx_count++] = r.d.vtx_count + 1;
		r.index_data[r.d.total_idx_count++] = r.d.vtx_count + 2;

		r.index_data[r.d.total_idx_count++] = r.d.vtx_count + 0;
		r.index_data[r.d.total_idx_count++] = r.d.vtx_count + 2;
		r.index_data[r.d.total_idx_count++] = r.d.vtx_count + 3;

		r.index_data[r.d.total_idx_count++] = r.d.vtx_count + 0;
		r.index_data[r.d.total_idx_count++] = r.d.vtx_count + 3;
		r.index_data[r.d.total_idx_count++] = r.d.vtx_count + 4;

		r.index_data[r.d.total_idx_count++] = r.d.vtx_count + 0;
		r.index_data[r.d.total_idx_count++] = r.d.vtx_count + 4;
		r.index_data[r.d.total_idx_count++] = r.d.vtx_count + 1;

		auto const rot = fs::m22::Rotation(rotation);

		r.vertex_data[r.d.total_vtx_count++] = {pos, col};
		r.vertex_data[r.d.total_vtx_count++] = {pos + rot*fs::v2f32(-size, 0), fs::color(col.r,col.g,col.b,0)};
		r.vertex_data[r.d.total_vtx_count++] = {pos + rot*fs::v2f32( 0,-size), fs::color(col.r,col.g,col.b,0)};
		r.vertex_data[r.d.total_vtx_count++] = {pos + rot*fs::v2f32( size, 0), fs::color(col.r,col.g,col.b,0)};
		r.vertex_data[r.d.total_vtx_count++] = {pos + rot*fs::v2f32( 0, size), fs::color(col.r,col.g,col.b,0)};

		r.d.idx_count += 12;
		r.d.vtx_count += 5;
	}

	void update(double dt, fs::Renderer_2D& r) {
		auto velocity_field = [](fs::v2f32 p) {
			return fs::v2f32( -1.0f, 10.0f );
		};

		using namespace fs;
		auto screen_size = v2f32{(float)engine.graphics.sc_extent.width, (float)engine.graphics.sc_extent.height};
		
		auto rd_p = [&](Particle& p) {
			p.position = {d(rng) * screen_size.x, d(rng) * -3.0f - 10.0f};
			p.size = d(rng) * 10.0f;
		};

		float _dt = (float)dt;

	//	auto rc = rf32::from_topleft(1280.0f, 720.0f).expand(5.0f);

		for (auto&& p: pool) {
			p.position += _dt * velocity_field(p.position) * (p.size);
			p.rotation += _dt * p.rv;

			if (p.position.x < -5.0f || p.position.y > (screen_size.y+10.0f)) {
				rd_p(p);
			}

			add_particle(r, p.position, p.size, p.rotation, color(colors::CornflowerBlue, std::clamp(p.size/10.0f, 0.0f, 1.0f)));
			p.size -= _dt * 0.3f;

			if (p.size <= 0.0f) {
				rd_p(p);
			}
		}
	}
};
#endif
struct Image {
	VkImage         image;
	VmaAllocation   allocation;
	VkImageView     view;
	VkSampler       sampler;
	VkDescriptorSet set;
	unsigned int width, height;

	void create(const char * filename) {
		int comp;
		int w, h;
		auto pixels = stbi_load(filename, &w, &h, &comp, 0);
		width  = (unsigned)w;
		height = (unsigned)h;

		VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
		if (comp == 3) {
			format = VK_FORMAT_R8G8B8_UNORM;
		}

		auto& gfx = engine.graphics;

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
		VkImageCreateInfo imageInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
		imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		imageInfo.arrayLayers = 1;
		imageInfo.extent = {.width = width, .height = height, .depth = 1};
		imageInfo.format = format;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.mipLevels = 1;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		vmaCreateImage(gfx.allocator, &imageInfo, &allocInfo, &image, &allocation, nullptr);

		auto imageViewInfo = vk::image_view_2d(image, format);
		vkCreateImageView(gfx.device, &imageViewInfo, nullptr, &view);

		gfx.upload_image(image, pixels, {.width = width, .height = height, .depth = 1});

		stbi_image_free(pixels);

		auto samplerInfo = vk::sampler(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
		vkCreateSampler(gfx.device, &samplerInfo, nullptr, &sampler);

		VkDescriptorSetLayout layouts[1] = { engine.texture_layout };
		VkDescriptorSetAllocateInfo descSetAllocInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
		descSetAllocInfo.descriptorPool = engine.descriptor_pool;
		descSetAllocInfo.pSetLayouts = layouts;
		descSetAllocInfo.descriptorSetCount = 1;
		vkAllocateDescriptorSets(gfx.device, &descSetAllocInfo, &set);

		{
			VkDescriptorImageInfo imageInfo;
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = view;
			imageInfo.sampler = sampler;
			VkWriteDescriptorSet write{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
			write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write.descriptorCount = 1;
			write.dstBinding = 0;
			write.dstSet = set;
			write.pImageInfo = &imageInfo;
			vkUpdateDescriptorSets(engine.graphics.device, 1, &write, 0, nullptr);
		}
	}

	void destroy() {
		vmaDestroyImage(engine.graphics.allocator, image, allocation);
		vkDestroyImageView(engine.graphics.device, view, nullptr);
		vkDestroySampler(engine.graphics.device, sampler, nullptr);
	}
};

struct Menu_UI {
	struct vertex {
		fs::v2f32 position;
		fs::v2f32 texcoord;
	};

	struct PushConstants {
		float time;
		float min;
		float max;
	};

	void create(VkRenderPass render_pass, VkSampleCountFlagBits samples) {
		VkPushConstantRange pushRange;
		pushRange.offset = 0;
		pushRange.size = sizeof(PushConstants);
		pushRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		VkPipelineLayoutCreateInfo layoutInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
		layoutInfo.pSetLayouts = &engine.transform_2d.layout;
		layoutInfo.setLayoutCount = 1;
		layoutInfo.pPushConstantRanges = &pushRange;
		layoutInfo.pushConstantRangeCount = 1;
		vkCreatePipelineLayout(engine.graphics.device, &layoutInfo, nullptr, &pipeline_layout);

		fs::Pipeline_Create_Info pi;
		pi.device = engine.graphics.device;
		pi.render_pass = render_pass;
		pi.pipeline_layout = pipeline_layout;
		pi.vertex_shader   = fs::create_shader(engine.graphics.device, vert::size, vert::data);
		pi.fragment_shader = fs::create_shader(engine.graphics.device, frag::size, frag::data);
		vk::Basic_Vertex_Input<fs::v2f32, fs::v2f32> vi{};
		pi.vertex_input = &vi;
		pi.blend_mode = fs::Blend_Mode_Normal;
		pi.samples = samples;
		pi.sampleRateShading = 0.0f;
		fs::create_pipeline(pi, &pipeline);

		vkDestroyShaderModule(engine.graphics.device, pi.vertex_shader, nullptr);
		vkDestroyShaderModule(engine.graphics.device, pi.fragment_shader, nullptr);

		VmaAllocationCreateInfo allocInfo{};
		allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
		
		VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
		bufferInfo.size = 100 * sizeof(vertex);
		bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		vmaCreateBuffer(engine.graphics.allocator, &bufferInfo, &allocInfo, &vertex_buffer, &vertex_allocation, nullptr);

		static constexpr fs::rf32 rect = {100, 260, 50, 80};

		static constexpr vertex v[] = {
			{rect.topLeft() , {0, 0}},
			{rect.topRight(), {1, 0}},
			{rect.botRight(), {1, 1}},
			{rect.botRight(), {1, 1}},
			{rect.botLeft() , {0, 1}},
			{rect.topLeft() , {0, 0}},
		};

		engine.graphics.upload_buffer(vertex_buffer, v, sizeof(v));
	}
	void destroy() {
		vmaDestroyBuffer(engine.graphics.allocator, vertex_buffer, vertex_allocation);
		vkDestroyPipeline(engine.graphics.device, pipeline, nullptr);
		vkDestroyPipelineLayout(engine.graphics.device, pipeline_layout, nullptr);
	}

	void update(double dt, fs::Render_Context& ctx) {
		time += dt;

		vkCmdBindPipeline(ctx.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(ctx.command_buffer, 0, 1, &vertex_buffer, &offset);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(ctx.gfx->sc_extent.width);
		viewport.height = static_cast<float>(ctx.gfx->sc_extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(ctx.command_buffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent = ctx.gfx->sc_extent;
		vkCmdSetScissor(ctx.command_buffer, 0, 1, &scissor);

		PushConstants constants;
		constants.time = (float)time;
		constants.min = min;
		constants.max = max;
		vkCmdPushConstants(ctx.command_buffer, pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(constants), &constants);

		vkCmdBindDescriptorSets(ctx.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout,
			0, 1, &engine.transform_2d.set, 0, nullptr);

		vkCmdDraw(ctx.command_buffer, 6, 1, 0, 0);
	}

	double time = 0.0f;
	float min = 0.0f, max = 0.0f;
	VkPipeline pipeline;
	VkPipelineLayout pipeline_layout;
	VkBuffer vertex_buffer;
	VmaAllocation vertex_allocation;
};

struct Main_Menu_Scene : public fs::Scene {
	void on_update(double dt, std::vector<fs::Event> const& events, fs::Render_Context* ctx) override {
		static bool vsync = true;
		ui::state.mouse = (fs::v2f32)engine.window.mouse_position;

		// reset
		ui::state.mouse_went_down = false;
		ui::state.mouse_went_up = false;

		for (auto const& event : events) {
			switch (event.type)
			{
			default:
			break; case fs::Event_Key_Down:
				if (event.key_down.key_id == fs::keys::Delete) {
					engine.next_scene_key = {};
					engine.flags |= engine.fScene_Change;
				}
				if (event.key_down.key_id == fs::keys::Mouse_Left) {
					ui::state.mouse_went_down = true;
				}
			//	if (event.key_down.key_id == fs::keys::A) {
			//		engine.swap_chain_info.present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
			//		engine.flags |= engine.fGraphics_Recreate_Swap_Chain;
			//		vsync = false;
			//	}
			//	if (event.key_down.key_id == fs::keys::B) {
			//		engine.swap_chain_info.present_mode = VK_PRESENT_MODE_FIFO_KHR;
			//		engine.flags |= engine.fGraphics_Recreate_Swap_Chain;
			//		vsync = true;
			//	}
				if (event.key_down.key_id == fs::keys::F11) {
					engine.window.set_mode((fs::Window_Mode)!(bool)engine.window.mode);
					engine.flags |= engine.fGraphics_Recreate_Swap_Chain;
				}
			break; case fs::Event_Key_Up:
				if (event.key_up.key_id == fs::keys::Mouse_Left) {
					ui::state.mouse_went_up   = true;
				}
			break;
			}
		}
		
#define Exp_Update(CURRENT, TARGET, LERP_SPEED) fs::lerp(CURRENT, TARGET, 1.0f - std::powf(0.5f, (float)dt * LERP_SPEED))

		static constexpr fs::rf32 rect = {100, 260, 50, 80};
		static float expand = 0.0f;
		static float value = 0.0f;

		float value_target = 0.0f;
		if (rect[ui::state.mouse]) {
			value_target = 1.0f;
		}
		value = Exp_Update(value, value_target, 16.0f);
		ui.min = fs::lerp(0.04f, 0.2f, value);
		ui.max = fs::lerp(0.2f , 0.9f, value);
		expand = fs::lerp(0.0f, 5.0f, value);

		engine.debug_layer.add("min = %.2f, max = %.2f", ui.min, ui.max);
		engine.debug_layer.add("position = (%i, %i)", engine.window.position.x, engine.window.position.y);

		using namespace fs;

#if USE_MSAA
		rp.begin(ctx, frame_buffers[ctx->image_index], colors::Black);
#else
		rp.begin(ctx, colors::Black);
#endif

		VkDescriptorSet sets[] = { engine.transform_2d.set, engine.fonts.debug.texture };
		VK_GFX_BIND_DESCRIPTOR_SETS(ctx->command_buffer, engine.textured_renderer_2d.pipeline_layout, (uint32_t)std::size(sets), sets);
		engine.textured_renderer_2d.set_font(&engine.fonts.debug);

		tetris_update(tetris, (float)dt, events, engine.renderer_2d);
		engine.renderer_2d.draw_pipeline(pipeline, *ctx);

		ui.update(dt, *ctx);

		engine.renderer_2d.add_rect_outline(rect.expanded(expand), color(colors::White,1.0f-value));
		auto off = (rect.size() - bounding_box(&engine.fonts.debug, FS_str("select"))) * 0.5f;
		engine.textured_renderer_2d.add_string(FS_str("select"), rect.topLeft() + off, colors::gray(1.0f-value));

		VK_GFX_BIND_DESCRIPTOR_SETS(ctx->command_buffer, engine.textured_renderer_2d.pipeline_layout, (uint32_t)std::size(sets), sets);
		engine.renderer_2d.draw_pipeline(pipeline, *ctx); 
		engine.textured_renderer_2d.draw_pipeline(tpipeline, *ctx); 
	
		rp.end(ctx);
	}
	virtual void on_resize() override {
#if USE_MSAA
		auto& gfx = engine.graphics;

		vmaDestroyImage(engine.graphics.allocator, colorImage, colorAllocation);
		vkDestroyImageView(engine.graphics.device, colorImageView, nullptr);

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
		VkImageCreateInfo imageInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
		imageInfo.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		imageInfo.arrayLayers = 1;
		imageInfo.extent = {.width = gfx.sc_extent.width, .height = gfx.sc_extent.height, .depth = 1};
		imageInfo.format = gfx.sc_format;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.mipLevels = 1;
		imageInfo.samples = VK_SAMPLE_COUNT_8_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		vmaCreateImage(gfx.allocator, &imageInfo, &allocInfo, &colorImage, &colorAllocation, nullptr);

		auto imageViewInfo = vk::image_view_2d(colorImage, gfx.sc_format);
		vkCreateImageView(gfx.device, &imageViewInfo, nullptr, &colorImageView);

		FS_FOR(engine.graphics.sc_image_count) {
			vkDestroyFramebuffer(engine.graphics.device, frame_buffers[i], nullptr);
		}
		VkImageView attachments[] = {colorImageView, VK_NULL_HANDLE};
		VkFramebufferCreateInfo framebufferInfo{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
		framebufferInfo.renderPass = rp;
		framebufferInfo.attachmentCount = 2;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = gfx.sc_extent.width;
		framebufferInfo.height = gfx.sc_extent.height;
		framebufferInfo.layers = 1;

		FS_FOR(engine.graphics.sc_image_count) {
			attachments[1] = gfx.sc_image_views[i];
			vkCreateFramebuffer(gfx.device, &framebufferInfo, nullptr, frame_buffers + i);
		}
#endif
	}

	Main_Menu_Scene() {
		auto& gfx = engine.graphics;
#if USE_MSAA
		auto samples = VK_SAMPLE_COUNT_8_BIT;
#else
		auto samples = VK_SAMPLE_COUNT_1_BIT;
#endif
		tetris = tetris_init(GAME_WIDTH, GAME_HEIGHT);

		rp.create(samples, true);
		ui.create(rp, samples);

#if USE_MSAA
		{
			VmaAllocationCreateInfo allocInfo = {};
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
			VkImageCreateInfo imageInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
			imageInfo.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			imageInfo.arrayLayers = 1;
			imageInfo.extent = {.width = gfx.sc_extent.width, .height = gfx.sc_extent.height, .depth = 1};
			imageInfo.format = gfx.sc_format;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.mipLevels = 1;
			imageInfo.samples = samples;
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			vmaCreateImage(gfx.allocator, &imageInfo, &allocInfo, &colorImage, &colorAllocation, nullptr);

			auto imageViewInfo = vk::image_view_2d(colorImage, gfx.sc_format);
			vkCreateImageView(gfx.device, &imageViewInfo, nullptr, &colorImageView);
		}
		{
			VkImageView attachments[] = {colorImageView, VK_NULL_HANDLE};
			VkFramebufferCreateInfo framebufferInfo{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
			framebufferInfo.renderPass = rp;
			framebufferInfo.attachmentCount = 2;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width  = gfx.sc_extent.width;
			framebufferInfo.height = gfx.sc_extent.height;
			framebufferInfo.layers = 1;

			FS_FOR(engine.graphics.sc_image_count) {
				attachments[1] = gfx.sc_image_views[i];
				vkCreateFramebuffer(gfx.device, &framebufferInfo, nullptr, frame_buffers + i);
			}
		}
#endif

		fs::Pipeline_Create_Info pi;
		pi.device = engine.graphics.device;
		pi.render_pass = rp;
		pi.pipeline_layout = engine.renderer_2d.pipeline_layout;
		pi.vertex_shader = engine.renderer_2d.vert;
		pi.fragment_shader = engine.renderer_2d.frag;
		vk::Basic_Vertex_Input<fs::v2f32, fs::color> vi{};
		pi.vertex_input = &vi;
		pi.blend_mode = fs::Blend_Mode_Normal;
		pi.samples = samples;
		fs::create_pipeline(pi, &pipeline);

		pi.pipeline_layout = engine.textured_renderer_2d.pipeline_layout;
		pi.vertex_shader   = engine.textured_renderer_2d.vert;
		pi.fragment_shader = engine.textured_renderer_2d.frag;
		vk::Basic_Vertex_Input<fs::v2f32, fs::v2f32, fs::color> tvi{};
		pi.vertex_input = &tvi;
		pi.blend_mode = fs::Blend_Mode_Normal;
		pi.samples = samples;
		pi.sampleRateShading = 0.0f;
		fs::create_pipeline(pi, &tpipeline);

		fs::console::register_command(FS_str("exit"), [](fs::string) { engine.running = false; });

		fs::string buffer_view;
		char buffer[128];
		buffer_view = FS_str_make(buffer, 0);

		for (auto&& d : engine.displays) {
			buffer_view.count = sprintf_s(buffer, "Monitor index: %i", d.index);
			fs::console::println(buffer_view);
			fs::console::println(d.name());
		}
	}
	~Main_Menu_Scene() override {
		rp.destroy();
		vkDestroyPipeline(engine.graphics.device, pipeline, nullptr);
		vkDestroyPipeline(engine.graphics.device, tpipeline, nullptr);
#if USE_MSAA
		vmaDestroyImage(engine.graphics.allocator, colorImage, colorAllocation);
		vkDestroyImageView(engine.graphics.device, colorImageView, nullptr);
		FS_FOR(engine.graphics.sc_image_count) {
			vkDestroyFramebuffer(engine.graphics.device, frame_buffers[i], nullptr);
		}
#endif
		ui.destroy();
		tetris_uninit(tetris);
	}

#if USE_MSAA
	VkImage       colorImage;
	VmaAllocation colorAllocation;
	VkImageView   colorImageView;
	VkFramebuffer frame_buffers[fs::Graphics::max_sc_images];
#endif

	fs::Render_Pass rp;
	VkPipeline pipeline;
	VkPipeline tpipeline;
	Tetris* tetris;

	Menu_UI ui;
};

fs::Scene* on_create_scene(fs::Scene_Key const& key) {
	// if(fs::equal(key.id, "Level_Editor")) {
	// 	return new Level_Editor(level_path);
	// }
//	fs::disable_layer(engine.debug_layer);

	return new Main_Menu_Scene;
}

fs::Defaults on_create() {

	engine.app_version = {0, 1, 7};
#ifdef DEBUG
	engine.app_version_info = FS_str("vanilla/dev");
#else
	engine.app_version_info = FS_str("vanilla");
#endif
	engine.app_name = FS_str("Sandbox");
	return {
		.window_title = FS_str("Sandbox"),
		.window_width  = 600 + GAME_WIDTH * CELL_SIZE,
		.window_height = GAME_HEIGHT * CELL_SIZE + 20,
	//	.window_mode  = fs::Windowed_Fullscreen,
	};
}
