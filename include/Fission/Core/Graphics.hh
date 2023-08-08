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
#include <Fission/config.hpp>
#include <Fission/Base/Color.hpp>
#include <Fission/Base/Version.hpp>
#include <Fission/Base/Math/Vector.hpp>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

__FISSION_BEGIN__

struct Graphics_Create_Info {
	struct Window* window;
//	bool vsync;
};

struct RenderTarget {
	int sampleCount = 1;
	
	VkAttachmentDescription attachment;
	VkFramebuffer framebuffer[4];
	VkRenderPass renderpass;

	VkImage image;
	VmaAllocation allocation;
	VkImageView imageview;
};

struct RenderTargetCreateInfo {
	struct Graphics* gfx;

	int sample_count = 1;
};
RenderTarget create_swap_chain_render_target(RenderTargetCreateInfo& info);

struct Render_Context {
	struct Graphics* gfx;
	VkFramebuffer frame_buffer;
	VkCommandBuffer command_buffer;
	u32 frame;
	u32 image_index;
};

struct Graphics
{
	void upload_buffer(VkBuffer dstBuffer, void const* data, VkDeviceSize size);
	void upload_image(VkImage dstImage, void* data, VkExtent3D extent);

	static constexpr int max_sc_images = 4;

	VkInstance       instance;
	VkPhysicalDevice physical_device;
	VkDevice         device;
	VkQueue          graphics_queue;
	VkQueue          present_queue;
	VkSurfaceKHR     surface;
	VkSwapchainKHR   swap_chain;

	// "sc" stands for "swap chain"
	VkExtent2D       sc_extent;
	VkFormat         sc_format;
	u32              sc_image_count;
	VkPresentModeKHR sc_present_mode;
	VkImageView      sc_image_views[max_sc_images];
	VkFramebuffer    sc_framebuffers[max_sc_images];

	VkCommandPool    command_pool;

	// use to create short-lived command buffers for copy operations
	//   that are used with the transfer queue (on a separate thread)
	VkQueue          transfer_queue;
	VkCommandPool    transfer_command_pool;

	// Primary Command Buffers
	VkCommandBuffer  command_buffers          [2];
	VkFence          cb_fences                [2];
	VkSemaphore      sc_image_write_semaphore [2];
	VkSemaphore      sc_image_read_semaphore  [2];

	VmaAllocator     allocator;

	VkDebugUtilsMessengerEXT debugMessenger; // only valid in debug mode

	bool create(Graphics_Create_Info* info); // SUCCESS == false
	void recreate_swap_chain(struct Window* wnd, VkPresentModeKHR present_mode);
	version get_api_version();

	Graphics() = default;
	Graphics(Graphics const&) = delete;
	~Graphics();
};

struct Render_Pass {
	VkRenderPass handle;
	VkImage multisampled_image;

	inline constexpr operator VkRenderPass() const { return handle; }

	void create(VkSampleCountFlagBits samples, bool clear);
	void destroy();

	void begin(Render_Context* ctx, VkFramebuffer fb, color clear);
	void begin(Render_Context* ctx, color clear);
	void begin(Render_Context* ctx);
	void end(Render_Context* ctx);

	Render_Pass() = default;
	Render_Pass(Render_Pass const&) = delete;
};

template <VkShaderStageFlags ShaderStage, VkDescriptorType DescriptorType>
struct Single_Descriptor_Set_Layout {
	Single_Descriptor_Set_Layout() = default;
	Single_Descriptor_Set_Layout(Graphics& gfx) { create(gfx); }
	inline void create(Graphics& gfx) {
		VkDescriptorSetLayoutBinding binding;
		binding.binding = 0;
		binding.descriptorCount = 1;
		binding.descriptorType = DescriptorType;
		binding.stageFlags = ShaderStage;
		binding.pImmutableSamplers = nullptr;
		VkDescriptorSetLayoutCreateInfo descriptorInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		descriptorInfo.bindingCount = 1;
		descriptorInfo.pBindings = &binding;
		vkCreateDescriptorSetLayout(gfx.device, &descriptorInfo, nullptr, &handle);
	}
	inline void destroy(Graphics& gfx) {
		vkDestroyDescriptorSetLayout(gfx.device, handle, nullptr);
	}
	inline operator VkDescriptorSetLayout() { return handle; }
	inline VkDescriptorSetLayout* operator &() { return &handle; }
	VkDescriptorSetLayout handle;
};

