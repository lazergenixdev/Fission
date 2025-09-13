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
#include "os.hpp"
#include "vulkan/vulkan.h"
DISABLE_ALL_WARNINGS_BEGIN
#include "vk_mem_alloc.h"
DISABLE_ALL_WARNINGS_END

// --------------------------------------------------------------------------------
// OS Types

namespace fission
{
	using os::Mutex;
	using os::File;
}

// --------------------------------------------------------------------------------
// Time

namespace fission
{
	auto ticks() -> u64;
	auto seconds_elapsed_and_reset(u64& ticks) -> f64;

	struct Logging_Timestamp
	{
		u32 date; // 0xYEARMMDD
		u32 time; // 0x00HHMMSS
		u32 milliseconds;

		static Logging_Timestamp now();
	};

	inline void format_single(Arena& arena, Logging_Timestamp ts)
	{
		auto p = reinterpret_cast<c8*>(arena.start);
		u32 year = (ts.date>>16)&0xFFFF, month = (ts.date>>8)&0xFF, day = ts.date&0xFF;
		p[arena.allocated++] = c8('0' + (year) / 1000);
		p[arena.allocated++] = c8('0' + (year / 100) % 10);
		p[arena.allocated++] = c8('0' + (year / 10) % 10);
		p[arena.allocated++] = c8('0' + (year) % 10);
		p[arena.allocated++] = c8('-');
		p[arena.allocated++] = c8('0' + (month) / 10);
		p[arena.allocated++] = c8('0' + (month) % 10);
		p[arena.allocated++] = c8('-');
		p[arena.allocated++] = c8('0' + (day) / 10);
		p[arena.allocated++] = c8('0' + (day) % 10);
		p[arena.allocated++] = c8(' ');

		u32 hour = (ts.time>>16)&0xFF, min = (ts.time>>8)&0xFF, sec = ts.time&0xFF;
		p[arena.allocated++] = c8('0' + (hour) / 10);
		p[arena.allocated++] = c8('0' + (hour) % 10);
		p[arena.allocated++] = c8(':');
		p[arena.allocated++] = c8('0' + (min) / 10);
		p[arena.allocated++] = c8('0' + (min) % 10);
		p[arena.allocated++] = c8(':');
		p[arena.allocated++] = c8('0' + (sec) / 10);
		p[arena.allocated++] = c8('0' + (sec) % 10);
		p[arena.allocated++] = c8('.');
		p[arena.allocated++] = c8('0' + (ts.milliseconds / 100) % 10);
		p[arena.allocated++] = c8('0' + (ts.milliseconds / 10) % 10);
		p[arena.allocated++] = c8('0' + (ts.milliseconds) % 10);
	}
}

// --------------------------------------------------------------------------------
// Logging

namespace fission
{
	struct Logger
	{
		Arena       arena;
		Mutex       mutex;
		File        backing_file;
		int         minimum_level;
		const char* prefix; // Prefix to put before message
	};
	
	global Logger logger;
}
namespace fission::log
{
    enum {
        Verbose = 0,
        Debug   = 1,
        Info    = 2, // Default
        Warn    = 3,
        Error   = 4,
        LEVEL_COUNT
    };

	void write_log_from_logger(int level);

	template <typename...T>
    inline void log(int level, T&&...args)
	{
		using namespace formatting;
		static const string level_strings [] {
			"  VERBOSE  ",
			"    DEBUG  ",
			"     INFO  ",
			"     WARN  ",
			"    ERROR  ",
		};
		// Note: could probably do better than using locks
		os_mutex_lock(logger.mutex);
		logger.arena.reset();
    	format(logger.arena, Logging_Timestamp::now(), level_strings[level]);
		if (logger.prefix) format(logger.arena, "(", logger.prefix, ") ");
		(format_single(logger.arena, std::forward<T>(args)), ...);
		format(logger.arena, null); // null terminate in case we use C functions
		write_log_from_logger(level);
		os_mutex_unlock(logger.mutex);
	}
	
    template <typename...T> inline void verbose (T&&...args) { log(Verbose, std::forward<T>(args)...); }
    template <typename...T> inline void debug   (T&&...args) { log(Debug  , std::forward<T>(args)...); }
    template <typename...T> inline void info    (T&&...args) { log(Info   , std::forward<T>(args)...); }
    template <typename...T> inline void warn    (T&&...args) { log(Warn   , std::forward<T>(args)...); }
    template <typename...T> inline void error   (T&&...args) { log(Error  , std::forward<T>(args)...); }
}

