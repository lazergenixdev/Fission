#include <Fission/Core/Engine.hh>
#include <Fission/Core/Renderer_2D.hh>
#include <Fission/Core/Input/Keys.hh>
#include <iostream>
#include <sstream>
#include <random>

extern fs::Engine engine;

struct Particle {
	fs::v2f32 position;
	float size;
	float rotation;
	float rv;
};

struct vertex {
	fs::v3f32 position_emmission;
	fs::rgba  color;
};

struct Particle_System {
	std::mt19937_64 rng;
	std::uniform_real_distribution<float> d{0.0f, 1.0f};
	Particle pool[264];

	Particle_System() {
		for (auto&& p : pool) {
			p.position = {d(rng) * 1280.0f, d(rng) * 720.0f};
			//	p.weight = d(rng) + 1.0f;
			p.size = d(rng) * 10.0f;
			p.rotation = d(rng) * fs::math::tau;
			p.rv = d(rng) * 5.0f - 2.5f;
		}
	}

	static void add_particle(vertex* vd, fs::u16* id, fs::Draw_Data& d, fs::v2f32 pos, float size, float rotation, fs::color col) {
		id[d.total_idx_count++] = d.vtx_count + 0;
		id[d.total_idx_count++] = d.vtx_count + 1;
		id[d.total_idx_count++] = d.vtx_count + 2;
		id[d.total_idx_count++] = d.vtx_count + 0;
		id[d.total_idx_count++] = d.vtx_count + 2;
		id[d.total_idx_count++] = d.vtx_count + 3;
		id[d.total_idx_count++] = d.vtx_count + 0;
		id[d.total_idx_count++] = d.vtx_count + 3;
		id[d.total_idx_count++] = d.vtx_count + 4;
		id[d.total_idx_count++] = d.vtx_count + 0;
		id[d.total_idx_count++] = d.vtx_count + 4;
		id[d.total_idx_count++] = d.vtx_count + 1;

		auto const rot = fs::m22::Rotation(rotation);

		vd[d.total_vtx_count++] = {{pos                            , size*0.5f}, col};
		vd[d.total_vtx_count++] = {{pos + rot * fs::v2f32(-size, 0), size*0.5f}, fs::color(col.r,col.g,col.b,1)};
		vd[d.total_vtx_count++] = {{pos + rot * fs::v2f32(0,-size) , size*0.5f}, fs::color(col.r,col.g,col.b,1)};
		vd[d.total_vtx_count++] = {{pos + rot * fs::v2f32(size, 0) , size*0.5f}, fs::color(col.r,col.g,col.b,1)};
		vd[d.total_vtx_count++] = {{pos + rot * fs::v2f32(0, size) , size*0.5f}, fs::color(col.r,col.g,col.b,1)};

		d.idx_count += 12;
		d.vtx_count += 5;
	}

	void update(double dt, vertex* vd, fs::u16* id, fs::Draw_Data& dd) {
		auto velocity_field = [](fs::v2f32 p) {
			return fs::v2f32(-1.0f, 10.0f);
		};

		using namespace fs;
		auto screen_size = v2f32{(float)engine.graphics.sc_extent.width, (float)engine.graphics.sc_extent.height};

		auto rd_p = [&](Particle& p) {
			p.position = {d(rng) * screen_size.x, d(rng) * -3.0f - 10.0f};
			p.size = d(rng) * 10.0f;
		};

		float _dt = (float)dt;

		for (auto&& p : pool) {
			p.position += _dt * velocity_field(p.position) * (p.size);
			p.rotation += _dt * p.rv;

			if (p.position.x < -5.0f || p.position.y >(screen_size.y + 10.0f)) {
				rd_p(p);
			}

			auto a = p.position;
			a.y = screen_size.y - a.y;

			add_particle(vd, id, dd, a, p.size, p.rotation, color(colors::CornflowerBlue, std::clamp(p.size / 10.0f, 0.0f, 1.0f)));
			p.size -= _dt * 0.3f;

			if (p.size <= 0.0f) {
				rd_p(p);
			}
		}
	}
};

struct Object {
	std::vector<fs::v2f32> vertex_data;
	std::vector<fs::u16>    index_data;

	Object(char const* filename) {
		auto f = std::ifstream(filename, std::ios::binary);

		if (!f.is_open()) __debugbreak();

		fs::v2f32 v{};
		fs::u16 i0, i1, i2;

		std::string line;
		line.reserve(64);
		vertex_data.reserve(1500);
		index_data.reserve(6000);

		while (!f.eof()) {
			std::getline(f, line);

			if (line.empty()) continue;

			std::stringstream ss{line.substr(2)};
			float _;

			switch (line[0])
			{
			default:
			break; case 'v':
				ss >> v.x >> _ >> v.y;
				vertex_data.emplace_back(v);
			break; case 'f':
				ss >> i0 >> i1 >> i2;
				index_data.emplace_back(i0 - 1);
				index_data.emplace_back(i1 - 1);
				index_data.emplace_back(i2 - 1);
			break;
			}
		}
	}
};

