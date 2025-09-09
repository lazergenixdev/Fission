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
//#include "glm/glm.hpp"
#include "vulkan/vulkan.h"
DISABLE_ALL_WARNINGS_BEGIN
#include "vk_mem_alloc.h"
DISABLE_ALL_WARNINGS_END

// --------------------------------------------------------------------------------
// OS Types

namespace fission
{
	using os::Mutex;
}
 
// TODO: refactor
/*
namespace fission
{
	template <typename T, size_t S>
	struct thread_safe_queue {
		// S is ignored for now, but I want to use this to
		//    limit the number events that can be queued.

		thread_safe_queue() {
			os_mutex_create(&access_mutex);
			array.reserve(1000);
		}

		~thread_safe_queue() {
			os_mutex_destroy(access_mutex);
		}

		inline void append(T const& event) {
			os_mutex_lock(access_mutex);
			array.emplace_back(event);
			os_mutex_unlock(access_mutex);
		}

		inline void pop_all(std::vector<T>& out_array) {
			out_array.clear();
			os_mutex_lock(access_mutex);
			for (auto&& item: array)
				out_array.emplace_back(item);
			os_mutex_unlock(access_mutex);
			array.clear();
		}

		os::Mutex access_mutex;
		std::vector<T> array;
	};
}*/

// --------------------------------------------------------------------------------
// Time

namespace fission
{
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
		p[arena.allocated++] = ('0' + (year) / 1000);
		p[arena.allocated++] = ('0' + (year / 100) % 10);
		p[arena.allocated++] = ('0' + (year / 10) % 10);
		p[arena.allocated++] = ('0' + (year) % 10);
		p[arena.allocated++] = '-';
		p[arena.allocated++] = ('0' + (month) / 10);
		p[arena.allocated++] = ('0' + (month) % 10);
		p[arena.allocated++] = '-';
		p[arena.allocated++] = ('0' + (day) / 10);
		p[arena.allocated++] = ('0' + (day) % 10);
		p[arena.allocated++] = ' ';

		u32 hour = (ts.time>>16)&0xFF, min = (ts.time>>8)&0xFF, sec = ts.time&0xFF;
		p[arena.allocated++] = ('0' + (hour) / 10);
		p[arena.allocated++] = ('0' + (hour) % 10);
		p[arena.allocated++] = ':';
		p[arena.allocated++] = ('0' + (min) / 10);
		p[arena.allocated++] = ('0' + (min) % 10);
		p[arena.allocated++] = ':';
		p[arena.allocated++] = ('0' + (sec) / 10);
		p[arena.allocated++] = ('0' + (sec) % 10);
		p[arena.allocated++] = '.';
		p[arena.allocated++] = ('0' + (ts.milliseconds / 100) % 10);
		p[arena.allocated++] = ('0' + (ts.milliseconds / 10) % 10);
		p[arena.allocated++] = ('0' + (ts.milliseconds) % 10);
	}
}

// --------------------------------------------------------------------------------
// Logging

namespace fission
{
	global Arena       logging_arena;
	global Mutex       logging_mutex;
	global int         minimum_log_level;
	global const char* logging_prefix;
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

	void write_log_from_logging_arena(int level);

	template <typename...T>
    void log(int level, T&&...args)
	{
		static const string level_strings [] {
			"  VERBOSE  ",
			"    DEBUG  ",
			"     INFO  ",
			"     WARN  ",
			"    ERROR  ",
		};
    	if (level < minimum_log_level) return;
		// Note: could probably do better than using locks
		os_mutex_lock(logging_mutex);
		logging_arena.reset();
    	format(logging_arena, Logging_Timestamp::now(), level_strings[level]);
		(format_single(logging_arena, std::forward<T>(args)), ...);
		format(logging_arena, "\n\0"); // null terminate in case we use C functions
		write_log_from_logging_arena(level);
		os_mutex_unlock(logging_mutex);
	}
	
    template <typename...T> inline void verbose (T&&...args) { log(Verbose, std::forward<T>(args)...); }
    template <typename...T> inline void debug   (T&&...args) { log(Debug  , std::forward<T>(args)...); }
    template <typename...T> inline void info    (T&&...args) { log(Info   , std::forward<T>(args)...); }
    template <typename...T> inline void warn    (T&&...args) { log(Warn   , std::forward<T>(args)...); }
    template <typename...T> inline void error   (T&&...args) { log(Error  , std::forward<T>(args)...); }
}