// --------------------------------------------------------------------------------
// Displays

namespace fission
{
	struct Display_Mode {
		v2u32 resolution;
		int   refresh_rate;
	};

	enum Display_Index_: int
	{
		Display_Index_Primary   = 0,
		Display_Index_Automatic = -1,
	};

	struct Display : public os::Display
	{
		int index;
		c8 name_buffer[64];
		int name_count;
		rs32 rect;

		string name() const { return {name_buffer, (size_t)name_count}; }

		auto current_mode() const -> Display_Mode;

	//	auto supported_display_modes() const -> array<Display_Mode>;

		auto set_display_mode(const Display_Mode *) -> Result; // TODO: Move to Window?
		
		auto revert_display_mode() -> Result;
	};
}

// --------------------------------------------------------------------------------
// Events

namespace fission
{
	enum EventType: u8 {
		Event_Key_Down,
		Event_Key_Up,
		Event_Focus_Lost,
		Event_Character_Input,
		Event_Mouse_Move,
	};

	struct Event {
		s64 timestamp;
		u8 type;

		union {
			struct {
				u32 key_id;
				u32 repeat_count;
			} key_down;

			struct {
				u32 key_id;
			} key_up;

			struct {
				c32 codepoint;
			} character_input;

			struct {
				v2s32 delta;
			} mouse_move;
		};
	};
}
 
// --------------------------------------------------------------------------------
// Window
  
namespace fission
{
	enum Window_Mode: u32 {
		Windowed             = 0x01,
		Windowed_Fullscreen  = 0x02,
		Exclusive_Fullscreen = 0x04,
		Windowed_Resizeable  = 0x08,
	};
	
	struct Window : public os::Window
	{
		v2s32        mouse_position       {};
		Window_Mode  mode                 {Windowed_Fullscreen};
		bool         use_mouse_deltas     {false};
		//! TODO: this should be a data structure
		Event        event_queue   [1024] {};
		u32          event_head           {};
		u32          event_tail           {};
		bool         event_queue_overflow {false};

	public:
		// @see enum Window_Mode
		auto supported_modes() -> u32;

		//void set_title(string const& title);
		//bool is_minimized();
		//void set_mode(Window_Mode mode);

		inline void toggle_using_mouse_deltas() {
			set_using_mouse_delta(!use_mouse_deltas);
		}

		void set_using_mouse_delta(bool use);

	private:
		struct Create_Info {
			string title;
			u32 width;
			u32 height;
		};

		auto create(Create_Info const& info) -> Result;

		auto pop_all_events(Arena& arena) -> array<Event>;

		// Closes the window => causes engine to stop running => Application closes
		void close();

		~Window();

		friend struct Engine;
	};
}

// --------------------------------------------------------------------------------
// Graphics

namespace fission
{
	struct Queue_Families {
		u32 graphics;
		u32 present;
		u32 transfer;
	};

	struct Render_Context {
		VkFramebuffer   frame_buffer;
		VkCommandBuffer command_buffer;
		u32             frame;
		u32             image_index;
	};

// --------------------------------------------------------------------------------
// Main Graphics Context

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

	//	array<VkPresentModeKHR> supported_present_modes() { return {}; }
	//	version api_version();
	//	inline constexpr v2u32 size();

	//	auto pre_rotation () -> glm::mat2;
		inline auto render_size() -> vec2; // Physical size of the swap chain images
		inline void set_default_viewport(VkCommandBuffer cmd);
		inline void set_default_scissor(VkCommandBuffer cmd);

		static constexpr int MAX_FRAMES_IN_FLIGHT  = 2;
		static constexpr int MAX_SWAP_CHAIN_IMAGES = 8;

		// Shorthand
		static constexpr int F = MAX_FRAMES_IN_FLIGHT;
		static constexpr int M = MAX_SWAP_CHAIN_IMAGES;

		//! NOTE: VK_IMAGE_USAGE_TRANSFER_SRC_BIT must be set on swapchain to take screenshots

