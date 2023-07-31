#pragma once
#include <Fission/Core/Engine.hh>
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
extern fs::Engine engine;

struct r3d_vs {
#include "3d.vert.inl"
};
struct r3d_fs {
#include "3d.frag.inl"
};
struct post_vs {
#include "post.vert.inl"
};
struct post_fs {
#include "post.frag.inl"
};

#define USE_SUBPASS 1

struct Render_Pass_3D {
	VkRenderPass handle;
	
	inline constexpr operator VkRenderPass() const { return handle; }

	void begin(fs::Render_Context* ctx, VkFramebuffer frame_buffer, fs::color clear) {
		VkClearValue clear_values[2];
		clear_values[0].color = {clear.r, clear.g, clear.b, clear.a};
		clear_values[1].depthStencil = { 1.0f, 0 };
		VkRenderPassBeginInfo beginInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		beginInfo.framebuffer = frame_buffer;
		beginInfo.renderPass = handle;
		beginInfo.clearValueCount = 2;
		beginInfo.pClearValues = clear_values;
		beginInfo.renderArea.extent = ctx->gfx->sc_extent;
		beginInfo.renderArea.offset = { 0, 0 };
		vkCmdBeginRenderPass(ctx->command_buffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
	}
	void end(fs::Render_Context* ctx) {
		vkCmdEndRenderPass(ctx->command_buffer);
	}

	void create(fs::Graphics& gfx) {
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format         = gfx.sc_format;
		colorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depthAttachment{};
		depthAttachment.format         = VK_FORMAT_D32_SFLOAT;
		depthAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

#if USE_SUBPASS
		VkAttachmentReference depthReadAttachmentRef{};
		depthReadAttachmentRef.attachment = 1;
		depthReadAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
#endif

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

#if USE_SUBPASS
		VkSubpassDescription subpass1{};
		subpass1.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass1.colorAttachmentCount = 1;
		subpass1.pColorAttachments = &colorAttachmentRef;
		subpass1.pDepthStencilAttachment = nullptr;
		subpass1.inputAttachmentCount = 1;
		subpass1.pInputAttachments = &depthReadAttachmentRef;
#endif

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

#if USE_SUBPASS
		VkSubpassDependency dependency1{};
		dependency1.srcSubpass = 0;
		dependency1.dstSubpass = 1;
		dependency1.srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependency1.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependency1.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependency1.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
#endif
		
		auto attachments = { colorAttachment, depthAttachment };
#if USE_SUBPASS
		auto dependencies = { dependency, dependency1 };
		auto subpasses = { subpass, subpass1 };
#endif

		VkRenderPassCreateInfo renderPassInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
		renderPassInfo.attachmentCount = (fs::u32)attachments.size();
		renderPassInfo.pAttachments = attachments.begin();
#if USE_SUBPASS
		renderPassInfo.subpassCount = (fs::u32)subpasses.size();
		renderPassInfo.pSubpasses = subpasses.begin();
		renderPassInfo.dependencyCount = (fs::u32)dependencies.size();
		renderPassInfo.pDependencies = dependencies.begin();
#else
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;
#endif

		vkCreateRenderPass(gfx.device, &renderPassInfo, nullptr, &handle);
	}
	void destroy(fs::Graphics& gfx) {
		vkDestroyRenderPass(gfx.device, handle, nullptr);
	}
};

struct Pipeline_Create_Info {
	VkDevice device;
	VkRenderPass render_pass;
	VkPipelineLayout pipeline_layout;
	VkShaderModule vertex_shader;
	VkShaderModule fragment_shader;
	VkPipelineVertexInputStateCreateInfo const* vertex_input;
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
	dynamicStateInfo.dynamicStateCount = (fs::u32)std::size(dynamicStates);
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
	rasterizer.cullMode = 1?VK_CULL_MODE_NONE:VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = 0b1111;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	
	VkPipelineDepthStencilStateCreateInfo depthState{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
	depthState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthState.depthTestEnable = VK_TRUE;
	depthState.depthWriteEnable = VK_TRUE;
	depthState.depthCompareOp = VK_COMPARE_OP_LESS;
	depthState.depthBoundsTestEnable = VK_FALSE;
	depthState.stencilTestEnable = VK_FALSE;
	depthState.front = {}; // Optional
	depthState.back = {}; // Optional
	
	VkGraphicsPipelineCreateInfo pipelineInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
	pipelineInfo.stageCount = (fs::u32)std::size(shaderStages);
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = createInfo.vertex_input;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthState;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicStateInfo;
	pipelineInfo.layout = createInfo.pipeline_layout;
	pipelineInfo.renderPass = createInfo.render_pass;
	pipelineInfo.subpass = createInfo.subpass;

	vkCreateGraphicsPipelines(createInfo.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, outPipeline);
}

namespace vk {
	// extent format mapping to include GLM
	template <>	struct _format_of<glm::vec4> { static constexpr VkFormat value = VK_FORMAT_R32G32B32A32_SFLOAT; };
	template <>	struct _format_of<glm::vec3> { static constexpr VkFormat value = VK_FORMAT_R32G32B32_SFLOAT; };
	template <>	struct _format_of<glm::vec2> { static constexpr VkFormat value = VK_FORMAT_R32G32_SFLOAT; };
}

struct Renderer_3D {
	struct vertex {
		glm::vec3 position;
	};

