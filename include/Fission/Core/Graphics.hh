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
#include <Fission/Base/Array.hpp>
#include <Fission/Base/Math/Vector.hpp>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#define FISSION_DEFAULT_SWAP_CHAIN_USAGE \
VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT

__FISSION_BEGIN__

struct Graphics_Create_Info {
	struct Window*   window;
	VkPresentModeKHR present_mode;
};

struct MSAA_Info {
	VkSampleCountFlagBits sampleCount;
	VkImage       image;
	VmaAllocation allocation;
	VkImageView   imageview;
};

struct Render_Context {
	struct Graphics* const gfx;
	VkFramebuffer frame_buffer;
	VkCommandBuffer command_buffer;
	u32 frame;
	u32 image_index;
};

// Rarely used data for Graphics
struct Graphics_Stale_Data {
	struct Queue_Families {
		u32 graphics;
		u32 present;
		u32 transfer;
	} queue_family_index;
};

struct FISSION_API Graphics
{
	void upload_buffer(VkBuffer dstBuffer, void const* data, VkDeviceSize size);
	void upload_image(VkImage dstImage, void* data, VkExtent3D extent, VkFormat format, VkImageLayout outLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	array<VkPresentModeKHR> supported_present_modes() { return {}; }

	version get_api_version();

	static constexpr int max_sc_images = 4;

	VkInstance       instance;
	VkPhysicalDevice physical_device;
	VkDevice         device;
	VkQueue          graphics_queue;
	VkQueue          present_queue;
	VkSurfaceKHR     surface;
	VkSwapchainKHR   swap_chain;

	// "sc" stands for "swap chain"
	VkExtent2D        sc_extent;
	VkFormat          sc_format;
	VkImageUsageFlags sc_image_usage;
	u32               sc_image_count;
	VkPresentModeKHR  sc_present_mode;
	VkImage           sc_images       [max_sc_images];
	VkImageView       sc_image_views  [max_sc_images];

	// Main graphics command pool
	VkCommandPool    command_pool;

	VkQueue          transfer_queue;
	VkCommandPool    transfer_command_pool;

	// Primary Command Buffers
	VkCommandBuffer  command_buffers          [2];
	VkFence          cb_fences                [2];
	VkSemaphore      sc_image_write_semaphore [2];
	VkSemaphore      sc_image_read_semaphore  [2];

	VmaAllocator     allocator;

	VkDebugUtilsMessengerEXT debugMessenger; // only valid in debug mode

	Graphics() = default;
	Graphics(Graphics const&) = delete;
	~Graphics();

private:
	friend struct Engine;

	bool create(Graphics_Create_Info* info); // SUCCESS == false
	void recreate_swap_chain(struct Window* wnd);
};

struct Render_Pass {
	VkRenderPass handle;
	VkImage multisampled_image;

	inline constexpr operator VkRenderPass() const { return handle; }

	void create(VkSampleCountFlagBits samples, bool clear);
	void destroy();

	void begin(Render_Context* ctx, VkFramebuffer fb, color clear);
	void begin(Render_Context* ctx, color clear);
	void begin(Render_Context* ctx, VkFramebuffer fb);
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

