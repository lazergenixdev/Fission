/**
 *	______________              _____
 *	___  ____/__(_)________________(_)____________
 *	__  /_   __  /__  ___/_  ___/_  /_  __ \_  __ \
 *	_  __/   _  / _(__  )_(__  )_  / / /_/ /  / / /
 *	/_/      /_/  /____/ /____/ /_/  \____//_/ /_/
 *
 *
 * @Author:       lazergenixdev@gmail.com
 * @Development:  (https://github.com/lazergenixdev/Fission)
 * @License:      MIT (see end of file)
 */
#pragma once
#include "Fission/Core/Graphics.hh"
#include "Fission/Core/Font.hh"
#include "Fission/Base/Math/Matrix.hpp"
#include "Fission/Base/Rect.hpp"
#include "Fission/Base/Color.hpp"
#include "Fission/Base/String.hpp"
#include <fstream>

__FISSION_BEGIN__

template <typename Vtx, typename Idx>
struct Frame_Data {
	VkBuffer vertex_buffer;
	VkBuffer index_buffer;

	VmaAllocation vertex_allocation;
	VmaAllocation index_allocation;

	Frame_Data() = default;
	Frame_Data(VmaAllocator allocator, u32 max_vertex_count, u32 max_index_count) {
		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
		allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
		VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };

		bufferInfo.size = max_vertex_count * sizeof(Vtx);
		bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &vertex_buffer, &vertex_allocation, nullptr);

		bufferInfo.size = max_index_count * sizeof(Idx);
		bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &index_buffer, &index_allocation, nullptr);
	}

	void set_data(Graphics* gfx, void* vertex_data, void* index_data, u32 vertex_count, u32 index_count) {
		void* gpu_vertex_data, * gpu_index_data;
		vmaMapMemory(gfx->allocator, vertex_allocation, &gpu_vertex_data);
		vmaMapMemory(gfx->allocator, index_allocation, &gpu_index_data);

		memcpy(gpu_vertex_data, vertex_data, vertex_count * sizeof(Vtx));
		memcpy(gpu_index_data, index_data, index_count * sizeof(Idx));

		vmaUnmapMemory(gfx->allocator, vertex_allocation);
		vmaUnmapMemory(gfx->allocator, index_allocation);

		vmaFlushAllocation(gfx->allocator, vertex_allocation, 0, vertex_count * sizeof(Vtx));
		vmaFlushAllocation(gfx->allocator, index_allocation, 0, index_count * sizeof(Idx));
	}
};

struct solid_color_vertex {
	v2f32 position;
	rgba  color;
};
struct textured_vertex {
	v2f32 position;
	v2f32 texcoord;
	rgba  color;
};