// --------------------------------------------------------------------------------
// Events

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

		string name() const noexcept { return {name_buffer, (size_t)name_count}; }

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
	enum EventType {
		Event_Key_Down,
		Event_Key_Up,
		Event_Focus_Lost,
		Event_Character_Input,

		// Does this really need to be here?
		// mouse position is always stored in `engine.window.mouse_position`
		Event_Mouse_Move_Absolute,
		Event_Mouse_Move_Relative,
	};

	// TODO: try linked list for events (no pointers, only offsets)
	struct Event {
		s64 timestamp;
		u8 type;

		union EventSpecialization {
			struct EventKeyDown {
				u32 key_id;
			} key_down;

			struct EventKeyUp {
				u32 key_id;
			} key_up;

			struct EventCharacterInput {
				c32 codepoint;
			} character_input;

			struct EventMouseMoveAbsolute {
				v2s32 position;
			} mouse_move_absolute;

			struct EventMouseMoveRelative {
				v2s32 delta;
			} mouse_move_relative;
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

	using Event_Queue = struct {}; //thread_safe_queue<Event, 64>;

	struct Window : public os::Window
	{
		Event_Queue  event_queue      {};
		v2s32        mouse_position   {};
		Window_Mode  mode             {Windowed_Fullscreen};
		int          display_index    {Display_Index_Automatic};
		bool         use_mouse_deltas {false};

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
			u32 width;
			u32 height;
		};

		auto create(Create_Info const& info) -> Result;

		// When a window is created, it will be hidden, calling
		//  this will show the window (at least in Windows).
		void show();

		// Closes the window => causes engine to stop running => Application closes
		void close();

		~Window();

		friend struct Engine;
	};
}

// --------------------------------------------------------------------------------
// Window

namespace fission
{
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

		VkInstance                     instance                  {};
		VkPhysicalDevice               physical_device           {};
		VkDevice                       device                    {};
		VkQueue                        graphics_queue            {};
		VkQueue                        present_queue             {};
		VkSurfaceKHR                   surface                   {};
		VkSwapchainKHR                 swap_chain                {};
		VkExtent2D                     extent                    {}; // Swap Chain
		VkFormat                       format                    {}; // Swap Chain
		VkImageUsageFlags              image_usage               {}; // Swap Chain
		u32                            image_count               {0}; // Swap Chain
		VkPresentModeKHR               present_mode              {VK_PRESENT_MODE_FIFO_KHR}; // Swap Chain
		VkSurfaceTransformFlagBitsKHR  transform                 {}; // Swap Chain
		VkImage*                       images                    {}; // Swap Chain
		VkImageView*                   image_views               {}; // Swap Chain
		VkCommandPool                  command_pool              {};
		VkQueue                        transfer_queue            {};
		VkCommandPool                  transfer_command_pool     {};
		VkCommandBuffer                command_buffers       [2] {};
		VkFence                        fences                [2] {};
		VkSemaphore                    image_write_semaphore [2] {};
		VkSemaphore                    image_read_semaphore  [2] {};
		VmaAllocator                   allocator                 {};
		VkDebugUtilsMessengerEXT       debug_messenger           {};
	//	Graphics_Extra   extra {};

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

//	extern auto OS_CALL render_main(void*) noexcept -> os::Thread_Result;
	 
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

		int             exit_code       {EXIT_SUCCESS};
		os::Thread      render_thread   {};
		Arena           frame_arena     {};
		Window          window          {};
		Graphics        graphics        {};

	public:
		auto version_string  () -> string;
		auto create          (Defaults const& defaults) -> Result;
		void run             ();
		void destroy         ();

	private:
	#ifdef os_main
		friend os_main();
	#endif
		static auto OS_CALL render_main(void*) noexcept -> os::Thread_Result;
		
		auto setup () -> Result;
		auto render_frame () -> Result;
		void shutdown ();

		void resize ();
		auto create_layers () -> Result;
		auto create_frame_buffers (u32 old_count) -> Result;
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