	struct Transform {
		glm::mat4 projection;
		glm::mat4 view;
		glm::mat4 model;
	};

	struct Shader_Constant_Data {
		glm::vec4 position;
		glm::vec4 color;
		glm::vec4 time;
	};

	/*
		(-1, 1)   ( 1, 1)

		(-1,-1)   ( 1,-1)
	*/
	static constexpr vertex vertex_data[] = {
#if 0
		// X+
		vertex{{ 1,-1,-1}},
		vertex{{ 1, 1,-1}},
		vertex{{ 1,-1, 1}},
		vertex{{ 1, 1, 1}},
		vertex{{ 1,-1, 1}},
		vertex{{ 1, 1,-1}},

		// X-
		vertex{{-1, 1,-1}},
		vertex{{-1,-1,-1}},
		vertex{{-1,-1, 1}},
		vertex{{-1,-1, 1}},
		vertex{{-1, 1, 1}},
		vertex{{-1, 1,-1}},

		// Y+
		vertex{{ 1, 1,-1}},
		vertex{{-1, 1,-1}},
		vertex{{-1, 1, 1}},
		vertex{{-1, 1, 1}},
		vertex{{ 1, 1, 1}},
		vertex{{ 1, 1,-1}},

		// Y-
		vertex{{-1,-1,-1}},
		vertex{{ 1,-1,-1}},
		vertex{{-1,-1, 1}},
		vertex{{ 1,-1, 1}},
		vertex{{-1,-1, 1}},
		vertex{{ 1,-1,-1}},

		// Z+
		vertex{{-1,-1, 1}},
		vertex{{ 1,-1, 1}},
		vertex{{-1, 1, 1}},
		vertex{{ 1, 1, 1}},
		vertex{{-1, 1, 1}},
		vertex{{ 1,-1, 1}},

		// Z-
		vertex{{ 1,-1,-1}},
		vertex{{-1,-1,-1}},
		vertex{{-1, 1,-1}},
		vertex{{-1, 1,-1}},
		vertex{{ 1, 1,-1}},
		vertex{{ 1,-1,-1}},
#elif 1
#define _V -100000
#define R 0.5
#define LONG_BOI(X,Y,A,B) \
		vertex{{ R + (X), R*B + (Y), 0}}, \
		vertex{{ R + (X), R*A + (Y), 0}}, \
		vertex{{ R + (X), R*B + (Y),_V}}, \
		vertex{{ R + (X), R*A + (Y),_V}}, \
		vertex{{ R + (X), R*B + (Y),_V}}, \
		vertex{{ R + (X), R*A + (Y), 0}}, \
		vertex{{-R + (X), R*A + (Y), 0}}, \
		vertex{{-R + (X), R*B + (Y), 0}}, \
		vertex{{-R + (X), R*B + (Y),_V}}, \
		vertex{{-R + (X), R*B + (Y),_V}}, \
		vertex{{-R + (X), R*A + (Y),_V}}, \
		vertex{{-R + (X), R*A + (Y), 0}}, \
		vertex{{ R + (X), R*A + (Y), 0}}, \
		vertex{{-R + (X), R*A + (Y), 0}}, \
		vertex{{-R + (X), R*A + (Y),_V}}, \
		vertex{{-R + (X), R*A + (Y),_V}}, \
		vertex{{ R + (X), R*A + (Y),_V}}, \
		vertex{{ R + (X), R*A + (Y), 0}}, \
		vertex{{-R + (X), R*B + (Y), 0}}, \
		vertex{{ R + (X), R*B + (Y), 0}}, \
		vertex{{-R + (X), R*B + (Y),_V}}, \
		vertex{{ R + (X), R*B + (Y),_V}}, \
		vertex{{-R + (X), R*B + (Y),_V}}, \
		vertex{{ R + (X), R*B + (Y), 0}}, \
		vertex{{ R + (X), R*B + (Y), 0}}, \
		vertex{{-R + (X), R*B + (Y), 0}}, \
		vertex{{-R + (X), R*A + (Y), 0}}, \
		vertex{{-R + (X), R*A + (Y), 0}}, \
		vertex{{ R + (X), R*A + (Y), 0}}, \
		vertex{{ R + (X), R*B + (Y), 0}},

		LONG_BOI(0, -1, 1, -100)
		LONG_BOI(0,  1, 100, -1)
	//	LONG_BOI(  0, -1.5 - 1, 1, -100)
	//	LONG_BOI(  0,  1.5 - 1.5, 100, -1)
	//	LONG_BOI( 10, -1.5 + 1.5, 1, -100)
	//	LONG_BOI( 10,  1.5 + 1, 100, -1)
#else
		vertex{{ 1,-1, 0}},
		vertex{{-1,-1, 0}},
		vertex{{-1, 1, 0}},
		vertex{{-1, 1, 0}},
		vertex{{ 1, 1, 0}},
		vertex{{ 1,-1, 0}},
#endif
	};