void generateMipmaps(VkImage image, VkCommandBuffer commandBuffer, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) {
	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	{
		barrier.subresourceRange.levelCount = mipLevels - 1;
		barrier.subresourceRange.baseMipLevel = 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);
	}

	barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = texWidth;
	int32_t mipHeight = texHeight;

	for (uint32_t i = 1; i < mipLevels; i++) {
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		VkImageBlit blit = {};
		blit.srcOffsets[0] = {0, 0, 0};
		blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = {0, 0, 0};
		blit.dstOffsets[1] = {mipWidth / 2, mipHeight / 2, 1};
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		vkCmdBlitImage(commandBuffer,
			image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit,
			VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}

	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
		0, nullptr,
		0, nullptr,
		1, &barrier);
}

struct Bloom_Scene : public fs::Scene {
	static constexpr auto _max_count = 1 << 16;

	struct color_vert {
#		include "../shaders/color.vert.inl"
	};
	struct color_frag {
#		include "../shaders/color.frag.inl"
	};
	struct emmission_vert {
#		include "../shaders/emmission.vert.inl"
	};
	struct emmission_frag {
#		include "../shaders/emmission.frag.inl"
	};
	struct fullscreen_vert {
#		include "../shaders/fullscreen.vert.inl"
	};
	struct multiply_frag {
#		include "../shaders/fullscreen.frag.inl"
	};
	struct blur_frag {
#		include "../shaders/blur.frag.inl"
	};
	struct composite_frag {
#		include "../shaders/composite.frag.inl"
	};

