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
#include <Fission/base/color.hpp>
#include <Fission/base/version.hpp>
#include <Fission/base/array.hpp>
#include <Fission/base/rect.hpp>
#include <Fission/platform.hpp>

// reduce warning level, bc vk_mem_alloc has lots of warnings...
#if defined(FISSION_COMPILER_MSVC)
#	pragma warning (push, 0)
#elif defined(FISSION_COMPILER_GCC) || defined(FISSION_COMPILER_CLANG)
#   pragma GCC diagnostic push
    FISSION_DISABLE_WARNING("-Weverything")
#endif

#include <vma/vk_mem_alloc.h>
#include <glm/mat2x2.hpp>

#if defined(FISSION_COMPILER_MSVC)
#	pragma warning (pop)
#elif defined(FISSION_COMPILER_GCC) || defined(FFISSION_COMPILER_CLANG)
#   pragma GCC diagnostic pop
#endif

#include <vector>

// Must have VK_IMAGE_USAGE_TRANSFER_SRC_BIT for screenshots (for now)
#define FISSION_DEFAULT_SWAP_CHAIN_USAGE \
VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT

__FISSION_BEGIN__

struct MSAA_Info {
	VkSampleCountFlagBits sampleCount;
	VkImage       image;
	VmaAllocation allocation;
	VkImageView   imageview;
};

struct Render_Context {
	VkFramebuffer   frame_buffer;
	VkCommandBuffer command_buffer;
	u32             frame;
	u32             image_index;
};

struct Graphics_Extra {
	struct Queue_Families {
		u32 graphics;
		u32 present;
		u32 transfer;
	} queue_family;
};

enum Graphics_Present_Mode {
	// does not wait for vsync (high power usage)
	Present_Immediate            = VK_PRESENT_MODE_IMMEDIATE_KHR,
	// no vsync, but without tearing (high power usage)
	Present_Immediate_No_Tearing = VK_PRESENT_MODE_MAILBOX_KHR,
	// regular vsync (low power usage)
	Present_V_Sync               = VK_PRESENT_MODE_FIFO_KHR,
	// waits for vsync, but presents immediately when vsync is missed (low power usage)
	Present_Partial_V_Sync       = VK_PRESENT_MODE_FIFO_RELAXED_KHR,
};

struct Graphics
{
	void upload (VkBuffer destination, void const* data, VkDeviceSize size);

	void upload (
		VkImage       destination,
		void const*   image_data,
		VkExtent3D    extent,
		VkFormat      format,
		VkImageLayout final_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		u32           layer = 0
	);

	array<VkPresentModeKHR> supported_present_modes() { return {}; }

	version api_version();

	auto pre_rotation () -> glm::mat2;

	inline constexpr v2u32 size() const {
		u32 w = sc_extent.width;
		u32 h = sc_extent.height;
		return sc_transform
		& (VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR|VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR)
		? v2u32{h, w} : v2u32{w, h};
	}

	inline constexpr v2f32 render_size() const {
		auto x = float(sc_extent.width);
		auto y = float(sc_extent.height);
		return { x, y };
	}

	VkInstance       instance         {};
	VkPhysicalDevice physical_device  {};
	VkDevice         device           {};
	VkQueue          graphics_queue   {};
	VkQueue          present_queue    {};
	VkSurfaceKHR     surface          {};
	VkSwapchainKHR   swap_chain       {};

	// "sc" <=> "swap chain"
	VkExtent2D        sc_extent       {};
	VkFormat          sc_format       {};
	VkImageUsageFlags sc_image_usage  {};
    u32               sc_image_count  {0};
    VkPresentModeKHR  sc_present_mode {};
    VkSurfaceTransformFlagBitsKHR sc_transform {};
    VkImage*          sc_images       {};
	VkImageView*      sc_image_views  {};

	// Main graphics command pool
	VkCommandPool    command_pool {};

	VkQueue          transfer_queue {};
	VkCommandPool    transfer_command_pool {};

	// Primary Command Buffers
	VkCommandBuffer  command_buffers          [2] {};
	VkFence          cb_fences                [2] {};
	VkSemaphore      sc_image_write_semaphore [2] {};
	VkSemaphore      sc_image_read_semaphore  [2] {};

	VmaAllocator     allocator {};

