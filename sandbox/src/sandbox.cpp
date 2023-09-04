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
//#include "stdio.h"
//#include <random>
//#include <freetype/freetype.h>
//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"

#define CELL_SIZE   24
#define GAME_WIDTH  10
#define GAME_HEIGHT 30
#define USE_MSAA 1

extern struct Tetris* tetris_init(int w, int h);
extern void tetris_update(Tetris* t, float dt, std::vector<fs::Event> const& events, fs::Renderer_2D& r);
extern void tetris_uninit(Tetris* t);

extern fs::Engine engine;

namespace blend_vs {
#include "../shaders/blend.vert.inl"
}
namespace blend_fs {
#include "../shaders/blend.frag.inl"
}

struct Tetris_Scene : public fs::Scene {
	void on_update(double dt, std::vector<fs::Event> const& events, fs::Render_Context* ctx) override {
		for (auto const& event : events) {
			switch (event.type)
			{
			default:
			break; case fs::Event_Key_Down:
				if (event.key_down.key_id == fs::keys::F11) {
					engine.window.set_mode((fs::Window_Mode)!(bool)engine.window.mode);
					engine.flags |= engine.fGraphics_Recreate_Swap_Chain;
				}
			break;
			}
		}
		
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

	Tetris_Scene() {
		auto& gfx = engine.graphics;
#if USE_MSAA
		auto samples = VK_SAMPLE_COUNT_8_BIT;
#else
		auto samples = VK_SAMPLE_COUNT_1_BIT;
#endif
		tetris = tetris_init(GAME_WIDTH, GAME_HEIGHT);

		rp.create(samples, true);

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
		fs::create_pipeline(pi, &tpipeline);

		fs::console::register_command(FS_str("exit"), [](fs::string) { engine.flags &=~ engine.fRunning; });

		fs::string buffer_view;
		char buffer[128];
		buffer_view = FS_str_make(buffer, 0);

		for (auto&& d : engine.displays) {
			buffer_view.count = sprintf_s(buffer, "Monitor index: %i", d.index);
			fs::console::println(buffer_view);
			fs::console::println(d.name());
		}
	}
	~Tetris_Scene() override {
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
};

#define THIRD_PASS 1

#ifdef THIRD_PASS
#	define PASS_COUNT 3
#else
#	define PASS_COUNT 2
#endif

#if 0
static constexpr auto     image_format = VK_FORMAT_B10G11R11_UFLOAT_PACK32;
static constexpr auto src_image_format = VK_FORMAT_A2R10G10B10_UNORM_PACK32;
#else
static constexpr auto     image_format = VK_FORMAT_R32G32B32A32_SFLOAT;
static constexpr auto src_image_format = VK_FORMAT_R32G32B32A32_SFLOAT;
#endif

struct Scene_OK : public fs::Scene {
	void create_render_pass() {
		VkAttachmentDescription srcAttachment{};
		srcAttachment.format = src_image_format;
		srcAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		srcAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		srcAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		srcAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		srcAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		srcAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		srcAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription dstAttachment{};
		dstAttachment.format = image_format;
		dstAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		dstAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		dstAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		dstAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		dstAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		dstAttachment.initialLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		dstAttachment.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

#ifdef THIRD_PASS
		dstAttachment.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		dstAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentDescription copyAttachment{};
		copyAttachment.format = engine.graphics.sc_format;
		copyAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		copyAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		copyAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		copyAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		copyAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		copyAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		copyAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		
		VkAttachmentReference copyAttachmentRef{};
		copyAttachmentRef.attachment = 2;
		copyAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
#endif

		VkAttachmentReference srcAttachmentRef{};
		srcAttachmentRef.attachment = 0;
		srcAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference dstAttachmentRef{};
		dstAttachmentRef.attachment = 1;
		dstAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

#ifdef THIRD_PASS
		VkSubpassDescription subpasses[3] = {};
#else
		VkSubpassDescription subpasses[2] = {};
#endif
		subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpasses[0].colorAttachmentCount = 1;
		subpasses[0].pColorAttachments = &srcAttachmentRef;

		VkAttachmentReference inputAttachments[1] = {
			{.attachment = 0, .layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
		};
		subpasses[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpasses[1].colorAttachmentCount = 1;
		subpasses[1].pColorAttachments = &dstAttachmentRef;
		subpasses[1].inputAttachmentCount = 1;
		subpasses[1].pInputAttachments = inputAttachments;
#ifdef THIRD_PASS
		VkAttachmentReference dstInputAttachments[1] = {
			{.attachment = 1, .layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
		};
		subpasses[2].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpasses[2].colorAttachmentCount = 1;
		subpasses[2].pColorAttachments = &copyAttachmentRef;
		subpasses[2].inputAttachmentCount = 1;
		subpasses[2].pInputAttachments = dstInputAttachments;
#endif

		VkSubpassDependency dependencies[THIRD_PASS ? 5 : 3] = {};
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = 0;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		dependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].dstSubpass = 1;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].srcAccessMask = 0;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		// Last subpass needs to wait on previous subpasses to complete before continuing
		dependencies[2].srcSubpass = 0;
		dependencies[2].dstSubpass = 1;
		dependencies[2].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[2].srcAccessMask = 0;
		dependencies[2].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[2].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
#ifdef THIRD_PASS
		dependencies[3].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[3].dstSubpass = 2;
		dependencies[3].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[3].srcAccessMask = 0;
		dependencies[3].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[3].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		dependencies[4].srcSubpass = 1;
		dependencies[4].dstSubpass = 2;
		dependencies[4].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[4].srcAccessMask = 0;
		dependencies[4].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[4].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
#endif

		auto attachments = {srcAttachment, dstAttachment,
#ifdef THIRD_PASS
			copyAttachment
#endif
		};

		VkRenderPassCreateInfo renderPassInfo{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
		renderPassInfo.attachmentCount = (fs::u32)attachments.size();
		renderPassInfo.pAttachments = attachments.begin();
		renderPassInfo.subpassCount = (fs::u32)std::size(subpasses);
		renderPassInfo.pSubpasses = subpasses;
		renderPassInfo.dependencyCount = (fs::u32)std::size(dependencies);
		renderPassInfo.pDependencies = dependencies;

		vkCreateRenderPass(engine.graphics.device, &renderPassInfo, nullptr, &render_pass.handle);
	}

	void create_blend_pipeline(VkDevice device) {
		VkPipelineShaderStageCreateInfo vertShaderStageInfo{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = fs::create_shader(device, blend_vs::size, blend_vs::data);
		vertShaderStageInfo.pName = "main";
		VkPipelineShaderStageCreateInfo fragShaderStageInfo{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fs::create_shader(device, blend_fs::size, blend_fs::data);
		fragShaderStageInfo.pName = "main";
		VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_BLEND_CONSTANTS};
		VkPipelineDynamicStateCreateInfo dynamicStateInfo{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
		dynamicStateInfo.dynamicStateCount = (fs::u32)std::size(dynamicStates);
		dynamicStateInfo.pDynamicStates = dynamicStates;

		VkPipelineViewportStateCreateInfo viewportState{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
		viewportState.viewportCount = 1;
		viewportState.pViewports = nullptr;
		viewportState.scissorCount = 1;
		viewportState.pScissors = nullptr;

		VkPipelineRasterizationStateCreateInfo rasterizer{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_NONE;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = 0b1111;
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_CONSTANT_ALPHA;

		VkPipelineColorBlendStateCreateInfo colorBlending{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
	//	colorBlending.blendConstants[3] = 0.98f;

		VkPipelineVertexInputStateCreateInfo vertex_input{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
		VkGraphicsPipelineCreateInfo pipelineInfo{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
		pipelineInfo.stageCount = (fs::u32)std::size(shaderStages);
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertex_input;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr; // Optional
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicStateInfo;
		pipelineInfo.layout = blend_pipeline_layout;
		pipelineInfo.renderPass = render_pass;
		pipelineInfo.subpass = 1;

		vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &blend_pipeline);

#ifdef THIRD_PASS
		pipelineInfo.subpass = 2;
		colorBlendAttachment.blendEnable = VK_FALSE;
		dynamicStateInfo.dynamicStateCount = 2;
		vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &copy_pipeline);
#endif

		vkDestroyShaderModule(device, vertShaderStageInfo.module, nullptr);
		vkDestroyShaderModule(device, fragShaderStageInfo.module, nullptr);
	}

	virtual void on_update(double dt, std::vector<fs::Event> const& events, fs::Render_Context* ctx) override {
		using namespace fs;

		static char buffer[256];
		static string text = FS_str_make(buffer, 0);

	//	for (auto&& e: events) {
	//		switch (e.type)
	//		{
	//		default:
	//		break; case Event_Character_Input: {
	//			auto c = (c8)e.character_input.codepoint;
	//			if (c == '\b') {
	//				if(text.count != 0) --text.count;
	//			}
	//			else text.data[text.count++] = c;
	//		}
	//		break;
	//		}
	//	}

		auto size = 100.0f;
		float rad = float(engine.graphics.sc_extent.height/2) - size*0.75f;
		auto mouse_pos = (v2f32)engine.window.mouse_position;
	
		static float t = 0.0f;
		v2f32 center = v2f32((float)engine.graphics.sc_extent.width, (float)engine.graphics.sc_extent.height)*0.5f;
		v2f32 pos = center + v2f32(cosf(t), sinf(t)) * rad;
		t += float(dt)*3.0f;
#if 0
		auto rect = rf32::from_center(pos, size, size);
		engine.renderer_2d.add_rect(rect, colors::CornflowerBlue);
		pos = center + v2f32(cosf(t+float(FS_PI)), sinf(t+float(FS_PI))) * rad;
		rect = rf32::from_center(pos, size, size);
		engine.renderer_2d.add_rect(rect, colors::CornflowerBlue);
#else
#define SQRT3_2 float(0.8660254037844386)
		v2f32 a, b, c;
		a = { 0.0f,0.6f* SQRT3_2};
		b = { 0.6f,0.6f*-SQRT3_2};
		c = {-0.6f,0.6f*-SQRT3_2};

		m22 rot = m22::Rotation(t*1.45312f);

		engine.renderer_2d.add_triangle(pos + rot*a*size, pos + rot*b*size, pos + rot*c*size, rgb(hsv(0.6f, 0.8f, 1.0f)));
		tetris_update(tetris, (float)dt, events, engine.renderer_2d);
#endif
	//	engine.renderer_2d.add_rect(rf32::from_center(mouse_pos, 50.0f, 50.0f), colors::White);

		engine.textured_renderer_2d.set_font(&engine.fonts.console);
		{
			auto s = engine.textured_renderer_2d.add_string(text, {}, colors::Lime);
			engine.renderer_2d.add_rect(rf32::from_topleft(s.x, 0.0f, engine.fonts.console.table.fallback.advance, engine.fonts.console.height), colors::Lime);
		}

		auto& cmd = ctx->command_buffer;

		VkClearValue clearColor[PASS_COUNT];
		memset(clearColor, 0, sizeof(clearColor));
	//	clearColor->color.float32[0] = 0.05f;
	//	clearColor->color.float32[1] = clearColor->color.float32[0];
	//	clearColor->color.float32[2] = clearColor->color.float32[0];
		VkRenderPassBeginInfo beginInfo{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
		beginInfo.clearValueCount = PASS_COUNT;
		beginInfo.pClearValues = clearColor;
		beginInfo.framebuffer = frame_buffers[ctx->image_index];
		beginInfo.renderPass = render_pass;
		beginInfo.renderArea.extent = engine.graphics.sc_extent;
		vkCmdBeginRenderPass(cmd, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

		// Draw Image to src Image
		engine.bind_font(cmd, &engine.fonts.console);
		engine.textured_renderer_2d.draw_pipeline(tpipeline, *ctx);
		engine.renderer_2d.draw_pipeline(pipeline, *ctx);

		vkCmdNextSubpass(cmd, VK_SUBPASS_CONTENTS_INLINE);
		
		// Blend src Image with dst Image
		VK_GFX_BIND_DESCRIPTOR_SETS(cmd, blend_pipeline_layout, 1, &blend_descriptor_set);
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, blend_pipeline);
		float blend = std::powf(0.9f, (float)dt * 120.0f);
		engine.debug_layer.add("blend = %.2f", blend);
		float blend_constants[4] = {0.0f, 0.0f, 0.0f, blend};
		vkCmdSetBlendConstants(cmd, blend_constants);
		vkCmdDraw(cmd, 3, 1, 0, 0);

#ifdef THIRD_PASS
		vkCmdNextSubpass(cmd, VK_SUBPASS_CONTENTS_INLINE);

		VK_GFX_BIND_DESCRIPTOR_SETS(cmd, blend_pipeline_layout, 1, &copy_descriptor_set);
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, copy_pipeline);
		vkCmdDraw(cmd, 3, 1, 0, 0);
#endif

		vkCmdEndRenderPass(cmd);
	}
	virtual void on_resize() override {
	
	}
	Scene_OK() {
		create_render_pass();

		auto& gfx = engine.graphics;

		{
			auto format = image_format;
			VmaAllocationCreateInfo allocInfo{};
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
			VkImageCreateInfo imageInfo{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
			imageInfo.arrayLayers = 1;
			imageInfo.extent = vk::extent3d(gfx.sc_extent);
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.mipLevels = 1;

			imageInfo.format = src_image_format;
			imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
			vmaCreateImage(gfx.allocator, &imageInfo, &allocInfo, &src_image, &src_allocation, nullptr);

			imageInfo.format = format;
			imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
#ifdef THIRD_PASS
			| VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
#else
			| VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
#endif
			vmaCreateImage(gfx.allocator, &imageInfo, &allocInfo, &dst_image, &dst_allocation, nullptr);

			auto viewInfo = vk::image_view_2d(src_image, src_image_format);
			vkCreateImageView(gfx.device, &viewInfo, nullptr, &src_view);

			viewInfo.format = format;
			viewInfo.image = dst_image;
			vkCreateImageView(gfx.device, &viewInfo, nullptr, &dst_view);

			auto const stride = vk::size_of(format);
			auto data = _aligned_malloc(imageInfo.extent.width * imageInfo.extent.height * stride, 1024);
			memset(data, 0, imageInfo.extent.width * imageInfo.extent.height * stride);
#ifdef THIRD_PASS
			gfx.upload_image(dst_image, data, imageInfo.extent, format);
#else
			gfx.upload_image(dst_image, data, imageInfo.extent, format, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
#endif
			_aligned_free(data);
		}
		
		{
			VkImageView attachments[] = {src_view, dst_view,VK_NULL_HANDLE};
			VkFramebufferCreateInfo framebufferInfo{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
			framebufferInfo.renderPass = render_pass;
			framebufferInfo.attachmentCount = (fs::u32)std::size(attachments);
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width  = gfx.sc_extent.width;
			framebufferInfo.height = gfx.sc_extent.height;
			framebufferInfo.layers = 1;
			FS_FOR(gfx.sc_image_count) {
				attachments[2] = gfx.sc_image_views[i];
				vkCreateFramebuffer(gfx.device, &framebufferInfo, nullptr, frame_buffers+i);
			}
		}

		{
			VkDescriptorSetLayoutBinding binding{};
			binding.binding = 0;
			binding.descriptorCount = 1;
			binding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			VkDescriptorSetLayoutCreateInfo descriptorInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
			descriptorInfo.bindingCount = 1;
			descriptorInfo.pBindings = &binding;
			vkCreateDescriptorSetLayout(gfx.device, &descriptorInfo, nullptr, &blend_descriptor_layout);

			VkPipelineLayoutCreateInfo pipelineLayoutInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
			pipelineLayoutInfo.pSetLayouts = &blend_descriptor_layout;
			pipelineLayoutInfo.setLayoutCount = 1;
			vkCreatePipelineLayout(gfx.device, &pipelineLayoutInfo, nullptr, &blend_pipeline_layout);

			VkDescriptorSetAllocateInfo descSetAllocInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
			descSetAllocInfo.descriptorPool = engine.descriptor_pool;
			descSetAllocInfo.pSetLayouts = &blend_descriptor_layout;
			descSetAllocInfo.descriptorSetCount = 1;
			vkAllocateDescriptorSets(gfx.device, &descSetAllocInfo, &blend_descriptor_set);

#ifdef THIRD_PASS
			vkAllocateDescriptorSets(gfx.device, &descSetAllocInfo, &copy_descriptor_set);
#endif

			VkDescriptorImageInfo imageInfo;
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = src_view;
			imageInfo.sampler = nullptr;
			VkWriteDescriptorSet write{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
			write.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			write.descriptorCount = 1;
			write.dstBinding = 0;
			write.dstSet = blend_descriptor_set;
			write.pImageInfo = &imageInfo;
			vkUpdateDescriptorSets(gfx.device, 1, &write, 0, nullptr);
#ifdef THIRD_PASS
			imageInfo.imageView = dst_view;
			write.dstSet = copy_descriptor_set;
			vkUpdateDescriptorSets(gfx.device, 1, &write, 0, nullptr);
#endif
		}

		fs::Pipeline_Create_Info pipelineInfo;
		pipelineInfo.device = gfx.device;
		pipelineInfo.render_pass = render_pass;
		{
			auto vertex_input = vk::Basic_Vertex_Input<fs::v2f32, fs::color>{};
			pipelineInfo.pipeline_layout = engine.renderer_2d.pipeline_layout;
			pipelineInfo.vertex_shader = engine.renderer_2d.vert;
			pipelineInfo.fragment_shader = engine.renderer_2d.frag;
			pipelineInfo.vertex_input = &vertex_input;
			pipelineInfo.blend_mode = fs::Blending_Mode::Blend_Mode_Add;
			fs::create_pipeline(pipelineInfo, &pipeline);
		}
		{
			auto vertex_input = vk::Basic_Vertex_Input<fs::v2f32, fs::v2f32, fs::color>{};
			pipelineInfo.pipeline_layout = engine.textured_renderer_2d.pipeline_layout;
			pipelineInfo.vertex_shader = engine.textured_renderer_2d.vert;
			pipelineInfo.fragment_shader = engine.textured_renderer_2d.frag;
			pipelineInfo.vertex_input = &vertex_input;
			pipelineInfo.blend_mode = fs::Blend_Mode_Add;
			fs::create_pipeline(pipelineInfo, &tpipeline);
		}

		create_blend_pipeline(gfx.device);

		tetris = tetris_init(GAME_WIDTH, GAME_HEIGHT);
	}
	virtual ~Scene_OK() override {
		auto& gfx = engine.graphics;

		vmaDestroyImage(gfx.allocator, src_image, src_allocation);
		vmaDestroyImage(gfx.allocator, dst_image, dst_allocation);
		vkDestroyImageView(gfx.device, src_view, nullptr);
		vkDestroyImageView(gfx.device, dst_view, nullptr);
		vkDestroyDescriptorSetLayout(gfx.device, blend_descriptor_layout, nullptr);
		vkDestroyPipelineLayout(gfx.device, blend_pipeline_layout, nullptr);
		vkDestroyPipeline(gfx.device, pipeline, nullptr);
		vkDestroyPipeline(gfx.device, tpipeline, nullptr);
		vkDestroyPipeline(gfx.device, copy_pipeline, nullptr);
		vkDestroyPipeline(gfx.device, blend_pipeline, nullptr);
		FS_FOR(engine.graphics.sc_image_count) vkDestroyFramebuffer(gfx.device, frame_buffers[i], nullptr);
		render_pass.destroy();

		tetris_uninit(tetris);
	}

	Tetris* tetris;
	fs::Render_Pass render_pass;

	VkImage       src_image;
	VkImage       dst_image;
	VmaAllocation src_allocation;
	VmaAllocation dst_allocation;
	VkImageView   src_view;
	VkImageView   dst_view;
#ifdef THIRD_PASS
	VkPipeline      copy_pipeline;
	VkDescriptorSet copy_descriptor_set;
#endif

	VkFramebuffer frame_buffers[fs::Graphics::max_sc_images];

	VkDescriptorSetLayout blend_descriptor_layout;
	VkPipelineLayout      blend_pipeline_layout;
	VkDescriptorSet       blend_descriptor_set;

	VkPipeline    pipeline;
	VkPipeline    tpipeline;
	VkPipeline    blend_pipeline;
};

bool operator==(fs::string Left, char const* Right) {
	FS_FOR(Left.count) {
		if (Left.data[i] != Right[i]) {
			return false;
		}
	}
	return Left.count == strlen(Right);
}

fs::Scene* on_create_scene(fs::Scene_Key const& key) {
	using namespace fs;
	return nullptr;
	// Default:
	if (key.id().is_empty()) {
		return new Scene_OK;
	}
	else if (key.id() == "Tetris") {
		return new Tetris_Scene;
	}
	else if (key.id() == "Start") {
		return new Scene_OK;
	}
	return nullptr; // scene id is undefined
}

fs::Defaults on_create() {

	engine.app_version = {0, 1, 7};
#ifdef DEBUG
	engine.app_version_info = FS_str("vanilla/dev");
#else
	engine.app_version_info = FS_str("vanilla");
#endif
	engine.app_name = FS_str("sandbox");
	return {
		.window_title = FS_str(u8" Sandbox reflexões 出展 うこそ"),
#if 0
		.window_width  = 600 + GAME_WIDTH * CELL_SIZE,
		.window_height = GAME_HEIGHT * CELL_SIZE + 20,
#elif 1
		.window_width = 1280,
		.window_height = 720,
#else
		.window_width  = 1920,
		.window_height = 1080,
		.window_mode  = fs::Windowed_Fullscreen,
#endif
	};
}