enum Blending_Mode {
	Blend_Mode_Disabled,
	Blend_Mode_Normal,
	Blend_Mode_Add,
};
struct Pipeline_Create_Info {
	VkDevice device;
	VkRenderPass render_pass;
	VkPipelineLayout pipeline_layout;
	VkShaderModule vertex_shader;
	VkShaderModule fragment_shader;
	VkPipelineVertexInputStateCreateInfo const* vertex_input;
	Blending_Mode blend_mode;
	uint32_t subpass = 0;
};
static void create_pipeline(Pipeline_Create_Info const& createInfo, VkPipeline* outPipeline) {
	VkPipelineShaderStageCreateInfo vertShaderStageInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = createInfo.vertex_shader;
	vertShaderStageInfo.pName = "main";
	VkPipelineShaderStageCreateInfo fragShaderStageInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = createInfo.fragment_shader;
	fragShaderStageInfo.pName = "main";
	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamicStateInfo{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
	dynamicStateInfo.dynamicStateCount = (u32)std::size(dynamicStates);
	dynamicStateInfo.pDynamicStates = dynamicStates;

	VkPipelineViewportStateCreateInfo viewportState{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
	viewportState.viewportCount = 1;
	viewportState.pViewports = nullptr;
	viewportState.scissorCount = 1;
	viewportState.pScissors = nullptr;

	VkPipelineRasterizationStateCreateInfo rasterizer{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = 0b1111;
	switch (createInfo.blend_mode)
	{
	default:
	break; case Blend_Mode_Disabled:
		colorBlendAttachment.blendEnable = VK_FALSE;
	break; case Blend_Mode_Normal:
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	break; case Blend_Mode_Add:
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	break;
	}

	VkPipelineColorBlendStateCreateInfo colorBlending{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;

	VkGraphicsPipelineCreateInfo pipelineInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
	pipelineInfo.stageCount = (u32)std::size(shaderStages);
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = createInfo.vertex_input;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr; // Optional
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicStateInfo;
	pipelineInfo.layout = createInfo.pipeline_layout;
	pipelineInfo.renderPass = createInfo.render_pass;
	pipelineInfo.subpass = createInfo.subpass;

	vkCreateGraphicsPipelines(createInfo.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, outPipeline);
}

static VkShaderModule create_shader(VkDevice device, size_t size, void const* data) {
	VkShaderModule module;
	VkShaderModuleCreateInfo createInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
	createInfo.codeSize = size;
	createInfo.pCode = reinterpret_cast<const uint32_t*>(data);
	vkCreateShaderModule(device, &createInfo, nullptr, &module);
	return module;
}

struct Draw_Data {
	u32 total_vtx_count {0};
	u32 total_idx_count {0};

	u32 vtx_offset {0};
	u32 idx_offset {0};
	u32 vtx_count  {0};
	u32 idx_count  {0};

	inline void reset() { memset(this, 0, sizeof(*this)); }
	inline void start_new_draw() {
		vtx_offset = total_vtx_count;
		idx_offset = total_idx_count;
		vtx_count = 0;
		idx_count = 0;
	}
};

static constexpr u32 _r2d_max_count = 1 << 16;

struct Renderer_2D
{
	using vertex = solid_color_vertex;
	using Frame_Data = Frame_Data<vertex, u16>;
public:
	Renderer_2D() = default;

	void create(Graphics* gfx, VkRenderPass render_pass, Transform_2D_Layout layout);

	void add_triangle(v2f32 p0, v2f32 p1, v2f32 p2, color color) {
		index_data[d.total_idx_count++] = d.vtx_count;
		index_data[d.total_idx_count++] = d.vtx_count + 1;
		index_data[d.total_idx_count++] = d.vtx_count + 2;
		
		vertex_data[d.total_vtx_count++] = {p0, color};
		vertex_data[d.total_vtx_count++] = {p1, color};
		vertex_data[d.total_vtx_count++] = {p2, color};

		d.idx_count += 3;
		d.vtx_count += 3;
	}

	void add_rect(rf32 rect, color color) {
		index_data[d.total_idx_count++] = d.vtx_count + 0;
		index_data[d.total_idx_count++] = d.vtx_count + 1;
		index_data[d.total_idx_count++] = d.vtx_count + 2;
		index_data[d.total_idx_count++] = d.vtx_count + 2;
		index_data[d.total_idx_count++] = d.vtx_count + 3;
		index_data[d.total_idx_count++] = d.vtx_count + 0;
		
		vertex_data[d.total_vtx_count++] = {{rect.x.low , rect.y.low }, color};
		vertex_data[d.total_vtx_count++] = {{rect.x.low , rect.y.high}, color};
		vertex_data[d.total_vtx_count++] = {{rect.x.high, rect.y.high}, color};
		vertex_data[d.total_vtx_count++] = {{rect.x.high, rect.y.low }, color};

		d.idx_count += 6;
		d.vtx_count += 4;
	}

	// TODO: remove
	void add_rect(rf32 rect, color color1, color color2) {
		index_data[d.total_idx_count++] = d.vtx_count + 0;
		index_data[d.total_idx_count++] = d.vtx_count + 1;
		index_data[d.total_idx_count++] = d.vtx_count + 2;
		index_data[d.total_idx_count++] = d.vtx_count + 2;
		index_data[d.total_idx_count++] = d.vtx_count + 3;
		index_data[d.total_idx_count++] = d.vtx_count + 0;
		
		vertex_data[d.total_vtx_count++] = {{rect.x.low , rect.y.low }, color1};
		vertex_data[d.total_vtx_count++] = {{rect.x.low , rect.y.high}, color1};
		vertex_data[d.total_vtx_count++] = {{rect.x.high, rect.y.high}, color2};
		vertex_data[d.total_vtx_count++] = {{rect.x.high, rect.y.low }, color2};

		d.idx_count += 6;
		d.vtx_count += 4;
	}

	void end_render(Render_Context const* ctx) {
		if (d.total_vtx_count) {
			auto& fd = frame_data[ctx->frame];
			fd.set_data(ctx->gfx, vertex_data, index_data, d.total_vtx_count, d.total_idx_count);
			d.reset();
		}
	}

	void draw(Render_Context& ctx) {
		auto& fd = frame_data[ctx.frame];

		vkCmdBindPipeline(ctx.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

		vkCmdBindIndexBuffer(ctx.command_buffer, fd.index_buffer, 0, VK_INDEX_TYPE_UINT16);
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(ctx.command_buffer, 0, 1, &fd.vertex_buffer, &offset);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(ctx.gfx->sc_extent.width);
		viewport.height = static_cast<float>(ctx.gfx->sc_extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(ctx.command_buffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = ctx.gfx->sc_extent;
		vkCmdSetScissor(ctx.command_buffer, 0, 1, &scissor);

		vkCmdDrawIndexed(ctx.command_buffer, d.idx_count, 1, d.idx_offset, d.vtx_offset, 0);

		d.start_new_draw();
	}
	void destroy() {
		vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
		vkDestroyPipeline(device, pipeline, nullptr);
		_aligned_free(vertex_data);
		_aligned_free(index_data);
		for (auto&& fd : frame_data) {
			vmaDestroyBuffer(allocator, fd.vertex_buffer, fd.vertex_allocation);
			vmaDestroyBuffer(allocator, fd.index_buffer, fd.index_allocation);
		}
	}

	VkDevice device;
	VmaAllocator allocator;
	
	VkPipeline pipeline;
	VkPipelineLayout pipeline_layout;

	u32 max_vertex_count;
	u32 max_index_count;

	Frame_Data frame_data[2];

	vertex* vertex_data;
	u16*    index_data;

	Draw_Data d;
};

struct Textured_Renderer_2D
{
	using vertex = textured_vertex;
	using Frame_Data = Frame_Data<vertex, u16>;
public:
	Textured_Renderer_2D() = default;

	void create(Graphics* gfx, VkRenderPass render_pass, Transform_2D_Layout transform_layout, Texture_Layout texture_layout);

	void add_rect(rf32 rect, rf32 uv, color color) {
		index_data[d.total_idx_count++] = d.vtx_count + 0;
		index_data[d.total_idx_count++] = d.vtx_count + 1;
		index_data[d.total_idx_count++] = d.vtx_count + 2;
		index_data[d.total_idx_count++] = d.vtx_count + 2;
		index_data[d.total_idx_count++] = d.vtx_count + 3;
		index_data[d.total_idx_count++] = d.vtx_count + 0;
		
		vertex_data[d.total_vtx_count++] = {{rect.x.low , rect.y.low }, {uv.x.low , uv.y.low }, color};
		vertex_data[d.total_vtx_count++] = {{rect.x.low , rect.y.high}, {uv.x.low , uv.y.high}, color};
		vertex_data[d.total_vtx_count++] = {{rect.x.high, rect.y.high}, {uv.x.high, uv.y.high}, color};
		vertex_data[d.total_vtx_count++] = {{rect.x.high, rect.y.low }, {uv.x.high, uv.y.low }, color};

		d.idx_count += 6;
		d.vtx_count += 4;
	}

	void add_glyph( const fs::Glyph* g, const v2f32& origin, const float& scale, const color& color )
	{
		index_data[d.total_idx_count++] = d.vtx_count;
		index_data[d.total_idx_count++] = d.vtx_count + 1u;
		index_data[d.total_idx_count++] = d.vtx_count + 2u;
		index_data[d.total_idx_count++] = d.vtx_count + 3u;
		index_data[d.total_idx_count++] = d.vtx_count;
		index_data[d.total_idx_count++] = d.vtx_count + 2u;

		const auto rect = rf32{
			origin.x + scale * g->rc.x.low,
			origin.x + scale * g->rc.x.high,
			origin.y + scale * g->rc.y.low,
			origin.y + scale * g->rc.y.high,
		};

		vertex_data[d.total_vtx_count++] = {{rect.x.low,  rect.y.high}, {g->uv.x.low,  g->uv.y.high}, color};
		vertex_data[d.total_vtx_count++] = {{rect.x.low,  rect.y.low }, {g->uv.x.low,  g->uv.y.low }, color};
		vertex_data[d.total_vtx_count++] = {{rect.x.high, rect.y.low }, {g->uv.x.high, g->uv.y.low }, color};
		vertex_data[d.total_vtx_count++] = {{rect.x.high, rect.y.high}, {g->uv.x.high, g->uv.y.high}, color};

		d.idx_count += 6;
		d.vtx_count += 4;
	}

	// exists so that there is no need to pass extra parameter to add_string
	void set_font(struct Font* font) {
		current_font = font;
	}

	// right-aligned string
	v2f32 add_string_rtl(string str, v2f32 top_right, color col) {
		auto pos = top_right;

		const float right  = top_right.x;
		const float starty = top_right.y;
		float width = 0.0f;
		fs::Glyph const* glyph;

		for (u64 i = 0; i < str.count; ++i)
		{
			u32 c = str.data[str.count - i - 1];
			// newline
			if (c == '\r' || c == '\n') {
				float w = right - pos.x;
				if (w > width)
					width = w;

				pos.y += current_font->height;
				pos.x = right;
				continue;
			}

			glyph = current_font->lookup(c);
			pos.x -= glyph->advance;

			if (c != ' ') {
				add_glyph(glyph, pos, 1.0f, col);
			}
		}

		width = std::max(width, right - pos.x);
		return { width, pos.y - starty + current_font->height };
	}

	v2f32 add_string(string str, v2f32 top_left, color col) {
#if _DEBUG
		if (!current_font) throw "failure";
#endif
		auto pos = top_left;

		const float left = top_left.x;
		const float starty = top_left.y;
		float width = 0.0f;
		fs::Glyph const* glyph;

		for (u64 i = 0; i < str.count; ++i)
		{
			u32 c = str.data[i];
			// newline
			if (c == '\r' || c == '\n') {
				float w = pos.x - left;
				if (w > width)
					width = w;

				pos.y += current_font->height;
				pos.x = left;
				continue;
			}

			glyph = current_font->lookup(c);

			if (c != ' ') {
				add_glyph(glyph, pos, 1.0f, col);
			}

			pos.x += glyph->advance;
		}

		width = std::max(width, pos.x - left);
		return { width, pos.y - starty + current_font->height };
	}

	void end_render(Render_Context const* ctx) {
		if (d.total_vtx_count) {
			auto& fd = frame_data[ctx->frame];
			fd.set_data(ctx->gfx, vertex_data, index_data, d.total_vtx_count, d.total_idx_count);
			d.reset();
		}
	}

	void draw(Render_Context& ctx) {
		auto& fd = frame_data[ctx.frame];

		vkCmdBindPipeline(ctx.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

		vkCmdBindIndexBuffer(ctx.command_buffer, fd.index_buffer, 0, VK_INDEX_TYPE_UINT16);
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(ctx.command_buffer, 0, 1, &fd.vertex_buffer, &offset);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(ctx.gfx->sc_extent.width);
		viewport.height = static_cast<float>(ctx.gfx->sc_extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(ctx.command_buffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = ctx.gfx->sc_extent;
		vkCmdSetScissor(ctx.command_buffer, 0, 1, &scissor);

		vkCmdDrawIndexed(ctx.command_buffer, d.idx_count, 1, d.idx_offset, d.vtx_offset, 0);
		
		d.start_new_draw();
	}
	void destroy() {
		vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
		vkDestroyPipeline(device, pipeline, nullptr);
		_aligned_free(vertex_data);
		_aligned_free(index_data);
		for (auto&& fd : frame_data) {
			vmaDestroyBuffer(allocator, fd.vertex_buffer, fd.vertex_allocation);
			vmaDestroyBuffer(allocator, fd.index_buffer, fd.index_allocation);
		}
	}

	VkDevice device;
	VmaAllocator allocator;
	
	VkPipeline pipeline;
	VkPipelineLayout pipeline_layout;

	u32 max_vertex_count;
	u32 max_index_count;

	Frame_Data frame_data[2];

	Font* current_font;

	vertex* vertex_data;
	u16*    index_data;

	Draw_Data d;
};

__FISSION_END__

/**
 *	MIT License
 *
 *	Copyright (c) 2021-2023 lazergenixdev
 *
 *	Permission is hereby granted, free of charge, to any person obtaining a copy
 *	of this software and associated documentation files (the "Software"), to deal
 *	in the Software without restriction, including without limitation the rights
 *	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *	copies of the Software, and to permit persons to whom the Software is
 *	furnished to do so, subject to the following conditions:
 *
 *	The above copyright notice and this permission notice shall be included in all
 *	copies or substantial portions of the Software.
 *
 *	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *	SOFTWARE.
 */