		VkInstance                     instance                    {};
		VkPhysicalDevice               physical_device             {};
		VkDevice                       device                      {};
		VkQueue                        graphics_queue              {};
		VkQueue                        present_queue               {};
		VkSurfaceKHR                   surface                     {};
		VkSwapchainKHR                 swap_chain                  {};
		VkExtent2D                     extent                      {}; // Swap Chain
		VkFormat                       format                      {}; // Swap Chain
    	VkColorSpaceKHR                color_space                 {}; // Swap Chain
		VkImageUsageFlags              image_usage                 {VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT|VK_IMAGE_USAGE_TRANSFER_SRC_BIT}; // Swap Chain
		u32                            image_count                 {0}; // Swap Chain
		VkPresentModeKHR               present_mode                {VK_PRESENT_MODE_FIFO_KHR}; // Swap Chain
		VkSurfaceTransformFlagBitsKHR  transform                   {}; // Swap Chain
		VkImage                        images                  [M] {}; // Swap Chain
		VkImageView                    image_views             [M] {}; // Swap Chain
		VkCommandPool                  command_pool                {};
		VkQueue                        transfer_queue              {};
		VkCommandPool                  transfer_command_pool       {};
		VkCommandBuffer                command_buffers         [F] {}; // Max 2 frames in flight
		VkFence                        fences                  [F] {};
		VkSemaphore                    image_ready_semaphore   [F] {}; // Signalled when image is ready to be rendered to
		VkSemaphore                    present_ready_semaphore [M] {}; // Signalled when image is ready to be presented
		VmaAllocator                   allocator                   {};
		VkDebugUtilsMessengerEXT       debug_messenger             {};
		Queue_Families                 queue_family                {};


		Graphics() = default;
		Graphics(Graphics const&) = delete;

	private:
		friend struct Engine;

		struct Create_Info {
			Window* window;
			bool debug;
		};

		auto create(Create_Info const& info) -> Result;
		void destroy();

	private:
		auto create_instance        (bool debug) -> Result;
		auto create_surface         (struct Window* window) -> Result;
		auto pick_physical_device   () -> Result;
		auto pick_queue_families    () -> Result;
		auto create_device          (bool debug) -> Result;
		auto create_allocator       () -> Result;
		auto create_swap_chain      (struct Window* window) -> Result;
		auto create_sc_image_views  () -> Result;
		auto create_command_buffers () -> Result;
		auto create_sync_objects    () -> Result;
	};

	inline auto begin(VkCommandBuffer command_buffer, VkCommandBufferUsageFlags flags = 0) -> VkResult
	{
		VkCommandBufferBeginInfo begin_info {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = flags,
		};
		return vkBeginCommandBuffer(command_buffer, &begin_info);
	}

	enum Attachment_Preset {
		// Use this for Depth and Color images that we want to write to
		Attachment_Preset_New_Image_Present, // load = CLEAR, store = STORE, stencil = DONT CARE, inital = UNDEFINED, final = PRESENT_SRC_KHR
		Attachment_Preset_New_Image,         // load = CLEAR, store = STORE, stencil = DONT CARE, inital = UNDEFINED
		Attachment_Preset_Cumulative_Image,  // load = LOAD , store = STORE, stencil = DONT CARE, inital = UNDEFINED
	};

// --------------------------------------------------------------------------------
// Render Pass Creator

	struct Render_Pass_Creator {
		Arena arena;
		array<VkAttachmentReference> attachment_references {};
		array<VkAttachmentDescription> attachments {};
		array<VkSubpassDescription> subpasses {};
		array<VkSubpassDependency> subpass_dependencies {};

		static constexpr size_t max_attachment_reference_count = 16;

		// Helpers
		VkPipelineStageFlags pick_stage_mask_from_access_mask(VkAccessFlags access);
		VkImageLayout pick_final_image_layout_for_format(VkFormat format);

		Render_Pass_Creator(Arena& arena = temp_arena());

		Render_Pass_Creator& add_external_subpass_dependency(uint32_t subpass);
		Render_Pass_Creator& add_dependency(uint32_t src_subpass, uint32_t dst_subpass, VkAccessFlags src_access, VkAccessFlags dst_access);
		Render_Pass_Creator& add_subpass(std::initializer_list<VkAttachmentReference> const& refs);
		Render_Pass_Creator& add_attachment(VkFormat format, Attachment_Preset preset, VkSampleCountFlagBits sample_count = VK_SAMPLE_COUNT_1_BIT);

		VkResult create(VkRenderPass* pRenderPass);
	};

