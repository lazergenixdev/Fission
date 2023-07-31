#include <Fission/Core/Renderer_2D.hh>

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
	device = gfx->device,
	allocator = gfx->allocator,
	max_vertex_count = _r2d_max_count,
	max_index_count = _r2d_max_count * 2,

	// TODO: only one malloc needed
	vertex_data = (vertex*)_aligned_malloc(max_vertex_count * sizeof(vertex), 32);
	index_data = (u16*)_aligned_malloc(max_index_count * sizeof(u16), 32);

	frame_data[0] = Frame_Data(gfx->allocator, max_vertex_count, max_index_count);
	frame_data[1] = Frame_Data(gfx->allocator, max_vertex_count, max_index_count);

	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	VkVertexInputAttributeDescription attributeDescriptions[2];
	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(vertex, position);
	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(vertex, color);
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = (u32)std::size(attributeDescriptions);
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions;
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
		pipelineLayoutInfo.pSetLayouts = &layout;
		pipelineLayoutInfo.setLayoutCount = 1;
		vkCreatePipelineLayout(gfx->device, &pipelineLayoutInfo, nullptr, &pipeline_layout);
	}
	Pipeline_Create_Info pipeline_info;
	pipeline_info.device = gfx->device;
	pipeline_info.vertex_shader = create_shader(gfx->device, solid_color_vs::size, solid_color_vs::data);
	pipeline_info.fragment_shader = create_shader(gfx->device, solid_color_fs::size, solid_color_fs::data);
	pipeline_info.pipeline_layout = pipeline_layout;
	pipeline_info.blend_mode = Blend_Mode_Normal;
	pipeline_info.render_pass = render_pass;
	pipeline_info.vertex_input = &vertexInputInfo;
	create_pipeline(pipeline_info, &pipeline);

	vkDestroyShaderModule(gfx->device, pipeline_info.fragment_shader, nullptr);
	vkDestroyShaderModule(gfx->device, pipeline_info.vertex_shader, nullptr);
}

void Textured_Renderer_2D::create(
	Graphics* gfx,
	VkRenderPass render_pass,
	Transform_2D_Layout transform_layout,
	Texture_Layout texture_layout
) {
	device = gfx->device;
	allocator = gfx->allocator;

	max_vertex_count = _r2d_max_count;
	max_index_count = _r2d_max_count * 2;

	vertex_data = (vertex*)_aligned_malloc(max_vertex_count * sizeof(vertex), 32);
	index_data = (u16*)_aligned_malloc(max_index_count * sizeof(u16), 32);

	frame_data[0] = Frame_Data(gfx->allocator, max_vertex_count, max_index_count);
	frame_data[1] = Frame_Data(gfx->allocator, max_vertex_count, max_index_count);

	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	VkVertexInputAttributeDescription attributeDescriptions[3];
	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(vertex, position);
	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(vertex, texcoord);
	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(vertex, color);
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = (u32)std::size(attributeDescriptions);
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions;
	{
		VkDescriptorSetLayout layouts[2] = {transform_layout, texture_layout};
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
		pipelineLayoutInfo.pSetLayouts = layouts;
		pipelineLayoutInfo.setLayoutCount = 2;
		vkCreatePipelineLayout(gfx->device, &pipelineLayoutInfo, nullptr, &pipeline_layout);
	}
	Pipeline_Create_Info pipeline_info;
	pipeline_info.device = gfx->device;
	pipeline_info.vertex_shader = create_shader(gfx->device, textured_2d_vs::size, textured_2d_vs::data);
	pipeline_info.fragment_shader = create_shader(gfx->device, textured_2d_fs::size, textured_2d_fs::data);
	pipeline_info.pipeline_layout = pipeline_layout;
	pipeline_info.blend_mode = Blend_Mode_Normal;
	pipeline_info.render_pass = render_pass;
	pipeline_info.vertex_input = &vertexInputInfo;
	create_pipeline(pipeline_info, &pipeline);

	vkDestroyShaderModule(gfx->device, pipeline_info.fragment_shader, nullptr);
	vkDestroyShaderModule(gfx->device, pipeline_info.vertex_shader, nullptr);
}

__FISSION_END__