	static constexpr uint32_t size_of(VkFormat format) {
		switch (format)
		{
		case VK_FORMAT_UNDEFINED:
			return 0;
		case VK_FORMAT_R8_UNORM:
		case VK_FORMAT_R8_SNORM:
		case VK_FORMAT_R8_USCALED:
		case VK_FORMAT_R8_SSCALED:
		case VK_FORMAT_R8_UINT:
		case VK_FORMAT_R8_SINT:
		case VK_FORMAT_R8_SRGB:
			return 1;
		case VK_FORMAT_R8G8_UNORM:
		case VK_FORMAT_R8G8_SNORM:
		case VK_FORMAT_R8G8_USCALED:
		case VK_FORMAT_R8G8_SSCALED:
		case VK_FORMAT_R8G8_UINT:
		case VK_FORMAT_R8G8_SINT:
		case VK_FORMAT_R8G8_SRGB:
		case VK_FORMAT_R16_UNORM:
		case VK_FORMAT_R16_SNORM:
		case VK_FORMAT_R16_USCALED:
		case VK_FORMAT_R16_SSCALED:
		case VK_FORMAT_R16_UINT:
		case VK_FORMAT_R16_SINT:
		case VK_FORMAT_R16_SFLOAT:
			return 2;
		case VK_FORMAT_R8G8B8_UNORM:
		case VK_FORMAT_R8G8B8_SNORM:
		case VK_FORMAT_R8G8B8_USCALED:
		case VK_FORMAT_R8G8B8_SSCALED:
		case VK_FORMAT_R8G8B8_UINT:
		case VK_FORMAT_R8G8B8_SINT:
		case VK_FORMAT_R8G8B8_SRGB:
		case VK_FORMAT_B8G8R8_UNORM:
		case VK_FORMAT_B8G8R8_SNORM:
		case VK_FORMAT_B8G8R8_USCALED:
		case VK_FORMAT_B8G8R8_SSCALED:
		case VK_FORMAT_B8G8R8_UINT:
		case VK_FORMAT_B8G8R8_SINT:
		case VK_FORMAT_B8G8R8_SRGB:
			return 3;
		case VK_FORMAT_R8G8B8A8_UNORM:
		case VK_FORMAT_R8G8B8A8_SNORM:
		case VK_FORMAT_R8G8B8A8_USCALED:
		case VK_FORMAT_R8G8B8A8_SSCALED:
		case VK_FORMAT_R8G8B8A8_UINT:
		case VK_FORMAT_R8G8B8A8_SINT:
		case VK_FORMAT_R8G8B8A8_SRGB:
		case VK_FORMAT_B8G8R8A8_UNORM:
		case VK_FORMAT_B8G8R8A8_SNORM:
		case VK_FORMAT_B8G8R8A8_USCALED:
		case VK_FORMAT_B8G8R8A8_SSCALED:
		case VK_FORMAT_B8G8R8A8_UINT:
		case VK_FORMAT_B8G8R8A8_SINT:
		case VK_FORMAT_B8G8R8A8_SRGB:
		case VK_FORMAT_R16G16_UNORM:
		case VK_FORMAT_R16G16_SNORM:
		case VK_FORMAT_R16G16_USCALED:
		case VK_FORMAT_R16G16_SSCALED:
		case VK_FORMAT_R16G16_UINT:
		case VK_FORMAT_R16G16_SINT:
		case VK_FORMAT_R16G16_SFLOAT:
		case VK_FORMAT_R32_UINT:
		case VK_FORMAT_R32_SINT:
		case VK_FORMAT_R32_SFLOAT:
			return 4;
		case VK_FORMAT_R16G16B16_UNORM:
		case VK_FORMAT_R16G16B16_SNORM:
		case VK_FORMAT_R16G16B16_USCALED:
		case VK_FORMAT_R16G16B16_SSCALED:
		case VK_FORMAT_R16G16B16_UINT:
		case VK_FORMAT_R16G16B16_SINT:
		case VK_FORMAT_R16G16B16_SFLOAT:
			return 6;
		case VK_FORMAT_R16G16B16A16_UNORM:
		case VK_FORMAT_R16G16B16A16_SNORM:
		case VK_FORMAT_R16G16B16A16_USCALED:
		case VK_FORMAT_R16G16B16A16_SSCALED:
		case VK_FORMAT_R16G16B16A16_UINT:
		case VK_FORMAT_R16G16B16A16_SINT:
		case VK_FORMAT_R16G16B16A16_SFLOAT:
		case VK_FORMAT_R32G32_UINT:
		case VK_FORMAT_R32G32_SINT:
		case VK_FORMAT_R32G32_SFLOAT:
		case VK_FORMAT_R64_UINT:
		case VK_FORMAT_R64_SINT:
		case VK_FORMAT_R64_SFLOAT:
			return 8;
		case VK_FORMAT_R32G32B32_UINT:
		case VK_FORMAT_R32G32B32_SINT:
		case VK_FORMAT_R32G32B32_SFLOAT:
			return 12;
		case VK_FORMAT_R32G32B32A32_UINT:
		case VK_FORMAT_R32G32B32A32_SINT:
		case VK_FORMAT_R32G32B32A32_SFLOAT:
		case VK_FORMAT_R64G64_UINT:
		case VK_FORMAT_R64G64_SINT:
		case VK_FORMAT_R64G64_SFLOAT:
			return 16;
		case VK_FORMAT_R64G64B64_UINT:
		case VK_FORMAT_R64G64B64_SINT:
		case VK_FORMAT_R64G64B64_SFLOAT:
			return 24;
		case VK_FORMAT_R64G64B64A64_UINT:
		case VK_FORMAT_R64G64B64A64_SINT:
		case VK_FORMAT_R64G64B64A64_SFLOAT:
			return 32;
		case VK_FORMAT_D16_UNORM:
		case VK_FORMAT_D32_SFLOAT:
		case VK_FORMAT_S8_UINT:
		case VK_FORMAT_D16_UNORM_S8_UINT:
		case VK_FORMAT_D24_UNORM_S8_UINT:
		case VK_FORMAT_D32_SFLOAT_S8_UINT:
		default:
			return -1;
		}
	}

	static constexpr VkExtent3D extent3d(VkExtent2D extent) {
		return VkExtent3D{.width = extent.width, .height = extent.height, .depth = 1};
	}

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