// --------------------------------------------------------------------------------
// Data Type => Vulkan Type

	//! TODO: move this to base
	union rgba8
	{
		struct {
			u8 r, g, b, a;
		};
		u32 value;
		
		rgba8(u8 r, u8 g, u8 b, u8 a = 0xFF)
		:	r(r), g(g), b(b), a(a)
		{}
	};

	template <typename T> struct _vk_format_of {
		static constexpr VkFormat value = VK_FORMAT_UNDEFINED;
	};

	template <>	struct _vk_format_of<rgba8> { static constexpr VkFormat value = VK_FORMAT_R8G8B8A8_UNORM;      };
	template <>	struct _vk_format_of<v4f32> { static constexpr VkFormat value = VK_FORMAT_R32G32B32A32_SFLOAT; };
	template <>	struct _vk_format_of<v3f32> { static constexpr VkFormat value = VK_FORMAT_R32G32B32_SFLOAT;    };
	template <>	struct _vk_format_of<v2f32> { static constexpr VkFormat value = VK_FORMAT_R32G32_SFLOAT;       };
	template <>	struct _vk_format_of<f32>   { static constexpr VkFormat value = VK_FORMAT_R32_SFLOAT;          };
	template <>	struct _vk_format_of<s32>   { static constexpr VkFormat value = VK_FORMAT_R32_SINT;            };
	template <>	struct _vk_format_of<u32>   { static constexpr VkFormat value = VK_FORMAT_R32_UINT;            };

	template <typename T> static constexpr VkFormat vk_format_of = _vk_format_of<T>::value;

// --------------------------------------------------------------------------------
// Vertex Layout

	template <typename V, typename...Attributes>
	struct Vertex_Layout
	{
		static constexpr uint32_t attribute_count = sizeof...(Attributes);

		VkPipelineVertexInputStateCreateInfo info;
		VkVertexInputBindingDescription binding;
		VkVertexInputAttributeDescription attributes[attribute_count];

		inline constexpr Vertex_Layout() noexcept
		{
			using namespace meta;

			info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			info.pNext = nullptr;
			info.flags = 0;
			info.vertexBindingDescriptionCount = 1;
			info.pVertexBindingDescriptions = &binding;
			info.vertexAttributeDescriptionCount = attribute_count;
			info.pVertexAttributeDescriptions = attributes;

			binding.binding = 0;
			binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			binding.stride = size_of_n<attribute_count, Attributes...>;

			[&] <typename T, size_t...n>(std::integer_sequence<T, n...>) {
				((attributes[n].binding = 0), ...);
				((attributes[n].format = vk_format_of<meta::type_at<n, Attributes...>>), ...);
				((attributes[n].location = n), ...);
				(set_offset<n>(), ...);
			} (std::make_index_sequence<attribute_count>{});
		}

		template <size_t n>
		inline constexpr void set_offset()
		{
			using namespace meta;
			attributes[n].offset = size_of_n<(int)n, Attributes...>;
		}
	};

// --------------------------------------------------------------------------------
// Pipeline Creator

	struct Pipeline_Creator
	{
		//! TODO: no std::vector
		std::vector<VkDynamicState> dynamic_states;
		std::vector<VkPipelineShaderStageCreateInfo> shaders;
		VkPipelineColorBlendAttachmentState blend_attachment {
			.colorWriteMask = 0b1111,
		};
		VkPipelineVertexInputStateCreateInfo const* vertex_input_state;
		VkPipelineInputAssemblyStateCreateInfo input_assembly_state {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.primitiveRestartEnable = VK_FALSE,
		};
		VkPipelineViewportStateCreateInfo viewport_state {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			.viewportCount = 1,
			.scissorCount = 1,
		};
		VkPipelineRasterizationStateCreateInfo rasterization_state {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.depthClampEnable = VK_FALSE,
			.rasterizerDiscardEnable = VK_FALSE,
			.polygonMode = VK_POLYGON_MODE_FILL,
			.cullMode = VK_CULL_MODE_NONE,
			.frontFace = VK_FRONT_FACE_CLOCKWISE,
			.depthBiasEnable = VK_FALSE,
			.lineWidth = 1.0f,
		};
		VkPipelineMultisampleStateCreateInfo multisample_state {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
			.sampleShadingEnable = VK_FALSE,
		};
		VkPipelineDepthStencilStateCreateInfo depth_stencil_state {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			.depthTestEnable = VK_TRUE,
			.depthWriteEnable = VK_TRUE,
			.depthCompareOp = VK_COMPARE_OP_LESS,
			.depthBoundsTestEnable = VK_FALSE,
			.stencilTestEnable = VK_FALSE,
			.minDepthBounds = 0.0f,
			.maxDepthBounds = 1.0f,
		};
		VkPipelineColorBlendStateCreateInfo color_blend_state {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.logicOpEnable = VK_FALSE,
			.attachmentCount = 1,
			.pAttachments = nullptr,
		};
		VkPipelineDynamicStateCreateInfo dynamic_state {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO
		};

		template <typename V, typename...T>
		Pipeline_Creator& vertex_layout(Vertex_Layout<V,T...> const& layout)
		{
			vertex_input_state = &layout.info;
			return *this;
		}

		Pipeline_Creator& add_dynamic_state(VkDynamicState state);
		Pipeline_Creator& add_shader(VkShaderStageFlagBits stage, VkShaderModule shader);

		VkResult create(VkPipeline* pPipeline, VkPipelineLayout layout, VkRenderPass render_pass);
	};

