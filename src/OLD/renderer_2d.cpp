#include <Fission/graphics/renderer_2d.hpp>
#include <Fission/core/engine.hpp>
#include <Fission/base/memory.hpp>
#include "fmt/format.h"

extern fs::Engine engine;

FISSION_NAMESPACE_BEGIN

impl::Frame_Data::Frame_Data(VmaAllocator allocator, u32 max_vertex_size, u32 max_index_size) {
    VmaAllocationCreateInfo allocation_info {
        .usage = VMA_MEMORY_USAGE_AUTO,
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
    };
    VkBufferCreateInfo buffer_info { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };

    buffer_info.size = max_vertex_size;
    buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    vmaCreateBuffer(allocator, &buffer_info, &allocation_info, &vertex_buffer, &vertex_allocation, nullptr);

    buffer_info.size = max_index_size;
    buffer_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    vmaCreateBuffer(allocator, &buffer_info, &allocation_info, &index_buffer, &index_allocation, nullptr);
}

void impl::Frame_Data::send(VmaAllocator allocator, void* vertex_data, void* index_data, u32 vertex_size, u32 index_size) {
    void* gpu_vertex_data, * gpu_index_data;
    vmaMapMemory(allocator, vertex_allocation, &gpu_vertex_data);
    vmaMapMemory(allocator, index_allocation, &gpu_index_data);

    memcpy(gpu_vertex_data, vertex_data, vertex_size);
    memcpy(gpu_index_data, index_data, index_size);

    vmaUnmapMemory(allocator, vertex_allocation);
    vmaUnmapMemory(allocator, index_allocation);

    vmaFlushAllocation(allocator, vertex_allocation, 0, vertex_size);
    vmaFlushAllocation(allocator, index_allocation, 0, index_size);
}

struct solid_color_vs : public vk::embed::Vertex_Shader {
#   include "shaders/solid_color.vert.inl"
};
struct solid_color_fs : public vk::embed::Fragment_Shader {
#   include "../resources/BinaryShaders/solid_color.frag.inl"
};
struct textured_2d_vs : public vk::embed::Vertex_Shader {
#   include "shaders/texture_2d.vert.inl"
};
struct textured_2d_fs : public vk::embed::Fragment_Shader {
#   include "../resources/BinaryShaders/textured_2d.frag.inl"
};

#define MAX_COUNT 0xFFFF

void set_normal_blending(VkPipelineColorBlendAttachmentState& attachment) {
	//break; case Blend_Mode_Disabled:
	//	colorBlendAttachment.blendEnable = VK_FALSE;
	//break; case Blend_Mode_Normal:
	attachment.blendEnable         = VK_TRUE;
	attachment.colorBlendOp        = VK_BLEND_OP_ADD;
	attachment.alphaBlendOp        = VK_BLEND_OP_ADD;
	attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	//break; case Blend_Mode_Add:
	//	colorBlendAttachment.blendEnable = VK_TRUE;
	//	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	//	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	//	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	//	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
	//	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	//	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
}

void Renderer_2D::create(Graphics* gfx, VkRenderPass render_pass, Transform_2D_Layout layout)
{
	max_vertex_count = MAX_COUNT;
	max_index_count = MAX_COUNT * 2;

	bump_allocator {max_vertex_count * sizeof(Vertex) + max_index_count * sizeof(u16)}
	    .alloc_to(vertex_data, max_vertex_count)
	    .alloc_to(index_data, max_index_count)
	    .release();

	frame_data[0] = impl::Frame_Data(gfx->allocator, max_vertex_count * sizeof(Vertex), max_index_count * sizeof(u16));
	frame_data[1] = impl::Frame_Data(gfx->allocator, max_vertex_count * sizeof(Vertex), max_index_count * sizeof(u16));

	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		    .pSetLayouts = &layout,
		    .setLayoutCount = 1,
        };
		vkCreatePipelineLayout(gfx->device, &pipelineLayoutInfo, nullptr, &pipeline_layout);
	}

	auto vertex_input = vk::Basic_Vertex_Input<v2f32, color>{};
    auto pc = vk::Pipeline_Creator{render_pass, pipeline_layout}
        .vertex_input(&vertex_input)
        .add_dynamic_state(VK_DYNAMIC_STATE_VIEWPORT)
        .add_dynamic_state(VK_DYNAMIC_STATE_SCISSOR)
        .add_shader<solid_color_vs>()
        .add_shader<solid_color_fs>();

    set_normal_blending(pc.blend_attachment);

    pc.create(&pipeline);

	frag = pc.shaders[0].module;
	vert = pc.shaders[1].module;

    draw_call_count = 0;
}