	static constexpr fs::v4f32 quad_vertices[] = {
		{-1.0f, 1.0f, 0.0f, 1.0f},
		{-1.0f,-1.0f, 0.0f, 0.0f},
		{ 1.0f,-1.0f, 1.0f, 0.0f},
		{ 1.0f,-1.0f, 1.0f, 0.0f},
		{ 1.0f, 1.0f, 1.0f, 1.0f},
		{-1.0f, 1.0f, 0.0f, 1.0f},
	};

	struct Transform_Data {
		VkBuffer        buffers[2];
		VmaAllocation   allocations[2];
		VkDescriptorSet sets[2];
		fs::Single_Descriptor_Set_Layout<VK_SHADER_STAGE_VERTEX_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER>
			transform_layout;

		void create(fs::Graphics& gfx) {
			VmaAllocationCreateInfo allocInfo = {};
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
			allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
			VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
			bufferInfo.size = sizeof(Transform);
			bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			FS_FOR(2) {
				vmaCreateBuffer(gfx.allocator, &bufferInfo, &allocInfo, buffers + i, allocations + i, nullptr);
			}
			transform_layout.create(gfx);
			VkDescriptorSetLayout layouts[2] = { transform_layout, transform_layout };
			VkDescriptorSetAllocateInfo descSetAllocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
			descSetAllocInfo.descriptorPool = engine.descriptor_pool;
			descSetAllocInfo.pSetLayouts = layouts;
			descSetAllocInfo.descriptorSetCount = 2;
			vkAllocateDescriptorSets(gfx.device, &descSetAllocInfo, sets);

			{
				VkDescriptorBufferInfo bufferInfo;
				bufferInfo.offset = 0;
				bufferInfo.range = sizeof(Transform);
				VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
				write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				write.descriptorCount = 1;
				write.dstBinding = 0;
				write.pBufferInfo = &bufferInfo;
				FS_FOR(2) {
					bufferInfo.buffer = buffers[i];
					write.dstSet = sets[i];
					vkUpdateDescriptorSets(gfx.device, 1, &write, 0, nullptr);
				}
			}
		}
		void destroy(fs::Graphics& gfx) {
			FS_FOR(2) {
				vmaDestroyBuffer(gfx.allocator, buffers[i], allocations[i]);
			}
			transform_layout.destroy(gfx);
		}

		void set_data(fs::Render_Context* ctx, Transform& transform) {
			void* gpu_data;
			vmaMapMemory(ctx->gfx->allocator, allocations[ctx->frame], &gpu_data);
			memcpy(gpu_data, &transform, sizeof(transform));
			vmaUnmapMemory(ctx->gfx->allocator, allocations[ctx->frame]);
			vmaFlushAllocation(ctx->gfx->allocator, allocations[ctx->frame], 0, VK_WHOLE_SIZE);
		}
	};