	void create_first_render_pass() {
		VkAttachmentDescription blurInputColorAttachment{};
		blurInputColorAttachment.format = VK_FORMAT_R32G32B32A32_SFLOAT;
		blurInputColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		blurInputColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		blurInputColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		blurInputColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		blurInputColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		blurInputColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		blurInputColorAttachment.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = VK_FORMAT_B8G8R8A8_UNORM;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentDescription emmissionAttachment{};
		emmissionAttachment.format = VK_FORMAT_R32_SFLOAT;
		emmissionAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		emmissionAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		emmissionAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		emmissionAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		emmissionAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		emmissionAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		emmissionAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference blurInputColorAttachmentRef{};
		blurInputColorAttachmentRef.attachment = 0;
		blurInputColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 1;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference emmissionAttachmentRef{};
		emmissionAttachmentRef.attachment = 2;
		emmissionAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpasses[3] = {};
		subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpasses[0].colorAttachmentCount = 1;
		subpasses[0].pColorAttachments = &colorAttachmentRef;

		subpasses[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpasses[1].colorAttachmentCount = 1;
		subpasses[1].pColorAttachments = &emmissionAttachmentRef;

		VkAttachmentReference inputAttachments[2] = {
			{.attachment = 1, .layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
			{.attachment = 2, .layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL}
		};
		subpasses[2].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpasses[2].colorAttachmentCount = 1;
		subpasses[2].pColorAttachments = &blurInputColorAttachmentRef;
		subpasses[2].inputAttachmentCount = 2;
		subpasses[2].pInputAttachments = inputAttachments;

		VkSubpassDependency dependencies[4] = {};
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
		dependencies[2].dstSubpass = 2;
		dependencies[2].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[2].srcAccessMask = 0;
		dependencies[2].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[2].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependencies[3].srcSubpass = 1;
		dependencies[3].dstSubpass = 2;
		dependencies[3].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[3].srcAccessMask = 0;
		dependencies[3].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[3].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		auto attachments = {blurInputColorAttachment, colorAttachment, emmissionAttachment};

		VkRenderPassCreateInfo renderPassInfo{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
		renderPassInfo.attachmentCount = (fs::u32)attachments.size();
		renderPassInfo.pAttachments = attachments.begin();
		renderPassInfo.subpassCount = (fs::u32)std::size(subpasses);
		renderPassInfo.pSubpasses = subpasses;
		renderPassInfo.dependencyCount = (fs::u32)std::size(dependencies);
		renderPassInfo.pDependencies = dependencies;

		vkCreateRenderPass(engine.graphics.device, &renderPassInfo, nullptr, &color_render_pass.handle);
	}

	void create_blur_render_pass() {
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = VK_FORMAT_R32G32B32A32_SFLOAT;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpasses[1] = {};
		subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpasses[0].colorAttachmentCount = 1;
		subpasses[0].pColorAttachments = &colorAttachmentRef;

		VkSubpassDependency dependencies[1] = {};
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = 0;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		auto attachments = {colorAttachment};

		VkRenderPassCreateInfo renderPassInfo{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
		renderPassInfo.attachmentCount = (fs::u32)attachments.size();
		renderPassInfo.pAttachments = attachments.begin();
		renderPassInfo.subpassCount = (fs::u32)std::size(subpasses);
		renderPassInfo.pSubpasses = subpasses;
		renderPassInfo.dependencyCount = (fs::u32)std::size(dependencies);
		renderPassInfo.pDependencies = dependencies;

		vkCreateRenderPass(engine.graphics.device, &renderPassInfo, nullptr, &blur_render_pass.handle);
	}

	void create_composite_render_pass() {
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = engine.graphics.sc_format;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpasses[1] = {};
		subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpasses[0].colorAttachmentCount = 1;
		subpasses[0].pColorAttachments = &colorAttachmentRef;

		VkSubpassDependency dependencies[1] = {};
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = 0;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		auto attachments = {colorAttachment};

		VkRenderPassCreateInfo renderPassInfo{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
		renderPassInfo.attachmentCount = (fs::u32)attachments.size();
		renderPassInfo.pAttachments = attachments.begin();
		renderPassInfo.subpassCount = (fs::u32)std::size(subpasses);
		renderPassInfo.pSubpasses = subpasses;
		renderPassInfo.dependencyCount = (fs::u32)std::size(dependencies);
		renderPassInfo.pDependencies = dependencies;

		vkCreateRenderPass(engine.graphics.device, &renderPassInfo, nullptr, &composite_render_pass.handle);
	}

	void create_emmission_pipeline(
		const VkRenderPass                          render_pass,
		const VkPipelineLayout                      pipeline_layout,
		const VkPipelineVertexInputStateCreateInfo* vertex_input
	) {
		auto& gfx = engine.graphics;

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = fs::create_shader(gfx.device, emmission_vert::size, emmission_vert::data);
		vertShaderStageInfo.pName = "main";
		VkPipelineShaderStageCreateInfo fragShaderStageInfo{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fs::create_shader(gfx.device, emmission_frag::size, emmission_frag::data);
		fragShaderStageInfo.pName = "main";
		VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
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
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;

		VkPipelineColorBlendStateCreateInfo colorBlending{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;

		VkGraphicsPipelineCreateInfo pipelineInfo{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
		pipelineInfo.stageCount = (fs::u32)std::size(shaderStages);
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = vertex_input;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr; // Optional
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicStateInfo;
		pipelineInfo.layout = pipeline_layout;
		pipelineInfo.renderPass = render_pass;
		pipelineInfo.subpass = 1;

		vkCreateGraphicsPipelines(gfx.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &emmission_pipeline);

		vkDestroyShaderModule(gfx.device, vertShaderStageInfo.module, nullptr);
		vkDestroyShaderModule(gfx.device, fragShaderStageInfo.module, nullptr);
	}

	void end_render(fs::Render_Context const* ctx) {
		if (d.total_vtx_count) {
			auto& fd = frame_data[ctx->frame];
			fd.set_data(ctx->gfx, vertex_data, index_data, d.total_vtx_count, d.total_idx_count);
			d.reset();
		}
	}

	void add_triangle(fs::v2f32 center, float size, float em, fs::color color) {
		index_data[d.total_idx_count++] = d.vtx_count;
		index_data[d.total_idx_count++] = d.vtx_count + 1;
		index_data[d.total_idx_count++] = d.vtx_count + 2;

		vertex_data[d.total_vtx_count++] = {{center + fs::v2f32( 0.0f,-size), em}, color};
		vertex_data[d.total_vtx_count++] = {{center + fs::v2f32( size, size), em}, color};
		vertex_data[d.total_vtx_count++] = {{center + fs::v2f32(-size, size), em}, color};

		d.idx_count += 3;
		d.vtx_count += 3;
	}

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

		static float t = 0.0f;
		add_triangle((fs::v2f32)engine.window.mouse_position, 20, 5.0f*(sinf(t) + 1.0f), colors::White);
		t += (float)dt;

		ps.update(dt, vertex_data, index_data, d);

		VkClearValue clear_values[3];
		clear_values[0].color = {0.0f, 0.0f, 0.0f, 0.0f};
		clear_values[1].color = {0.0f, 0.0f, 0.0f, 0.0f};
		clear_values[2].color = {0.0f, 0.0f, 0.0f, 0.0f};
		VkRenderPassBeginInfo beginInfo{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
		beginInfo.framebuffer = first_pass_frame_buffer;
		beginInfo.renderPass = color_render_pass.handle;
		beginInfo.clearValueCount = 3;
		beginInfo.pClearValues = clear_values;
		beginInfo.renderArea.extent = ctx->gfx->sc_extent;
		beginInfo.renderArea.offset = {0, 0};
		vkCmdBeginRenderPass(ctx->command_buffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VK_GFX_BIND_DESCRIPTOR_SETS(ctx->command_buffer, engine.renderer_2d.pipeline_layout, 1, &engine.transform_2d.set);
		vkCmdBindPipeline(ctx->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, color_pipeline);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width  = static_cast<float>(ctx->gfx->sc_extent.width);
		viewport.height = static_cast<float>(ctx->gfx->sc_extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(ctx->command_buffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent = ctx->gfx->sc_extent;
		vkCmdSetScissor(ctx->command_buffer, 0, 1, &scissor);

		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(ctx->command_buffer, 0, 1, &vertex_buffer, &offset);
		vkCmdBindIndexBuffer(ctx->command_buffer, index_buffer, offset, VK_INDEX_TYPE_UINT16);
		vkCmdDrawIndexed(ctx->command_buffer, index_count, 1, 0, 0, 0);

		auto& fd = frame_data[ctx->frame];

		vkCmdBindIndexBuffer(ctx->command_buffer, fd.index_buffer, 0, VK_INDEX_TYPE_UINT16);
		vkCmdBindVertexBuffers(ctx->command_buffer, 0, 1, &fd.vertex_buffer, &offset);
		vkCmdDrawIndexed(ctx->command_buffer, d.idx_count, 1, d.idx_offset, d.vtx_offset, 0);

		/*******************************************************************************************************/

		vkCmdNextSubpass(ctx->command_buffer, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(ctx->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, emmission_pipeline);

		vkCmdBindVertexBuffers(ctx->command_buffer, 0, 1, &vertex_buffer, &offset);
		vkCmdBindIndexBuffer(ctx->command_buffer, index_buffer, offset, VK_INDEX_TYPE_UINT16);
		vkCmdDrawIndexed(ctx->command_buffer, index_count, 1, 0, 0, 0);

		vkCmdBindIndexBuffer(ctx->command_buffer, fd.index_buffer, 0, VK_INDEX_TYPE_UINT16);
		vkCmdBindVertexBuffers(ctx->command_buffer, 0, 1, &fd.vertex_buffer, &offset);
		vkCmdDrawIndexed(ctx->command_buffer, d.idx_count, 1, d.idx_offset, d.vtx_offset, 0);
		
		/*******************************************************************************************************/
	
		vkCmdNextSubpass(ctx->command_buffer, VK_SUBPASS_CONTENTS_INLINE);

		VK_GFX_BIND_DESCRIPTOR_SETS(ctx->command_buffer, mask_pipeline_layout, 1, &mask_descriptor_set);
		vkCmdBindPipeline(ctx->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mask_pipeline);

		vkCmdDraw(ctx->command_buffer, 3, 1, 0, 0);
		
		color_render_pass.end(ctx);

		generateMipmaps(blur_src_image, ctx->command_buffer, ctx->gfx->sc_extent.width, ctx->gfx->sc_extent.height, mip_count);

		FS_FOR(mip_count) {
			VkRenderPassBeginInfo beginInfo{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
			beginInfo.framebuffer = blur_frame_buffers[i];
			beginInfo.renderPass = blur_render_pass;
			beginInfo.clearValueCount = 0;
			beginInfo.renderArea.offset = {0, 0};
			beginInfo.renderArea.extent = ctx->gfx->sc_extent;
			beginInfo.renderArea.extent.width  >>= i;
			beginInfo.renderArea.extent.height >>= i;
			vkCmdBeginRenderPass(ctx->command_buffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

			VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width  = float(beginInfo.renderArea.extent.width);
			viewport.height = float(beginInfo.renderArea.extent.height);
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(ctx->command_buffer, 0, 1, &viewport);

			VkRect2D scissor{};
			scissor.offset = {0, 0};
			scissor.extent = beginInfo.renderArea.extent;
			vkCmdSetScissor(ctx->command_buffer, 0, 1, &scissor);

			VK_GFX_BIND_DESCRIPTOR_SETS(ctx->command_buffer, blur_pipeline_layout, 1, blur_descriptor_sets + i);

			vkCmdBindPipeline(ctx->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, blur_pipeline);
			vkCmdDraw(ctx->command_buffer, 3, 1, 0, 0);
			blur_render_pass.end(ctx);
		}

		composite_render_pass.begin(ctx, colors::Black);
		{
			vkCmdBindPipeline(ctx->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, composite_pipeline);

			VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = static_cast<float>(ctx->gfx->sc_extent.width);
			viewport.height = static_cast<float>(ctx->gfx->sc_extent.height);
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(ctx->command_buffer, 0, 1, &viewport);

			VkRect2D scissor{};
			scissor.offset = {0, 0};
			scissor.extent = ctx->gfx->sc_extent;
			vkCmdSetScissor(ctx->command_buffer, 0, 1, &scissor);

			FS_FOR(mip_count+1) {
				VK_GFX_BIND_DESCRIPTOR_SETS(ctx->command_buffer, composite_pipeline_layout, 1, composite_descriptor_sets + i);
				vkCmdDraw(ctx->command_buffer, 3, 1, 0, 0);
			}
		}
		composite_render_pass.end(ctx);

		end_render(ctx);
	}
	virtual void on_resize() override {}

	Bloom_Scene() {
		auto& gfx = engine.graphics;
		auto format = gfx.sc_format;
		
		max_vertex_count = _max_count,
		max_index_count  = _max_count * 2,

		// TODO: only one malloc needed
		vertex_data = (vertex*)_aligned_malloc(max_vertex_count * sizeof(vertex), 32);
		index_data = (fs::u16*)_aligned_malloc(max_index_count * sizeof(fs::u16), 32);

		frame_data[0] = fs::Frame_Data<vertex, fs::u16>(gfx.allocator, max_vertex_count, max_index_count);
		frame_data[1] = fs::Frame_Data<vertex, fs::u16>(gfx.allocator, max_vertex_count, max_index_count);

		create_first_render_pass();
		create_blur_render_pass();
		create_composite_render_pass();

		color_surface.create(gfx, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
		emmission_surface.create(gfx, VK_FORMAT_R32_SFLOAT);
		
		{
			auto samplerInfo = vk::sampler(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
			vkCreateSampler(gfx.device, &samplerInfo, nullptr, &linear_sampler);
		}

		{
			VkPipelineLayoutCreateInfo pipelineLayoutInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
			pipelineLayoutInfo.pSetLayouts = &engine.texture_layout;
			pipelineLayoutInfo.setLayoutCount = 1;
			vkCreatePipelineLayout(gfx.device, &pipelineLayoutInfo, nullptr, &composite_pipeline_layout);
		}

		{
			auto format = VK_FORMAT_R32G32B32A32_SFLOAT;
			VmaAllocationCreateInfo allocInfo = {};
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
			VkImageCreateInfo imageInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
			imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			imageInfo.arrayLayers = 1;
			imageInfo.extent = {.width = gfx.sc_extent.width, .height = gfx.sc_extent.height, .depth = 1};
			imageInfo.format = format;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.mipLevels = mip_count;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;

			vmaCreateImage(gfx.allocator, &imageInfo, &allocInfo, &blur_src_image, &blur_src_allocation, nullptr);
			
			imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			vmaCreateImage(gfx.allocator, &imageInfo, &allocInfo, &blur_dst_image, &blur_dst_allocation, nullptr);

			auto imageViewInfo = vk::image_view_2d(blur_src_image, format);
			FS_FOR(mip_count) {
				imageViewInfo.subresourceRange.baseMipLevel = i;
				imageViewInfo.subresourceRange.levelCount = 1;

				imageViewInfo.image = blur_src_image;
				vkCreateImageView(gfx.device, &imageViewInfo, nullptr, blur_src_views + i);

				imageViewInfo.image = blur_dst_image;
				vkCreateImageView(gfx.device, &imageViewInfo, nullptr, blur_dst_views + i);
			}
		}

		{
			VkImageView attachments[3] = { blur_src_views[0], color_surface.view, emmission_surface.view };
			VkFramebufferCreateInfo framebufferInfo{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
			framebufferInfo.renderPass = color_render_pass;
			framebufferInfo.attachmentCount = 3;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = gfx.sc_extent.width;
			framebufferInfo.height = gfx.sc_extent.height;
			framebufferInfo.layers = 1;
			vkCreateFramebuffer(gfx.device, &framebufferInfo, nullptr, &first_pass_frame_buffer);
		}

		{
			VkFramebufferCreateInfo framebufferInfo{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
			framebufferInfo.renderPass = blur_render_pass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.width = gfx.sc_extent.width;
			framebufferInfo.height = gfx.sc_extent.height;
			framebufferInfo.layers = 1;

			FS_FOR(mip_count) {
				framebufferInfo.pAttachments = blur_dst_views + i;
				vkCreateFramebuffer(gfx.device, &framebufferInfo, nullptr, blur_frame_buffers + i);
			
				framebufferInfo.width  >>= 1;
				framebufferInfo.height >>= 1;
			}
		}

		vk::Basic_Vertex_Input<fs::v3f32, fs::rgba> vi{};

		fs::Pipeline_Create_Info pi;
		pi.device          = gfx.device;
		pi.render_pass     = color_render_pass;
		pi.pipeline_layout = engine.renderer_2d.pipeline_layout;
		pi.vertex_shader   = fs::create_shader(gfx.device, color_vert::size, color_vert::data);
		pi.fragment_shader = fs::create_shader(gfx.device, color_frag::size, color_frag::data);
		pi.vertex_input = &vi;
		pi.blend_mode = fs::Blend_Mode_Normal;
		fs::create_pipeline(pi, &color_pipeline);

		vkDestroyShaderModule(gfx.device, pi.vertex_shader, nullptr);
		vkDestroyShaderModule(gfx.device, pi.fragment_shader, nullptr);

		create_emmission_pipeline(color_render_pass, pi.pipeline_layout, &vi);

		{
			VkDescriptorSetLayoutBinding bindings[2];
			bindings[0].binding = 0;
			bindings[0].descriptorCount = 1;
			bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			bindings[0].pImmutableSamplers = nullptr;
			bindings[1].binding = 1;
			bindings[1].descriptorCount = 1;
			bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			bindings[1].pImmutableSamplers = nullptr;

			VkDescriptorSetLayoutCreateInfo descriptorInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
			descriptorInfo.bindingCount = 2;
			descriptorInfo.pBindings = bindings;
			vkCreateDescriptorSetLayout(gfx.device, &descriptorInfo, nullptr, &mask_descriptor_layout);

			VkDescriptorSetAllocateInfo descSetAllocInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
			descSetAllocInfo.descriptorPool = engine.descriptor_pool;
			descSetAllocInfo.pSetLayouts = &mask_descriptor_layout;
			descSetAllocInfo.descriptorSetCount = 1;
			vkAllocateDescriptorSets(gfx.device, &descSetAllocInfo, &mask_descriptor_set);

			FS_FOR(2) {
				VkDescriptorImageInfo imageInfo;
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfo.imageView = i == 0 ? color_surface.view : emmission_surface.view;
				imageInfo.sampler = nullptr;
				VkWriteDescriptorSet write{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
				write.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
				write.descriptorCount = 1;
				write.dstBinding = i;
				write.dstSet = mask_descriptor_set;
				write.pImageInfo = &imageInfo;
				vkUpdateDescriptorSets(gfx.device, 1, &write, 0, nullptr);
			}

			VkPipelineLayoutCreateInfo pipelineLayoutInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
			pipelineLayoutInfo.pSetLayouts = &mask_descriptor_layout;
			pipelineLayoutInfo.setLayoutCount = 1;
			vkCreatePipelineLayout(gfx.device, &pipelineLayoutInfo, nullptr, &mask_pipeline_layout);
		}

		FS_FOR(mip_count) {
			VmaAllocationCreateInfo allocInfo = {};
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
			VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
			bufferInfo.size = sizeof(fs::v4f32);
			bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			vmaCreateBuffer(gfx.allocator, &bufferInfo, &allocInfo, &blur_uniform_buffers[i], &blur_uniform_allocations[i], nullptr);

			fs::v4f32 resolution = {};
			resolution.x = float(gfx.sc_extent.width  >> i);
			resolution.y = float(gfx.sc_extent.height >> i);
			resolution.z = 1.0f / float(1 << i);
			gfx.upload_buffer(blur_uniform_buffers[i], &resolution, sizeof(resolution));
		}

		{
			VkDescriptorSetLayoutBinding bindings[2];
			bindings[0].binding = 0;
			bindings[0].descriptorCount = 1;
			bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			bindings[0].pImmutableSamplers = nullptr;
			bindings[1].binding = 1;
			bindings[1].descriptorCount = 1;
			bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			bindings[1].pImmutableSamplers = nullptr;
			VkDescriptorSetLayoutCreateInfo descriptorInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
			descriptorInfo.bindingCount = 2;
			descriptorInfo.pBindings = bindings;
			vkCreateDescriptorSetLayout(gfx.device, &descriptorInfo, nullptr, &blur_descriptor_layout);
		}

		{
			VkDescriptorSetLayout layouts[mip_count];
			FS_FOR(mip_count) layouts[i] = blur_descriptor_layout;
			VkDescriptorSetAllocateInfo descSetAllocInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
			descSetAllocInfo.descriptorPool = engine.descriptor_pool;
			descSetAllocInfo.pSetLayouts = layouts;
			descSetAllocInfo.descriptorSetCount = mip_count;
			vkAllocateDescriptorSets(gfx.device, &descSetAllocInfo, blur_descriptor_sets);

			VkPipelineLayoutCreateInfo pipelineLayoutInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
			pipelineLayoutInfo.pSetLayouts = &blur_descriptor_layout;
			pipelineLayoutInfo.setLayoutCount = 1;
			vkCreatePipelineLayout(gfx.device, &pipelineLayoutInfo, nullptr, &blur_pipeline_layout);
		}

		{
			VkDescriptorImageInfo imageInfo;
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.sampler = linear_sampler;

			VkDescriptorBufferInfo bufferInfo;
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(fs::v2f32);

			VkWriteDescriptorSet writes[2] = {};
			writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writes[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			writes[0].descriptorCount = 1;
			writes[0].dstBinding = 0;
			writes[0].pImageInfo = &imageInfo;

			writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writes[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writes[1].descriptorCount = 1;
			writes[1].dstBinding = 1;
			writes[1].pBufferInfo = &bufferInfo;

			FS_FOR(mip_count) {
				imageInfo.imageView = blur_src_views[i];
				bufferInfo.buffer = blur_uniform_buffers[i];
				writes[0].dstSet = blur_descriptor_sets[i];
				writes[1].dstSet = blur_descriptor_sets[i];
				vkUpdateDescriptorSets(gfx.device, 2, writes, 0, nullptr);
			}
		}

		{
			VkDescriptorSetLayout layouts[mip_count+1];
			FS_FOR(mip_count+1) layouts[i] = engine.texture_layout;
			VkDescriptorSetAllocateInfo descSetAllocInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
			descSetAllocInfo.descriptorPool = engine.descriptor_pool;
			descSetAllocInfo.pSetLayouts = layouts;
			descSetAllocInfo.descriptorSetCount = mip_count+1;
			vkAllocateDescriptorSets(gfx.device, &descSetAllocInfo, composite_descriptor_sets);
			
			VkDescriptorImageInfo imageInfo;
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.sampler = linear_sampler;

			VkWriteDescriptorSet write{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
			write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write.descriptorCount = 1;
			write.dstBinding = 0;
			write.pImageInfo = &imageInfo;

			imageInfo.imageView = color_surface.view;
			write.dstSet = composite_descriptor_sets[0];
			vkUpdateDescriptorSets(gfx.device, 1, &write, 0, nullptr);

			FS_FOR(mip_count) {
				imageInfo.imageView = blur_dst_views[i];
				write.dstSet = composite_descriptor_sets[i+1];
				vkUpdateDescriptorSets(gfx.device, 1, &write, 0, nullptr);
			}
		}

		VkPipelineVertexInputStateCreateInfo vs{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
		pi.pipeline_layout = mask_pipeline_layout;
		pi.vertex_shader   = fs::create_shader(gfx.device, fullscreen_vert::size, fullscreen_vert::data);
		pi.fragment_shader = fs::create_shader(gfx.device, multiply_frag::size, multiply_frag::data);
		pi.vertex_input = &vs;
		pi.blend_mode = fs::Blend_Mode_Disabled;
		pi.subpass = 2;
		fs::create_pipeline(pi, &mask_pipeline);
		vkDestroyShaderModule(gfx.device, pi.fragment_shader, nullptr);

		pi.render_pass = blur_render_pass;
		pi.pipeline_layout = blur_pipeline_layout;
		pi.fragment_shader = fs::create_shader(gfx.device, blur_frag::size, blur_frag::data);
		pi.subpass = 0;
		fs::create_pipeline(pi, &blur_pipeline);
		vkDestroyShaderModule(gfx.device, pi.fragment_shader, nullptr);

		pi.render_pass = composite_render_pass;
		pi.pipeline_layout = composite_pipeline_layout;
		pi.fragment_shader = fs::create_shader(gfx.device, composite_frag::size, composite_frag::data);
		pi.blend_mode = fs::Blend_Mode_Add;
		fs::create_pipeline(pi, &composite_pipeline);
		vkDestroyShaderModule(gfx.device, pi.fragment_shader, nullptr);

		vkDestroyShaderModule(gfx.device, pi.vertex_shader, nullptr);


		Object obj{"emissive.obj"};
		vertex_count = (int)obj.vertex_data.size();
		index_count  = (int)obj.index_data.size();

		VmaAllocationCreateInfo allocInfo{};
		allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
		{
			VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
			bufferInfo.size = vertex_count * sizeof(vertex);
			bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			vmaCreateBuffer(engine.graphics.allocator, &bufferInfo, &allocInfo, &vertex_buffer, &vertex_allocation, nullptr);
		}
		{
			VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
			bufferInfo.size = index_count * sizeof(fs::u16);
			bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			vmaCreateBuffer(engine.graphics.allocator, &bufferInfo, &allocInfo, &index_buffer, &index_allocation, nullptr);
		}

		gfx.upload_buffer(index_buffer, obj.index_data.data(), index_count * sizeof(fs::u16));

		auto vd = new vertex[vertex_count];

		FS_FOR(vertex_count) {
			auto& v = obj.vertex_data[i];

			vd[i].position_emmission = fs::v3f32(v.x * 300.0f, v.y * 300.0f, 7.0f);
			vd[i].position_emmission.y += 320.0f;
			vd[i].position_emmission.x += 70.0f;
			vd[i].color = (fs::rgb)fs::hsv(v.x*0.3f+v.y*0.2f, 1.0f, 1.0f);
		}

		gfx.upload_buffer(vertex_buffer, vd, vertex_count * sizeof(vertex));

		delete [] vd;
#if 0
		static constexpr auto r1 = fs::rf32::from_center(200, 400, 100, 100);
		static constexpr auto r2 = fs::rf32::from_center(600, 700, 500, 100);
		static constexpr auto r3 = fs::rf32::from_center(500, 400, 50, 50);
		static constexpr auto r4 = fs::rf32::from_center(600, 480, 10, 10);

		static constexpr vertex v[] = {
			{{r1.topLeft() ,1.0f}, fs::colors::Blue},
			{{r1.topRight(),1.0f}, fs::colors::Blue},
			{{r1.botRight(),1.0f}, fs::colors::Blue},
			{{r1.botRight(),1.0f}, fs::colors::Blue},
			{{r1.botLeft() ,1.0f}, fs::colors::Blue},
			{{r1.topLeft() ,1.0f}, fs::colors::Blue},

			{{r2.topLeft() ,1.0f}, fs::colors::Red},
			{{r2.topRight(),1.0f}, fs::colors::Yellow},
			{{r2.botRight(),5.0f}, fs::colors::Yellow},
			{{r2.botRight(),5.0f}, fs::colors::Yellow},
			{{r2.botLeft() ,5.0f}, fs::colors::Red},
			{{r2.topLeft() ,1.0f}, fs::colors::Red},

			{{r3.topLeft() ,3.0f}, fs::colors::Red},
			{{r3.topRight(),3.0f}, fs::colors::Red},
			{{r3.botRight(),3.0f}, fs::colors::Red},
			{{r3.botRight(),3.0f}, fs::colors::Red},
			{{r3.botLeft() ,3.0f}, fs::colors::Red},
			{{r3.topLeft() ,3.0f}, fs::colors::Red},
			{{r4.topLeft() ,1.0f}, fs::colors::Red},
			{{r4.topRight(),1.0f}, fs::colors::Red},
			{{r4.botRight(),1.0f}, fs::colors::Red},
			{{r4.botRight(),1.0f}, fs::colors::Red},
			{{r4.botLeft() ,1.0f}, fs::colors::Red},
			{{r4.topLeft() ,1.0f}, fs::colors::Red},
		};
#endif

	}
	~Bloom_Scene() override {
		auto& gfx = engine.graphics;

		color_render_pass.destroy();
		blur_render_pass.destroy();
		composite_render_pass.destroy();

		vkDestroyPipeline(gfx.device, color_pipeline, nullptr);
		vkDestroyPipeline(gfx.device, emmission_pipeline, nullptr);
		vkDestroyPipeline(gfx.device, mask_pipeline, nullptr);
		vkDestroyPipeline(gfx.device, blur_pipeline, nullptr);
		vkDestroyPipeline(gfx.device, composite_pipeline, nullptr);

		vkDestroyPipelineLayout(gfx.device, mask_pipeline_layout, nullptr);
		vkDestroyPipelineLayout(gfx.device, blur_pipeline_layout, nullptr);
		vkDestroyPipelineLayout(gfx.device, composite_pipeline_layout, nullptr);

		vkDestroyDescriptorSetLayout(gfx.device, mask_descriptor_layout, nullptr);
		vkDestroyDescriptorSetLayout(gfx.device, blur_descriptor_layout, nullptr);

		FS_FOR(mip_count) vmaDestroyBuffer(gfx.allocator, blur_uniform_buffers[i], blur_uniform_allocations[i]);

		color_surface.destroy();
		emmission_surface.destroy();

		vmaDestroyImage(gfx.allocator, blur_src_image, blur_src_allocation);
		vmaDestroyImage(gfx.allocator, blur_dst_image, blur_dst_allocation);

		FS_FOR(mip_count) vkDestroyImageView(gfx.device, blur_src_views[i], nullptr);
		FS_FOR(mip_count) vkDestroyImageView(gfx.device, blur_dst_views[i], nullptr);

		vkDestroyFramebuffer(gfx.device, first_pass_frame_buffer, nullptr);
		FS_FOR(mip_count) vkDestroyFramebuffer(gfx.device, blur_frame_buffers[i], nullptr);

		vkDestroySampler(gfx.device, linear_sampler, nullptr);

		vmaDestroyBuffer(gfx.allocator, vertex_buffer, vertex_allocation);
		vmaDestroyBuffer(gfx.allocator, index_buffer, index_allocation);

		_aligned_free(vertex_data);
		_aligned_free(index_data);
		for (auto&& fd : frame_data) {
			vmaDestroyBuffer(gfx.allocator, fd.vertex_buffer, fd.vertex_allocation);
			vmaDestroyBuffer(gfx.allocator, fd.index_buffer, fd.index_allocation);
		}
	}

	struct Surface {
		VkImage       image;
		VkImageView   view;
		VmaAllocation allocation;

		void create(fs::Graphics& gfx, VkFormat format, VkImageUsageFlagBits extra_usage = {}) {
			VmaAllocationCreateInfo allocInfo = {};
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
			VkImageCreateInfo imageInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
			imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | extra_usage;
			imageInfo.arrayLayers = 1;
			imageInfo.extent = {.width = gfx.sc_extent.width, .height = gfx.sc_extent.height, .depth = 1};
			imageInfo.format = format;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.mipLevels = 1;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;

			vmaCreateImage(gfx.allocator, &imageInfo, &allocInfo, &image, &allocation, nullptr);

			auto imageViewInfo = vk::image_view_2d(image, format);
			vkCreateImageView(gfx.device, &imageViewInfo, nullptr, &view);
		}

		void destroy() {
			vmaDestroyImage(engine.graphics.allocator, image, allocation);
			vkDestroyImageView(engine.graphics.device, view, nullptr);
		}
	};

	int vertex_count = 0;
	int index_count  = 0;

	static constexpr int mip_count = 4;

	fs::Render_Pass color_render_pass;
	fs::Render_Pass blur_render_pass;
	fs::Render_Pass composite_render_pass;

	VkPipeline      color_pipeline;
	VkPipeline      emmission_pipeline;
	VkPipeline      mask_pipeline;
	VkPipeline      blur_pipeline;
	VkPipeline      composite_pipeline;

	VkPipelineLayout      mask_pipeline_layout;
	VkDescriptorSetLayout mask_descriptor_layout;
	VkDescriptorSet       mask_descriptor_set;

	VkPipelineLayout      blur_pipeline_layout;
	VkDescriptorSetLayout blur_descriptor_layout;
	VkDescriptorSet       blur_descriptor_sets     [mip_count];
	VkBuffer              blur_uniform_buffers     [mip_count];
	VmaAllocation         blur_uniform_allocations [mip_count];

	VkPipelineLayout composite_pipeline_layout;
	VkDescriptorSet  composite_descriptor_sets[mip_count+1];

	Surface         color_surface;
	Surface         emmission_surface;
	VkFramebuffer   first_pass_frame_buffer;

	VkImage         blur_src_image;
	VmaAllocation   blur_src_allocation;
	VkImageView     blur_src_views[mip_count];

	VkImage         blur_dst_image;
	VmaAllocation   blur_dst_allocation;
	VkImageView     blur_dst_views[mip_count];

	VkFramebuffer   blur_frame_buffers[mip_count];

	VkSampler       linear_sampler;

	VkBuffer        vertex_buffer;
	VmaAllocation   vertex_allocation;
	VkBuffer        index_buffer;
	VmaAllocation   index_allocation;


	fs::u32 max_vertex_count;
	fs::u32 max_index_count;

	vertex* vertex_data;
	fs::u16* index_data;

	fs::Frame_Data<vertex, fs::u16> frame_data[2];
	fs::Draw_Data d;


	Particle_System ps;
};

fs::Defaults on_create() {
	engine.app_name = FS_str("Emission Demo");
	return {
		.window_title = FS_str("Emission Demo"),
	};
}

fs::Scene* on_create_scene(fs::Scene_Key const& key) {
	return new Bloom_Scene;
}
