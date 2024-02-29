#include <Fission/Core/Renderer_2D.hh>
#include <Fission/Core/Engine.hh>
#include <Fission/Base/Memory.hpp>

extern fs::Engine engine;

__FISSION_BEGIN__

struct solid_color_vs {
#include "BinaryShaders/solid_color.vert.inl"
};
struct solid_color_fs {
#include "BinaryShaders/solid_color.frag.inl"
};
struct textured_2d_vs {
#include "BinaryShaders/textured_2d.vert.inl"
};
struct textured_2d_fs {
#include "BinaryShaders/textured_2d.frag.inl"
};

void Renderer_2D::create(Graphics* gfx, VkRenderPass render_pass, Transform_2D_Layout layout)
{
	max_vertex_count = _r2d_max_count;
	max_index_count = _r2d_max_count * 2;

	bump_allocator allocator{max_vertex_count * sizeof(vertex) + max_index_count * sizeof(u16)};
	vertex_data = allocator.alloc<vertex>(max_vertex_count);
	index_data  = allocator.alloc<u16>(max_index_count);
	allocator.release();

	frame_data[0] = FD(gfx->allocator, max_vertex_count, max_index_count);
	frame_data[1] = FD(gfx->allocator, max_vertex_count, max_index_count);
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
		pipelineLayoutInfo.pSetLayouts = &layout;
		pipelineLayoutInfo.setLayoutCount = 1;
		vkCreatePipelineLayout(gfx->device, &pipelineLayoutInfo, nullptr, &pipeline_layout);
	}
	auto vertex_input = vk::Basic_Vertex_Input<v2f32, color>{};
	Pipeline_Create_Info pipeline_info;
	pipeline_info.device = gfx->device;
	pipeline_info.vertex_shader = create_shader(gfx->device, solid_color_vs::size, solid_color_vs::data);
	pipeline_info.fragment_shader = create_shader(gfx->device, solid_color_fs::size, solid_color_fs::data);
	pipeline_info.pipeline_layout = pipeline_layout;
	pipeline_info.blend_mode = Blend_Mode_Normal;
	pipeline_info.render_pass = render_pass;
	pipeline_info.vertex_input = &vertex_input;
	create_pipeline(pipeline_info, &pipeline);

	frag = pipeline_info.fragment_shader;
	vert = pipeline_info.vertex_shader;
}

void Renderer_2D::end_render(Render_Context const* ctx) {
	if (d.total_vtx_count) {
		auto& fd = frame_data[ctx->frame];
		fd.set_data(ctx->gfx, vertex_data, index_data, d.total_vtx_count, d.total_idx_count);
//		engine.debug_layer.add("v: %u, i: %u", d.total_vtx_count, d.total_idx_count);
		d.reset();
	}
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

void Textured_Renderer_2D::create(
	Graphics* gfx,
	VkRenderPass render_pass,
	Transform_2D_Layout transform_layout,
	Texture_Layout texture_layout
) {
	max_vertex_count = _r2d_max_count;
	max_index_count = _r2d_max_count * 2;
	
	bump_allocator allocator{max_vertex_count * sizeof(vertex) + max_index_count * sizeof(u16)};
	vertex_data = allocator.alloc<vertex>(max_vertex_count);
	index_data  = allocator.alloc<u16>(max_index_count);
	allocator.release();

	frame_data[0] = FD(gfx->allocator, max_vertex_count, max_index_count);
	frame_data[1] = FD(gfx->allocator, max_vertex_count, max_index_count);

	{
		VkDescriptorSetLayout layouts[2] = {transform_layout, texture_layout};
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
		pipelineLayoutInfo.pSetLayouts = layouts;
		pipelineLayoutInfo.setLayoutCount = 2;
		vkCreatePipelineLayout(gfx->device, &pipelineLayoutInfo, nullptr, &pipeline_layout);
	}
	auto vertex_input = vk::Basic_Vertex_Input<v2f32, v2f32, color>{};
	Pipeline_Create_Info pipeline_info;
	pipeline_info.device = gfx->device;
	pipeline_info.vertex_shader = create_shader(gfx->device, textured_2d_vs::size, textured_2d_vs::data);
	pipeline_info.fragment_shader = create_shader(gfx->device, textured_2d_fs::size, textured_2d_fs::data);
	pipeline_info.pipeline_layout = pipeline_layout;
	pipeline_info.blend_mode = Blend_Mode_Normal;
	pipeline_info.render_pass = render_pass;
	pipeline_info.vertex_input = &vertex_input;
	create_pipeline(pipeline_info, &pipeline);

	frag = pipeline_info.fragment_shader;
	vert = pipeline_info.vertex_shader  ;
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

__FISSION_END__