    // only valid when graphics debugging
	VkDebugUtilsMessengerEXT debug_messenger {};

    Graphics_Extra   extra {};

	Graphics() = default;
	Graphics(Graphics const&) = delete;

private:
	friend struct Engine;

	auto create(struct Graphics_Create_Info const& info) -> bool; // SUCCESS == false
	void destroy();

private:
    // SUCCESS == false
    bool create_instance        (bool debug);
    bool create_surface         (struct Window* window);
    bool pick_physical_device   ();
    bool pick_queue_families    ();
    bool create_device          (bool debug);
    bool create_allocator       ();
    bool create_swap_chain      (struct Window* window);
    bool create_sc_image_views  ();
    bool create_command_buffers ();
    bool create_sync_objects    ();
};

extern void set_viewport_and_scissor(VkCommandBuffer cmd, rf32 rect);

// TODO: Templating this makes no sense, please refactor... anytime now!
template <VkShaderStageFlags ShaderStage, VkDescriptorType DescriptorType>
struct Single_Descriptor_Set_Layout {
	Single_Descriptor_Set_Layout() = default;
	Single_Descriptor_Set_Layout(Graphics& gfx) { create(gfx); }
	inline VkResult create(Graphics& gfx) {
		VkDescriptorSetLayoutBinding binding {
			.binding = 0,
			.descriptorCount = 1,
			.descriptorType = DescriptorType,
			.stageFlags = ShaderStage,
			.pImmutableSamplers = nullptr,
		};
		VkDescriptorSetLayoutCreateInfo descriptorInfo {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.bindingCount = 1,
			.pBindings = &binding,
		};
		return vkCreateDescriptorSetLayout(gfx.device, &descriptorInfo, nullptr, &handle);
	}
	inline void destroy(Graphics& gfx) {
		vkDestroyDescriptorSetLayout(gfx.device, handle, nullptr);
	}
	inline operator VkDescriptorSetLayout() { return handle; }
	inline VkDescriptorSetLayout* operator &() { return &handle; }
	VkDescriptorSetLayout handle;
};

//! @TODO: vvv  rename this
using Transform_2D_Layout = Single_Descriptor_Set_Layout<VK_SHADER_STAGE_VERTEX_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER>;
using Texture_Layout      = Single_Descriptor_Set_Layout<VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER>;

struct Transform_2D_Data {
	v2f32 scale;
	v2f32 offset;
};

__FISSION_END__

#define FS_VK_BIND_DESCRIPTOR_SETS(COMMAND_BUFFER, PIPELINE_LAYOUT, SET_COUNT, SETS) \
vkCmdBindDescriptorSets(COMMAND_BUFFER, VK_PIPELINE_BIND_POINT_GRAPHICS, PIPELINE_LAYOUT, 0, SET_COUNT, SETS, 0, nullptr)

namespace vk
{
	template <uint32_t variant, uint32_t major, uint32_t minor, uint32_t patch>
	inline constexpr uint32_t make_api_version =
		((((uint32_t)variant) << 29U)
	     | (((uint32_t)major) << 22U)
		 | (((uint32_t)minor) << 12U)
		 | ((uint32_t)patch));

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

	template <>	struct _format_of<fs::rgba8> { static constexpr VkFormat value = VK_FORMAT_R8G8B8A8_UNORM; };
	template <>	struct _format_of<fs::rgba>  { static constexpr VkFormat value = VK_FORMAT_R32G32B32A32_SFLOAT; };
	template <>	struct _format_of<fs::rgb>   { static constexpr VkFormat value = VK_FORMAT_R32G32B32_SFLOAT; };
	template <>	struct _format_of<fs::v4f32> { static constexpr VkFormat value = VK_FORMAT_R32G32B32A32_SFLOAT; };
	template <>	struct _format_of<fs::v3f32> { static constexpr VkFormat value = VK_FORMAT_R32G32B32_SFLOAT; };
	template <>	struct _format_of<fs::v2f32> { static constexpr VkFormat value = VK_FORMAT_R32G32_SFLOAT; };
	template <>	struct _format_of<fs::f32>   { static constexpr VkFormat value = VK_FORMAT_R32_SFLOAT; };
	template <>	struct _format_of<fs::s32>   { static constexpr VkFormat value = VK_FORMAT_R32_SINT; };
	template <>	struct _format_of<fs::u32>   { static constexpr VkFormat value = VK_FORMAT_R32_UINT; };