using Transform_2D_Layout = Single_Descriptor_Set_Layout<VK_SHADER_STAGE_VERTEX_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER>;
using Texture_Layout      = Single_Descriptor_Set_Layout<VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER>;

struct Transform_2D_Data {
	v2f32 scale;
	v2f32 offset;
};

__FISSION_END__

#define VK_GFX_BIND_DESCRIPTOR_SETS(COMMAND_BUFFER, PIPELINE_LAYOUT, SET_COUNT, SETS) \
vkCmdBindDescriptorSets(COMMAND_BUFFER, VK_PIPELINE_BIND_POINT_GRAPHICS, PIPELINE_LAYOUT, 0, SET_COUNT, SETS, 0, nullptr)

// vulkan.hpp is kinda trash ngl
// so here are some functions that are helpful to me
namespace vk {
	inline constexpr VkImageViewCreateInfo image_view_2d(VkImage image, VkFormat format, VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT) {
		return {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.flags = 0,
			.image = image,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = format,
			.components = {}, // Identity
			.subresourceRange = {
				.aspectMask = aspect,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			}
		};
	}
	inline constexpr VkSamplerCreateInfo sampler(VkFilter filter, VkSamplerAddressMode address_mode) {
		return {
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.magFilter = filter,
			.minFilter = filter,
			.addressModeU = address_mode,
			.addressModeV = address_mode,
			.addressModeW = address_mode,
		};
	}

	template <typename T> struct _format_of {
		static_assert(fs::always_false<T>, "No format mapped to this type.");
		static constexpr VkFormat value = VK_FORMAT_UNDEFINED;
	};

	template <>	struct _format_of<fs::rgba>  { static constexpr VkFormat value = VK_FORMAT_R32G32B32A32_SFLOAT; };
	template <>	struct _format_of<fs::rgb>   { static constexpr VkFormat value = VK_FORMAT_R32G32B32_SFLOAT; };
	template <>	struct _format_of<fs::v4f32> { static constexpr VkFormat value = VK_FORMAT_R32G32B32A32_SFLOAT; };
	template <>	struct _format_of<fs::v3f32> { static constexpr VkFormat value = VK_FORMAT_R32G32B32_SFLOAT; };
	template <>	struct _format_of<fs::v2f32> { static constexpr VkFormat value = VK_FORMAT_R32G32_SFLOAT; };
	template <>	struct _format_of<fs::f32>   { static constexpr VkFormat value = VK_FORMAT_R32_SFLOAT; };

	template <typename T> static constexpr VkFormat format_of = _format_of<T>::value;

	template <typename...Attributes>
	struct Basic_Vertex_Input {
		static constexpr uint32_t attribute_count = sizeof...(Attributes);
		VkPipelineVertexInputStateCreateInfo info;
		VkVertexInputBindingDescription binding;
		VkVertexInputAttributeDescription attributes[attribute_count];

		inline constexpr VkPipelineVertexInputStateCreateInfo const* operator&() const noexcept {
			return &info;
		}

		inline constexpr Basic_Vertex_Input() noexcept {
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			info.pNext = nullptr;
			info.flags = 0;
			info.vertexBindingDescriptionCount = 1;
			info.pVertexBindingDescriptions = &binding;
			info.vertexAttributeDescriptionCount = attribute_count;
			info.pVertexAttributeDescriptions = attributes;

			binding.binding = 0;
			binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			binding.stride = fs::size_of_n<attribute_count, Attributes...>;

			[&] <typename T, size_t...n>(std::integer_sequence<T, n...> int_seq) {
				((attributes[n].binding = 0), ...);
				((attributes[n].format = vk::format_of<fs::type_at<n, Attributes...>>), ...);
				((attributes[n].location = n), ...);
				(set_offset<n>(), ...);
			} (std::make_index_sequence<attribute_count>{});
		}

		// @note: Fuck templates
		template <size_t n>
		inline constexpr void set_offset() {
			attributes[n].offset = fs::size_of_n<(int)n, Attributes...>;
		}
	};
}

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