	void recreate_framebuffers(fs::Graphics& gfx, VkRenderPass render_pass) {
		FS_FOR(gfx.sc_image_count) vkDestroyFramebuffer(gfx.device, frame_buffers[i], nullptr);
		
		VkImageView attachments[2] = { nullptr, depth_image_view };
		FS_FOR(gfx.sc_image_count) {
			attachments[0] = gfx.sc_image_views[i];
			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = render_pass;
			framebufferInfo.attachmentCount = (uint32_t)std::size(attachments);
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width  = gfx.sc_extent.width;
			framebufferInfo.height = gfx.sc_extent.height;
			framebufferInfo.layers = 1;
			vkCreateFramebuffer(gfx.device, &framebufferInfo, nullptr, frame_buffers + i);
		}
	}

	void create(fs::Graphics& gfx, VkRenderPass render_pass, VkRenderPass post_render_pass) {
		vertex_count = (fs::u32)std::size(vertex_data);
		frame_transform.create(gfx);

		VkPushConstantRange push_ranges[1];
		push_ranges[0].offset = 0;
		push_ranges[0].size = sizeof(Shader_Constant_Data);
		push_ranges[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT;
		VkPipelineLayoutCreateInfo pipeline_info{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
		pipeline_info.pPushConstantRanges = push_ranges;
		pipeline_info.pushConstantRangeCount = (uint32_t)std::size(push_ranges);
		pipeline_info.pSetLayouts = &frame_transform.transform_layout;
		pipeline_info.setLayoutCount = 1;
		vkCreatePipelineLayout(gfx.device, &pipeline_info, nullptr, &pipeline_layout);

		Pipeline_Create_Info pi;
		pi.device          = gfx.device;
		pi.pipeline_layout = pipeline_layout;
		pi.render_pass     = render_pass;
		pi.vertex_shader   = fs::create_shader(gfx.device, r3d_vs::size, r3d_vs::data);
		pi.fragment_shader = fs::create_shader(gfx.device, r3d_fs::size, r3d_fs::data);

		auto vi = vk::Vertex_Input<glm::vec3>{};
		pi.vertex_input = &vi;

		create_pipeline(pi, &pipeline);
		
		vkDestroyShaderModule(gfx.device, pi.vertex_shader,   nullptr);
		vkDestroyShaderModule(gfx.device, pi.fragment_shader, nullptr);

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
		VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		bufferInfo.size = vertex_count * sizeof(vertex);
		bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT|VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		vmaCreateBuffer(gfx.allocator, &bufferInfo, &allocInfo, &vertex_buffer, &vertex_allocation, nullptr);
		gfx.upload_buffer(vertex_buffer, vertex_data, vertex_count * sizeof(vertex));

		{
			VmaAllocationCreateInfo allocInfo = {};
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
			VkImageCreateInfo imageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
			imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
			imageInfo.arrayLayers = 1;
			imageInfo.extent = { .width = gfx.sc_extent.width, .height = gfx.sc_extent.height, .depth = 1 };
			imageInfo.format = VK_FORMAT_D32_SFLOAT;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.mipLevels = 1;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			vmaCreateImage(gfx.allocator, &imageInfo, &allocInfo, &depth_image, &depth_allocation, nullptr);

			auto viewInfo = vk::image_view_2d(depth_image, imageInfo.format, VK_IMAGE_ASPECT_DEPTH_BIT);
			vkCreateImageView(gfx.device, &viewInfo, nullptr, &depth_image_view);
		}

		VkImageView attachments[2] = { nullptr, depth_image_view };
		FS_FOR(gfx.sc_image_count) {
			attachments[0] = gfx.sc_image_views[i];
			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = render_pass;
			framebufferInfo.attachmentCount = (uint32_t)std::size(attachments);
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width  = gfx.sc_extent.width;
			framebufferInfo.height = gfx.sc_extent.height;
			framebufferInfo.layers = 1;
			vkCreateFramebuffer(gfx.device, &framebufferInfo, nullptr, frame_buffers + i);
		}

		// POST FX
		{
			VkPipelineLayoutCreateInfo pipeline_info{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
			pipeline_info.pSetLayouts = &engine.texture_layout;
			pipeline_info.setLayoutCount = 1;
			vkCreatePipelineLayout(gfx.device, &pipeline_info, nullptr, &post_pipeline_layout);

			fs::Pipeline_Create_Info pi;
			pi.device = gfx.device;
			pi.pipeline_layout = post_pipeline_layout;
			pi.vertex_shader   = fs::create_shader(gfx.device, post_vs::size, post_vs::data);
			pi.fragment_shader = fs::create_shader(gfx.device, post_fs::size, post_fs::data);
			pi.blend_mode = fs::Blend_Mode_Add;
#if USE_SUBPASS
			pi.render_pass = render_pass;
			pi.subpass = 1;
#else
			pi.render_pass = post_render_pass;
#endif
			auto vi = vk::Vertex_Input<fs::v4f32>{};
			pi.vertex_input = &vi;
			fs::create_pipeline(pi, &post_pipeline);

			vkDestroyShaderModule(gfx.device, pi.vertex_shader,   nullptr);
			vkDestroyShaderModule(gfx.device, pi.fragment_shader, nullptr);

			VmaAllocationCreateInfo allocInfo = {};
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
			VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
			bufferInfo.size = sizeof(quad_vertices);
			bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT|VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			vmaCreateBuffer(gfx.allocator, &bufferInfo, &allocInfo, &quad_vertex_buffer, &quad_vertex_allocation, nullptr);
			gfx.upload_buffer(quad_vertex_buffer, quad_vertices, sizeof(quad_vertices));

			auto samplerInfo = vk::sampler(VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
			vkCreateSampler(gfx.device, &samplerInfo, nullptr, &sampler);


			VkDescriptorSetLayout layouts[1] = { engine.texture_layout };
			VkDescriptorSetAllocateInfo descSetAllocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
			descSetAllocInfo.descriptorPool = engine.descriptor_pool;
			descSetAllocInfo.pSetLayouts = layouts;
			descSetAllocInfo.descriptorSetCount = 1;
			vkAllocateDescriptorSets(gfx.device, &descSetAllocInfo, &depth_set);

			VkDescriptorImageInfo imageInfo;
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
			imageInfo.imageView = depth_image_view;
			imageInfo.sampler = sampler;
			VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
			write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write.descriptorCount = 1;
			write.dstBinding = 0;
			write.dstSet = depth_set;
			write.pImageInfo = &imageInfo;
			vkUpdateDescriptorSets(gfx.device, 1, &write, 0, nullptr);
		}
	}

	VkFramebuffer get_frame_buffer(fs::Render_Context* ctx) {
		return frame_buffers[ctx->image_index];
	}

	void destroy(fs::Graphics& gfx) {
		vkDestroyPipelineLayout(gfx.device, post_pipeline_layout, nullptr);
		vkDestroyPipelineLayout(gfx.device, pipeline_layout, nullptr);
		vkDestroyPipeline(gfx.device, post_pipeline, nullptr);
		vkDestroyPipeline(gfx.device, pipeline, nullptr);
		vkDestroySampler(gfx.device, sampler, nullptr);
		vmaDestroyBuffer(gfx.allocator, quad_vertex_buffer, quad_vertex_allocation);
		vmaDestroyBuffer(gfx.allocator, vertex_buffer, vertex_allocation);
		frame_transform.destroy(gfx);
		vmaDestroyImage(gfx.allocator, depth_image, depth_allocation);
		vkDestroyImageView(gfx.device, depth_image_view, nullptr);
		FS_FOR(gfx.sc_image_count) {
			vkDestroyFramebuffer(gfx.device, frame_buffers[i], nullptr);
		}
	}

	void draw(fs::Render_Context* ctx, float dt) {
		vkCmdBindPipeline(ctx->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(ctx->command_buffer, 0, 1, &vertex_buffer, &offset);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width  = static_cast<float>(ctx->gfx->sc_extent.width);
		viewport.height = static_cast<float>(ctx->gfx->sc_extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(ctx->command_buffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = ctx->gfx->sc_extent;
		vkCmdSetScissor(ctx->command_buffer, 0, 1, &scissor);
		
		//make a model view matrix for rendering the object
		//camera position
		float off = sinf(t*2.5f)*0.5f;
		glm::vec3 camPos = { -t*2.0f + off*3.6f,0.f,-2.0f};
	//	engine.debug_layer.add("pos = %.3f", camPos.x);

		glm::mat4 view = glm::translate(glm::mat4(1.f), camPos);
//		view = glm::lookAt(-camPos, glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));

#undef near
#undef far
		float fovy_radians = glm::half_pi<float>();
		float fovx_radians = glm::half_pi<float>();

		auto camera_setAspectRatio = [&](float aspect, float zNear, float zFar, bool offaxis)
		{
			// create a projection matrix
			const float f = 1.0f / tanf(fovy_radians / 2.0f);
			fovx_radians = 2.0f * atanf(aspect * tanf(fovy_radians / 2.0f));
			glm::mat4 m;

			m[0] = { f / aspect,
			0.0f,
			0.0f,
			0.0f };

			m[1] = { 0.0f,
			f,
			0.0f,
			 0.0f };

			m[2] = { off,
			0.0f, // off axis projection!
			 (zFar + zNear) / (zNear - zFar),
			 -1.0f };

			m[3] = { 0.0f,
				0.0f,
				2 * zFar * zNear / (zNear - zFar),
				0.0f };
			return m;
		};

		//camera projection
	//	glm::mat4 projection = glm::perspectiveFov(glm::half_pi<float>(), 160.0f, 90.0f, 0.5f, 100000.0f);
		glm::mat4 projection = camera_setAspectRatio(128.0f/72.0f, 1.0f, 100000.0f, true);
	//	projection[1][1] *= -1;
		//model rotation
		t += dt;
		glm::mat4 model = glm::rotate(glm::mat4{1.0f}, t, glm::vec3(0, 1, 0));
		         model *= glm::rotate(glm::mat4{1.0f}, t*0.32678f, glm::vec3(1, 0, 0));

	//	projection[1][0] += sinf(t * 1.2F) * 0.1F;
	//	projection[0][1] += sinf(t * 1.5F) * 0.1F;

		//calculate final mesh matrix
		Transform transform;
		transform.projection = projection;
		transform.view       = 0?glm::mat4{1.f}:view;
		transform.model      = 1?glm::mat4{1.f}:model;
		frame_transform.set_data(ctx, transform);

		Shader_Constant_Data info;
		info.color.a = 1.0f;
		info.time.y = 0.1f;//1.5f*fs::math::pi<float>;

		auto stage = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT;
		vkCmdBindDescriptorSets(ctx->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, frame_transform.sets + ctx->frame, 0, nullptr);

		int const count = 16;
		static float positions[count];
		static bool init = true;
		if (init) {
			FS_FOR(count) {
				positions[i] = float(i * 5 - 20);
			}
			init = false;
		}

		if (-camPos.x > positions[count/2]) {
			FS_FOR(count - 1) {
				positions[i] = positions[i + 1];
			}
			positions[count - 1] += 5;
		}

		FS_FOR(count) {
			srand(*(unsigned int*)&positions[i]);
			info.time.x = t * 10.0f + 20.0f*(float)rand()/(float)RAND_MAX;
			auto col = fs::rgb(fs::hsv{(float)rand()/(float)RAND_MAX, 1.0f, 1.0f});
			info.color.r = col.r;
			info.color.g = col.g;
			info.color.b = col.b;
			info.position = {positions[i], 3.0f*((float)rand()/(float)RAND_MAX)-1.5f, 0, 0};
			vkCmdPushConstants(ctx->command_buffer, pipeline_layout, stage, 0, sizeof(info), &info);
			vkCmdDraw(ctx->command_buffer, vertex_count, 1, 0, 0);
		}
	}

	void draw_postfx(fs::Render_Context* ctx) {
#if USE_SUBPASS
		vkCmdNextSubpass(ctx->command_buffer, VK_SUBPASS_CONTENTS_INLINE);
#endif
		vkCmdBindPipeline(ctx->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, post_pipeline);

		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(ctx->command_buffer, 0, 1, &quad_vertex_buffer, &offset);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(ctx->gfx->sc_extent.width);
		viewport.height = static_cast<float>(ctx->gfx->sc_extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(ctx->command_buffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = ctx->gfx->sc_extent;
		vkCmdSetScissor(ctx->command_buffer, 0, 1, &scissor);

		VK_GFX_BIND_DESCRIPTOR_SETS(ctx->command_buffer, post_pipeline_layout, 1, &depth_set);

		vkCmdDraw(ctx->command_buffer, 6, 1, 0, 0);
	}

	VkPipelineLayout  pipeline_layout;
	VkPipeline        pipeline;

	VkPipelineLayout  post_pipeline_layout;
	VkPipeline        post_pipeline;
	VkSampler         sampler;
	VkDescriptorSet   depth_set;

	VkBuffer          quad_vertex_buffer;
	VmaAllocation     quad_vertex_allocation;

	VkImage           depth_image;
	VmaAllocation     depth_allocation;
	VkImageView       depth_image_view;

	VkBuffer          vertex_buffer;
	VmaAllocation     vertex_allocation;
	uint32_t          vertex_count;

	Transform_Data    frame_transform;

	VkFramebuffer     frame_buffers[4];

	float t = 0.0f;
};
