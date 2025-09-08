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
 
// TODO: refactor
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
}

// --------------------------------------------------------------------------------
// Logging

namespace fission::log
{
    enum {
        Verbose = 0,
        Debug   = 1,
        Info    = 2,
        Warn    = 3,
        Error   = 4,
        LEVEL_COUNT
    };

    void log(int level, string const& message);
	
    inline void verbose (string const& message) { log(Verbose, message); }
    inline void debug   (string const& message) { log(Debug  , message); }
    inline void info    (string const& message) { log(Info   , message); }
    inline void warn    (string const& message) { log(Warn   , message); }
    inline void error   (string const& message) { log(Error  , message); }
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

		auto set_display_mode(const Display_Mode *) -> bool; // TODO: Move to Window?
		
		auto revert_display_mode() -> bool;
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

		union {
			struct {
				u32 key_id;
			} key_down;

			struct {
				u32 key_id;
			} key_up;

			struct {
				c32 codepoint;
			} character_input;

			struct {
				v2s32 position;
			} mouse_move_absolute;

			struct {
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

	using Event_Queue = thread_safe_queue<Event, 64>;

	struct Window : public os::Window
	{
		Event_Queue  event_queue      {};
		v2s32        mouse_position   {};
		Window_Mode  mode             {Windowed_Fullscreen};
		int          display_index    {Display_Index_Automatic};
		bool         use_mouse_deltas {false};

		// Private API
		auto create(struct Window_Create_Info const& info) -> bool;

		// Private API
		// When a window is created, it will be hidden, calling
		//  this will show the window (at least in Windows).
		void show();

		// Private API
		// Closes the window => causes engine to stop running
		// => Application closes
		// Don't call this function, use Engine::flags and
		//  set the Running Bit to zero.
		void close();

		// @see enum Window_Mode
		auto supported_modes() -> u32;

		//void set_title(string const& title);
		//bool is_minimized();
		//void set_mode(Window_Mode mode);

		inline void toggle_using_mouse_deltas() {
			set_using_mouse_delta(!use_mouse_deltas);
		}

		void set_using_mouse_delta(bool use);

		~Window();
	};
}

// --------------------------------------------------------------------------------

namespace fission
{
	// ONLY meant for HARD-CODED defaults
	// (engine will handle saving/loading settings to/from file)
	struct Defaults {
		string      window_title     = ":)";
		int         window_width     = 1280;
		int         window_height    = 720;
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

		auto get_version_string() -> string;

		os::Thread render_thread;
		int exit_code = EXIT_SUCCESS;

	public:
		auto create (Defaults const& defaults) -> bool;
		void run ();
		void destroy ();

	private:
	#ifdef os_main
		friend os_main();
	#endif
		static auto OS_CALL render_main(void*) noexcept -> os::Thread_Result;
		
		auto setup () -> bool;
		auto render_frame () -> bool;
		void shutdown ();

		void resize ();
		auto create_layers () -> bool;
		auto create_frame_buffers (u32 old_count) -> bool;
		auto create_screenshot_buffer () -> bool;
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