void Textured_Renderer_2D::create(
	Graphics* gfx,
	VkRenderPass render_pass,
	Transform_2D_Layout transform_layout,
	Texture_Layout texture_layout
) {
	max_vertex_count = MAX_COUNT;
	max_index_count = MAX_COUNT * 2;
	
	bump_allocator {max_vertex_count * sizeof(Vertex) + max_index_count * sizeof(u16)}
	    .alloc_to(vertex_data, max_vertex_count)
	    .alloc_to(index_data, max_index_count)
	    .release();

	frame_data[0] = impl::Frame_Data(gfx->allocator, max_vertex_count * sizeof(Vertex), max_index_count * sizeof(u16));
	frame_data[1] = impl::Frame_Data(gfx->allocator, max_vertex_count * sizeof(Vertex), max_index_count * sizeof(u16));

	{
		VkDescriptorSetLayout layouts[2] { transform_layout, texture_layout };
		VkPipelineLayoutCreateInfo pipelineLayoutInfo {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		    .pSetLayouts = layouts,
		    .setLayoutCount = 2,
        };
		vkCreatePipelineLayout(gfx->device, &pipelineLayoutInfo, nullptr, &pipeline_layout);
	}

	auto vertex_input = vk::Basic_Vertex_Input<v2f32, v2f32, color>{};
    auto pc = vk::Pipeline_Creator{render_pass, pipeline_layout}
        .vertex_input(&vertex_input)
        .add_dynamic_state(VK_DYNAMIC_STATE_VIEWPORT)
        .add_dynamic_state(VK_DYNAMIC_STATE_SCISSOR)
        .add_shader<textured_2d_vs>()
        .add_shader<textured_2d_fs>();

    
    set_normal_blending(pc.blend_attachment);

    pc.create(&pipeline);

	frag = pc.shaders[0].module;
	vert = pc.shaders[1].module;

    draw_call_count = 0;
}

void Renderer_2D::destroy() {
	FISSION_DEFAULT_FREE(vertex_data);
	vkDestroyShaderModule(engine.graphics.device, vert, nullptr);
	vkDestroyShaderModule(engine.graphics.device, frag, nullptr);
	vkDestroyPipelineLayout(engine.graphics.device, pipeline_layout, nullptr);
	vkDestroyPipeline(engine.graphics.device, pipeline, nullptr);
	for (auto&& fd : frame_data) {
		vmaDestroyBuffer(engine.graphics.allocator, fd.vertex_buffer, fd.vertex_allocation);
		vmaDestroyBuffer(engine.graphics.allocator, fd.index_buffer, fd.index_allocation);
	}
}

void Textured_Renderer_2D::destroy() {
	FISSION_DEFAULT_FREE(vertex_data);
	vkDestroyShaderModule(engine.graphics.device, vert, nullptr);
	vkDestroyShaderModule(engine.graphics.device, frag, nullptr);
	vkDestroyPipelineLayout(engine.graphics.device, pipeline_layout, nullptr);
	vkDestroyPipeline(engine.graphics.device, pipeline, nullptr);
	for (auto&& fd : frame_data) {
		vmaDestroyBuffer(engine.graphics.allocator, fd.vertex_buffer, fd.vertex_allocation);
		vmaDestroyBuffer(engine.graphics.allocator, fd.index_buffer, fd.index_allocation);
	}
}