// --------------------------------------------------------------------------------
// Draw Data 2D

	struct Draw_Data_2d
	{
		struct vertex
		{
			v2f32 position;
			v2f32 texcoord;
			rgba8 color;

			using Layout = Vertex_Layout<vertex, v2f32, v2f32, rgba8>;
		};

		struct Batch
		{
			u32 vertex_offset;
			u32 vertex_count;
			u32 index_offset;
			u32 index_count;
			VkDescriptorSet texture;
		};
		
		Batch         current;
		Arena         vertex_arena;
		Arena         index_arena;
		VkBuffer      vertex_buffer     [Graphics::MAX_FRAMES_IN_FLIGHT];
		VkBuffer      index_buffer      [Graphics::MAX_FRAMES_IN_FLIGHT];
		VmaAllocation vertex_allocation [Graphics::MAX_FRAMES_IN_FLIGHT];
		VmaAllocation index_allocation  [Graphics::MAX_FRAMES_IN_FLIGHT];

		inline void create(Graphics& graphics, u32 max_vertex_count = 128_KiB, u32 max_index_count = 256_KiB)
		{
			VmaAllocationCreateInfo allocation_info {
				.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
				.usage = VMA_MEMORY_USAGE_AUTO,
			};
			VkBufferCreateInfo buffer_info {
				.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO
			};
			//! TODO: error handling
			forn (Graphics::MAX_FRAMES_IN_FLIGHT)
			{
				buffer_info.size = max_vertex_count * sizeof(vertex);
				buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
				vmaCreateBuffer(graphics.allocator, &buffer_info, &allocation_info, &vertex_buffer[i], &vertex_allocation[i], nullptr);
				buffer_info.size = max_index_count * sizeof(u32);
				buffer_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
				vmaCreateBuffer(graphics.allocator, &buffer_info, &allocation_info, &index_buffer[i], &index_allocation[i], nullptr);
			}
			vertex_arena.create(max_vertex_count * sizeof(vertex));
			index_arena.create(max_index_count * sizeof(u32));
		}

		inline void push_vertex(vertex const& vtx) { vertex_arena.push(vtx); current.vertex_count += 1; }
		inline void push_index(u32 idx)            { index_arena.push(idx); current.index_count += 1; }

		inline void add_triangle(vec2 p0, vec2 p1, vec2 p2, rgba8 color)
		{
			u32 v = current.vertex_count;
			index_arena.push<v3u32>({v, v+1, v+2});
			current.index_count += 3;
			vertex_arena.push<vertex>({.position = p0, .color = color});
			vertex_arena.push<vertex>({.position = p1, .color = color});
			vertex_arena.push<vertex>({.position = p2, .color = color});
			current.vertex_count += 3;
		}

		inline void flush_batch(VkCommandBuffer cmd, u32 frame_index)
		{
			VkDeviceSize offset = 0;
			vkCmdBindIndexBuffer(cmd, index_buffer[frame_index], 0, VK_INDEX_TYPE_UINT32);
			vkCmdBindVertexBuffers(cmd, 0, 1, &vertex_buffer[frame_index], &offset);
			vkCmdDrawIndexed(cmd, current.index_count, 1, current.index_offset, current.vertex_offset, 0);
			current.vertex_offset += current.vertex_count;
			current.index_offset += current.index_count;
			current.vertex_count = 0;
			current.index_count = 0;
			current.texture = VK_NULL_HANDLE;
		}

		inline void send(Graphics& graphics, u32 frame_index)
		{
			vertex* gpu_vertex_memory;
			vmaMapMemory(graphics.allocator, vertex_allocation[frame_index], (void**)&gpu_vertex_memory);
			memcpy(gpu_vertex_memory, vertex_arena.start, current.vertex_offset * sizeof(vertex));
			vmaUnmapMemory(graphics.allocator, vertex_allocation[frame_index]);
			
			u32* gpu_index_memory;
			vmaMapMemory(graphics.allocator, index_allocation[frame_index], (void**)&gpu_index_memory);
			memcpy(gpu_index_memory, index_arena.start, current.index_offset * sizeof(u32));
			vmaUnmapMemory(graphics.allocator, index_allocation[frame_index]);

			current = {};
			vertex_arena.reset();
			index_arena.reset();
		}
	};
	