	template <typename T> static constexpr VkFormat format_of = _format_of<T>::value;

	template <typename T, size_t Count>
	static constexpr uint32_t count(T const(&)[Count]) { return (uint32_t)Count; }

	inline VkResult begin(VkCommandBuffer command_buffer, VkCommandBufferUsageFlags flags = 0)
	{
		VkCommandBufferBeginInfo begin_info {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = flags,
		};
		return vkBeginCommandBuffer(command_buffer, &begin_info);
	}

	inline VkResult end(VkCommandBuffer command_buffer)
	{
		return vkEndCommandBuffer(command_buffer);
	}
	
	FISSION_API void begin(VkCommandBuffer command_buffer, VkRenderPass render_pass, VkFramebuffer frame_buffer, VkClearColorValue color = {});

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
			return 0;
		}
	}

	enum Shader_Stage {
		Vertex = VK_SHADER_STAGE_VERTEX_BIT,
		Fragment = VK_SHADER_STAGE_FRAGMENT_BIT,
		Geometry = VK_SHADER_STAGE_GEOMETRY_BIT,
	};

    namespace embed {
        struct Vertex_Shader {
            static constexpr auto stage = Shader_Stage::Vertex;
        };
        struct Fragment_Shader {
            static constexpr auto stage = Shader_Stage::Fragment;
        };
    }

	static constexpr VkExtent3D extent3d(VkExtent2D extent) {
		return VkExtent3D{ .width = extent.width, .height = extent.height, .depth = 1 };
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

	FISSION_API VkShaderModule create_shader(size_t size, void const* data);


	struct Render_Pass_Creator {
		std::vector<VkAttachmentDescription> attachments;
		std::vector<VkAttachmentReference> attachment_references;
		std::vector<VkSubpassDescription> subpasses;
		std::vector<VkSubpassDependency> subpass_dependencies;

		Render_Pass_Creator(size_t attachment_reference_count) {
			attachment_references.reserve(attachment_reference_count);
		}

		Render_Pass_Creator& add_external_subpass_dependency(uint32_t subpass) {
			VkSubpassDependency dependency{};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = subpass;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			subpass_dependencies.emplace_back(dependency);
			return *this;
		}

		VkPipelineStageFlags pick_stage_mask_from_access_mask(VkAccessFlags access) {
			switch (access)
			{
			case VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT:         return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			case VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT: return VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			case VK_ACCESS_SHADER_READ_BIT:                    return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			default:                                           return 0;
			}
		}

		Render_Pass_Creator& add_dependency(uint32_t src_subpass, uint32_t dst_subpass, VkAccessFlags src_access, VkAccessFlags dst_access) {
			VkSubpassDependency dependency{};
			dependency.srcSubpass = src_subpass;
			dependency.dstSubpass = dst_subpass;
			dependency.srcStageMask = pick_stage_mask_from_access_mask(src_access);
			dependency.srcAccessMask = src_access;
			dependency.dstStageMask = pick_stage_mask_from_access_mask(dst_access);
			dependency.dstAccessMask = dst_access;
			subpass_dependencies.emplace_back(dependency);
			return *this;
		}

		Render_Pass_Creator& add_subpass(std::initializer_list<VkAttachmentReference> const& refs) {
			VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = (fs::u32)refs.size();
			subpass.pColorAttachments = attachment_references.data() + attachment_references.size();
			for (auto&& ref : refs) attachment_references.emplace_back(ref);
			subpasses.emplace_back(subpass);
			return *this;
		}
		Render_Pass_Creator& add_subpass_input(std::initializer_list<VkAttachmentReference> const& refs, std::initializer_list<VkAttachmentReference> const& input_refs) {
			VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = (fs::u32)refs.size();
			subpass.inputAttachmentCount = (fs::u32)input_refs.size();
			subpass.pColorAttachments = attachment_references.data() + attachment_references.size();
			for (auto&& ref : refs) attachment_references.emplace_back(ref);
			subpass.pInputAttachments = attachment_references.data() + attachment_references.size();
			for (auto&& ref : input_refs) attachment_references.emplace_back(ref);
			subpasses.emplace_back(subpass);
			return *this;
		}

		Render_Pass_Creator& add_subpass(std::initializer_list<VkAttachmentReference> const& refs, VkAttachmentReference depth_ref) {
			VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = (fs::u32)refs.size();
			subpass.pColorAttachments = attachment_references.data() + attachment_references.size();
			//	subpass.pResolveAttachments = &colorAttachmentResolveRef; // MSAA
			for (auto&& ref : refs) attachment_references.emplace_back(ref);
			subpass.pDepthStencilAttachment = attachment_references.data() + attachment_references.size();
			attachment_references.emplace_back(depth_ref);
			subpasses.emplace_back(subpass);
			return *this;
		}
		Render_Pass_Creator& add_subpass(std::initializer_list<VkAttachmentReference> const& refs, VkAttachmentReference depth_ref, VkAttachmentReference resolve_ref) {
			VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = (fs::u32)refs.size();
			subpass.pColorAttachments = attachment_references.data() + attachment_references.size();
			for (auto&& ref : refs) attachment_references.emplace_back(ref);
			subpass.pDepthStencilAttachment = attachment_references.data() + attachment_references.size();
			attachment_references.emplace_back(depth_ref);
			subpass.pResolveAttachments = attachment_references.data() + attachment_references.size(); // MSAA
			attachment_references.emplace_back(resolve_ref);
			subpasses.emplace_back(subpass);
			return *this;
		}

		Render_Pass_Creator& add_subpass_with_input_attachment(std::initializer_list<VkAttachmentReference> const& refs, VkAttachmentReference input_ref) {
			VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = (fs::u32)refs.size();
			subpass.pColorAttachments = attachment_references.data() + attachment_references.size();
			//	subpass.pResolveAttachments = &colorAttachmentResolveRef; // MSAA
			for (auto&& ref : refs) attachment_references.emplace_back(ref);
			subpass.pInputAttachments = attachment_references.data() + attachment_references.size();
			subpass.inputAttachmentCount = 1;
			attachment_references.emplace_back(input_ref);
			subpasses.emplace_back(subpass);
			return *this;
		}

		VkImageLayout pick_final_image_layout_for_format(VkFormat format) {
			switch (format)
			{
			case VK_FORMAT_D16_UNORM:
			case VK_FORMAT_X8_D24_UNORM_PACK32:
			case VK_FORMAT_D32_SFLOAT:
			case VK_FORMAT_D16_UNORM_S8_UINT:
			case VK_FORMAT_D24_UNORM_S8_UINT:
			case VK_FORMAT_D32_SFLOAT_S8_UINT:   return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			default:                             return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			}
		}

		enum Attachment_Preset {
			// Use this for Depth and Color images that we want to write to
			Attachment_New_Image,        // load = CLEAR, store = STORE, stencil = DONT CARE, inital = UNDEFINED
			Attachment_Cumulative_Image, // load = LOAD , store = STORE, stencil = DONT CARE, inital = UNDEFINED
		};

		Render_Pass_Creator& add_attachment(VkFormat format, VkSampleCountFlagBits sample_count = VK_SAMPLE_COUNT_1_BIT) {
			VkAttachmentDescription attachment{};
			attachment.format = format;
			attachment.samples = sample_count;
			attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachment.finalLayout = pick_final_image_layout_for_format(format);
			attachments.emplace_back(attachment);
			return *this;
		}
		Render_Pass_Creator& add_attachment(VkFormat format, VkAttachmentLoadOp loadOp, VkImageLayout layout, VkSampleCountFlagBits sample_count = VK_SAMPLE_COUNT_1_BIT) {
			VkAttachmentDescription attachment{};
			attachment.format = format;
			attachment.samples = sample_count;
			attachment.loadOp = loadOp;
			attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachment.initialLayout = layout;
			attachment.finalLayout = layout;
			attachments.emplace_back(attachment);
			return *this;
		}

		Render_Pass_Creator& add_attachment(
			VkFormat format,
			VkImageLayout initial_layout,
			VkImageLayout final_layout,
			VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
			VkSampleCountFlagBits sample_count = VK_SAMPLE_COUNT_1_BIT
		) {
			attachments.emplace_back(VkAttachmentDescription {
				.format = format,
				.samples = sample_count,
				.loadOp = loadOp,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout = initial_layout,
				.finalLayout = final_layout,
			});
			return *this;
		}

		VkResult create(VkRenderPass* pRenderPass);
	};

	struct Pipeline_Creator {
		std::vector<VkDynamicState>                  dynamic_states;
		std::vector<VkPipelineShaderStageCreateInfo> shaders;
		VkPipelineColorBlendAttachmentState          blend_attachment = {
			.colorWriteMask = 0b1111,
		};
		VkPipelineVertexInputStateCreateInfo const* vertex_input_state;
		VkPipelineInputAssemblyStateCreateInfo      input_assembly_state{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.primitiveRestartEnable = VK_FALSE,
		};
		VkPipelineViewportStateCreateInfo           viewport_state = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			.viewportCount = 1,
			.scissorCount = 1,
		};
		VkPipelineRasterizationStateCreateInfo      rasterization_state{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.depthClampEnable = VK_FALSE,
			.rasterizerDiscardEnable = VK_FALSE,
			.polygonMode = VK_POLYGON_MODE_FILL,
			.cullMode = VK_CULL_MODE_NONE,
			.frontFace = VK_FRONT_FACE_CLOCKWISE,
			.depthBiasEnable = VK_FALSE,
			.lineWidth = 1.0f,
		};
		VkPipelineMultisampleStateCreateInfo        multisample_state = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
			.sampleShadingEnable = VK_FALSE,
		};
		VkPipelineDepthStencilStateCreateInfo       depth_stencil_state = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			.depthTestEnable = VK_TRUE,
			.depthWriteEnable = VK_TRUE,
			.depthCompareOp = VK_COMPARE_OP_LESS,
			.depthBoundsTestEnable = VK_FALSE,
			.stencilTestEnable = VK_FALSE,
			.minDepthBounds = 0.0f,
			.maxDepthBounds = 1.0f,
		};
		VkPipelineColorBlendStateCreateInfo         color_blend_state = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.logicOpEnable = VK_FALSE,
			.attachmentCount = 1,
			.pAttachments = nullptr,
		};
		VkPipelineDynamicStateCreateInfo            dynamic_state{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
		VkPipelineLayout                            layout;
		VkRenderPass                                render_pass;
		uint32_t                                    subpass;

		Pipeline_Creator(VkRenderPass render_pass, VkPipelineLayout layout, uint32_t subpass = 0u) {
			this->layout = layout;
			this->render_pass = render_pass;
			this->subpass = subpass;
		}

		Pipeline_Creator& vertex_input(VkPipelineVertexInputStateCreateInfo const* vi) {
			vertex_input_state = vi;
			return *this;
		}

		Pipeline_Creator& sample_count(VkSampleCountFlagBits sample_count) {
			multisample_state.rasterizationSamples = sample_count;
			return *this;
		}
		Pipeline_Creator& topology(VkPrimitiveTopology topology) {
			input_assembly_state.topology = topology;
			return *this;
		}

		Pipeline_Creator& add_dynamic_state(VkDynamicState state) {
			dynamic_states.emplace_back(state);
			return *this;
		}

		template <typename Shader>
		Pipeline_Creator& add_shader() {
			VkPipelineShaderStageCreateInfo info{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
			info.module = create_shader(Shader::size, Shader::data);
			info.stage = static_cast<VkShaderStageFlagBits>(Shader::stage);
			info.pName = "main";
			shaders.emplace_back(info);
			return *this;
		}

		VkResult create_and_destroy_shaders(VkPipeline* pipeline);
		VkResult create(VkPipeline* pipeline);
		VkResult create_no_fragment(VkPipeline* pipeline);
	};

	struct Pipeline_Layout_Creator {
		std::vector<VkDescriptorSetLayout> layouts;
		std::vector<VkPushConstantRange> push_ranges;

		Pipeline_Layout_Creator& add_layout(VkDescriptorSetLayout layout) {
			layouts.emplace_back(layout);
			return *this;
		}
		Pipeline_Layout_Creator& add_push_range(VkShaderStageFlags stage, fs::u32 size, fs::u32 offset = 0u) {
			VkPushConstantRange range;
			range.offset = offset;
			range.size = size;
			range.stageFlags = stage;
			push_ranges.emplace_back(range);
			return *this;
		}
		VkResult create(VkPipelineLayout* pLayout);
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