void Renderer_2D::draw_pipeline(Render_Context const& ctx, VkPipeline pipeline) {
	auto& fd = frame_data[ctx.frame];

	vkCmdBindPipeline(ctx.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

	vkCmdBindIndexBuffer(ctx.command_buffer, fd.index_buffer, 0, VK_INDEX_TYPE_UINT16);
	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(ctx.command_buffer, 0, 1, &fd.vertex_buffer, &offset);
	vkCmdDrawIndexed(ctx.command_buffer, d.idx_count, 1, d.idx_offset, d.vtx_offset, 0);

	d.start_new_draw();
    draw_call_count++;
}

void Textured_Renderer_2D::draw_pipeline(Render_Context const& ctx, VkPipeline pipeline) {
	auto& fd = frame_data[ctx.frame];

	VkDeviceSize offset {0};

	vkCmdBindPipeline(ctx.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	vkCmdBindIndexBuffer(ctx.command_buffer, fd.index_buffer, 0, VK_INDEX_TYPE_UINT16);
	vkCmdBindVertexBuffers(ctx.command_buffer, 0, 1, &fd.vertex_buffer, &offset);
	vkCmdDrawIndexed(ctx.command_buffer, d.idx_count, 1, d.idx_offset, d.vtx_offset, 0);

	d.start_new_draw();
    draw_call_count++;
}

void Renderer_2D::end_render(Render_Context const& ctx) {
    float vmem = math::min(d.total_vtx_count, d.total_idx_count / 2);
    engine.debug_layer.add(fmt::format("2d renderer (memory {:.3f}%) ({} draw calls)", vmem / float(MAX_COUNT), draw_call_count));

	if (d.total_vtx_count) {
		auto& fd = frame_data[ctx.frame];
		fd.send(engine.graphics.allocator, vertex_data, index_data, d.total_vtx_count * sizeof(Vertex), d.total_idx_count * sizeof(u16));
		d.reset();
	}
    draw_call_count = 0;
}

void Textured_Renderer_2D::end_render(Render_Context const& ctx) {
    float vmem = math::min(d.total_vtx_count, d.total_idx_count / 2);
    engine.debug_layer.add(fmt::format("uv renderer (memory {:.3f}%) ({} draw calls)", vmem / float(MAX_COUNT), draw_call_count));

	if (d.total_vtx_count) {
		auto& fd = frame_data[ctx.frame];
		fd.send(engine.graphics.allocator, vertex_data, index_data, d.total_vtx_count * sizeof(Vertex), d.total_idx_count * sizeof(u16));
		d.reset();
	}
    draw_call_count = 0;
}

void Renderer_2D::add_triangle(v2f32 p0, v2f32 p1, v2f32 p2, color color) {
    index_data[d.total_idx_count++] = u16(d.vtx_count);
    index_data[d.total_idx_count++] = u16(d.vtx_count + 1);
    index_data[d.total_idx_count++] = u16(d.vtx_count + 2);
    
    vertex_data[d.total_vtx_count++] = {p0, color};
    vertex_data[d.total_vtx_count++] = {p1, color};
    vertex_data[d.total_vtx_count++] = {p2, color};

    d.idx_count += 3;
    d.vtx_count += 3;
}

void Renderer_2D::add_triangle(v2f32 p0, v2f32 p1, v2f32 p2, color c0, color c1, color c2) {
    index_data[d.total_idx_count++] = d.vtx_count;
    index_data[d.total_idx_count++] = d.vtx_count + 1;
    index_data[d.total_idx_count++] = d.vtx_count + 2;

    vertex_data[d.total_vtx_count++] = { p0, c0 };
    vertex_data[d.total_vtx_count++] = { p1, c1 };
    vertex_data[d.total_vtx_count++] = { p2, c2 };

    d.idx_count += 3;
    d.vtx_count += 3;
}

void Renderer_2D::add_rect(rf32 rect, color color) {
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

void Renderer_2D::add_rect(rf32 rect, color c_top_left, color c_top_right, color c_bot_left, color c_bot_right)
{
    index_data[d.total_idx_count++] = d.vtx_count + 0;
    index_data[d.total_idx_count++] = d.vtx_count + 1;
    index_data[d.total_idx_count++] = d.vtx_count + 2;
    index_data[d.total_idx_count++] = d.vtx_count + 2;
    index_data[d.total_idx_count++] = d.vtx_count + 3;
    index_data[d.total_idx_count++] = d.vtx_count + 0;
    
    vertex_data[d.total_vtx_count++] = {{rect.x.low , rect.y.low }, c_top_left};
    vertex_data[d.total_vtx_count++] = {{rect.x.low , rect.y.high}, c_bot_left};
    vertex_data[d.total_vtx_count++] = {{rect.x.high, rect.y.high}, c_bot_right};
    vertex_data[d.total_vtx_count++] = {{rect.x.high, rect.y.low }, c_top_right};

    d.idx_count += 6;
    d.vtx_count += 4;
}

void Renderer_2D::add_rect_outline(rf32 rect, float thick, Border_Style style, color c) {
    for (int i = 0; i < 8; i++) {
    // I bet you've never seen code like this:
        i & 0x1 ?
        (
            index_data[d.total_idx_count++] = d.vtx_count + i,
            index_data[d.total_idx_count++] = d.vtx_count + ( i + 1u ) % 8u,
            index_data[d.total_idx_count++] = d.vtx_count + ( i + 2u ) % 8u
        ):(
            index_data[d.total_idx_count++] = d.vtx_count + i,
            index_data[d.total_idx_count++] = d.vtx_count + ( i + 2u ) % 8u,
            index_data[d.total_idx_count++] = d.vtx_count + ( i + 1u ) % 8u
        );
    }
    float in_l = rect.x.low,  out_l = in_l;
    float in_r = rect.x.high, out_r = in_r;
    float in_t = rect.y.low,  out_t = in_t;
    float in_b = rect.y.high, out_b = in_b;
    float half = thick * 0.5f;

    switch (style)
    {
    default: case BORDER_CENTER:
        out_l -= half, out_t -= half;
        out_r += half, out_b += half;
         in_l += half,  in_t += half;
         in_r -= half,  in_b -= half;
    break; case BORDER_INSIDE:
        in_l += thick, in_t += thick;
        in_r -= thick, in_b -= thick;
    break; case BORDER_OUTSIDE:
        out_l -= thick, out_t -= thick;
        out_r += thick, out_b += thick;
    break;
    }

    vertex_data[d.total_vtx_count++] = {{out_l, out_b}, c};
    vertex_data[d.total_vtx_count++] = {{ in_l,  in_b}, c};
    vertex_data[d.total_vtx_count++] = {{out_l, out_t}, c};
    vertex_data[d.total_vtx_count++] = {{ in_l,  in_t}, c};
    vertex_data[d.total_vtx_count++] = {{out_r, out_t}, c};
    vertex_data[d.total_vtx_count++] = {{ in_r,  in_t}, c};
    vertex_data[d.total_vtx_count++] = {{out_r, out_b}, c};
    vertex_data[d.total_vtx_count++] = {{ in_r,  in_b}, c};

    d.idx_count += 24;
    d.vtx_count += 8;
}

void Renderer_2D::add_line(v2f32 start, v2f32 end, float stroke, color startColor, color endColor)
{
    const auto edge_vector = (end - start).perp().norm() * stroke / 2.0f;

    index_data[d.total_idx_count++] = d.vtx_count + 0u;
    index_data[d.total_idx_count++] = d.vtx_count + 1u;
    index_data[d.total_idx_count++] = d.vtx_count + 2u;
    index_data[d.total_idx_count++] = d.vtx_count + 2u;
    index_data[d.total_idx_count++] = d.vtx_count + 1u;
    index_data[d.total_idx_count++] = d.vtx_count + 3u;

    vertex_data[d.total_vtx_count++] = {(start + edge_vector), startColor};
    vertex_data[d.total_vtx_count++] = {(start - edge_vector), startColor};
    vertex_data[d.total_vtx_count++] = {(end   + edge_vector), endColor};
    vertex_data[d.total_vtx_count++] = {(end   - edge_vector), endColor};

    d.idx_count += 6;
    d.vtx_count += 4;
}

void Renderer_2D::add_circle(v2f32 position, float radius, color color) {
    int vtx_count = math::min(math::max(int(radius)*2, 10), 64);

    for (int i = 0; i < vtx_count-2; ++i) {
        index_data[d.total_idx_count++] = d.vtx_count;
        index_data[d.total_idx_count++] = d.vtx_count + i + 1;
        index_data[d.total_idx_count++] = d.vtx_count + i + 2;
    }

    for (int i = 0; i < vtx_count; ++i) {
        float t = float(FS_TAU) * float(i) / float(vtx_count);
        vertex_data[d.total_vtx_count++] = {position + v2f32(cosf(t), sinf(t))*radius, color};
    }

    d.vtx_count += vtx_count;
    d.idx_count += (vtx_count - 2) * 3;
}

void Renderer_2D::add_circle_outline(v2f32 position, float radius, float inner_radius, color c_out, color c_in)
{
    int vtx_count = math::min(math::max(int(radius)*2, 10), 64);

    for (int i = 0; i < vtx_count-1; ++i) {
        index_data[d.total_idx_count++] = d.vtx_count + i*2 + 0;
        index_data[d.total_idx_count++] = d.vtx_count + i*2 + 1;
        index_data[d.total_idx_count++] = d.vtx_count + i*2 + 2;

        index_data[d.total_idx_count++] = d.vtx_count + i*2 + 1;
        index_data[d.total_idx_count++] = d.vtx_count + i*2 + 3;
        index_data[d.total_idx_count++] = d.vtx_count + i*2 + 2;
    }

    index_data[d.total_idx_count++] = d.vtx_count + (vtx_count-1)*2 + 0;
    index_data[d.total_idx_count++] = d.vtx_count + (vtx_count-1)*2 + 1;
    index_data[d.total_idx_count++] = d.vtx_count + 0;

    index_data[d.total_idx_count++] = d.vtx_count + (vtx_count-1)*2 + 1;
    index_data[d.total_idx_count++] = d.vtx_count + 1;
    index_data[d.total_idx_count++] = d.vtx_count + 0;

    for (int i = 0; i < vtx_count; ++i) {
        float t = float(FS_TAU) * float(i) / float(vtx_count);
        vertex_data[d.total_vtx_count++] = {position + v2f32(cosf(t), sinf(t))*radius, c_out};
        vertex_data[d.total_vtx_count++] = {position + v2f32(cosf(t), sinf(t))*inner_radius, c_in};
    }

    d.vtx_count += vtx_count * 2;
    d.idx_count += vtx_count * 6;
}

void Textured_Renderer_2D::add_rect(rf32 const& rect, rf32 const& uv, color color) {
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

void Textured_Renderer_2D::add_glyph(Glyph const *g, v2f32 origin, float scale, color color)
{
    index_data[d.total_idx_count++] = d.vtx_count;
    index_data[d.total_idx_count++] = d.vtx_count + 1u;
    index_data[d.total_idx_count++] = d.vtx_count + 2u;
    index_data[d.total_idx_count++] = d.vtx_count + 3u;
    index_data[d.total_idx_count++] = d.vtx_count;
    index_data[d.total_idx_count++] = d.vtx_count + 2u;

    const auto rect = rf32 {
        roundf(origin.x + scale * g->rc.x.low),
        roundf(origin.x + scale * g->rc.x.high),
        roundf(origin.y + scale * g->rc.y.low),
        roundf(origin.y + scale * g->rc.y.high),
    };

    vertex_data[d.total_vtx_count++] = {{rect.x.low,  rect.y.high}, {g->uv.x.low,  g->uv.y.high}, color};
    vertex_data[d.total_vtx_count++] = {{rect.x.low,  rect.y.low }, {g->uv.x.low,  g->uv.y.low }, color};
    vertex_data[d.total_vtx_count++] = {{rect.x.high, rect.y.low }, {g->uv.x.high, g->uv.y.low }, color};
    vertex_data[d.total_vtx_count++] = {{rect.x.high, rect.y.high}, {g->uv.x.high, g->uv.y.high}, color};

    d.idx_count += 6;
    d.vtx_count += 4;
}

v2f32 Textured_Renderer_2D::add_string(string str, v2f32 top_left, color col) {
        (void)col;
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

v2f32 Textured_Renderer_2D::add_string_rtl(string str, v2f32 top_right, color col) {
        (void)col;
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

FISSION_NAMESPACE_END