// --------------------------------------------------------------------------------
// Renderer 2D

	struct Renderer_2d
	{
		VkPipeline     pipeline;
		Draw_Data_2d*  draw_data;

        struct Options
        {
            VkPrimitiveTopology topology;
        };

		static constexpr Options default_options = {
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		};

		void create(VkRenderPass render_pass, VkPipelineLayout pipeline_layout, Draw_Data_2d* draw_data, Options options = default_options);
		void draw(Render_Context const& ctx);
	};
}

// --------------------------------------------------------------------------------

namespace fission
{
	// ONLY meant for HARD-CODED defaults
	// (engine will handle saving/loading settings to/from file)
	struct Defaults {
		string      window_title     = ":)";
		u32         window_width     = 1280;
		u32         window_height    = 720;
		Window_Mode window_mode      = Windowed;
		int         display_index    = Display_Index_Automatic;
		string      config_location  = ".Fission"; // "app_name"
		u32         flags            = 0;

		enum Flag: u32 {
			fEnable_Graphics_Debugging = 1 << 0,
		};
	};
}

// --------------------------------------------------------------------------------
// User implemented functions

auto on_create() -> struct fission::Defaults;
void on_update(f64 dt, fission::array<fission::Event> events, fission::Render_Context const&);

// ******************************************************************
// - App_Info()
// - on_create()
// - on_create_scene()

namespace fission
{	
	struct App_Info
	{
	//	compressed_version version      = fs::make_compressed_version<0,1,0>;
		string  version_info = "dev";
		string  name         = "app name";

		App_Info();
	};

//	extern auto OS_CALL render_main(void*) -> os::Thread_Result;
	 
	struct Engine
	{
		enum Flag: u64 {
			Running                       = 1 << 0,
			Graphics_Recreate_Swap_Chain  = 1 << 1,
			Window_Resized                = 1 << 2,
			Window_Destroy_Enable         = 1 << 3,
			Change_Scene                  = 1 << 4,
			FPS_Limiter_Enable            = 1 << 5,
			Save_Current_Frame            = 1 << 6,
		};

		int              exit_code               {EXIT_SUCCESS};
		u64              flags                   {};
		Arena            temp_arena              {};
		Window           window                  {};
		Graphics         graphics                {};
		os::Thread       render_thread           {};
		Arena            frame_arena             {};
		VkRenderPass     overlay_render_pass     {};
		VkFramebuffer    frame_buffers [Graphics::MAX_SWAP_CHAIN_IMAGES] {};
		u32              frame_count             {};
		VkPipelineLayout pipeline_layout         {};
		Draw_Data_2d     draw_data               {};
		Renderer_2d      renderer                {};
		Renderer_2d      line_renderer          {};
		u64              last_ticks              {};

	public:
		auto version_string  () -> string;
		auto create          (Defaults const& defaults) -> Result;
		void run             ();
		void destroy         ();

		auto render_frame () -> bool;
		auto setup () -> Result;
	private:
	#ifdef os_main
		friend os_main();
	#endif
		static auto OS_CALL render_main(void*) -> os::Thread_Result;
		
		void shutdown ();

		void resize ();
		auto create_layers () -> Result;
		auto create_frame_buffers () -> Result;
		auto create_screenshot_buffer () -> Result;
	//	void save_frame (Render_Context& ctx);
		void write_frame ();
	};

	extern fission::Engine engine;
}

/**
 *	MIT License
 *
 *	Copyright (c) 2025 lazergenixdev
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
