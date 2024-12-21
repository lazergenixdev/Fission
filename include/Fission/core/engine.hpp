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
#include <Fission/core/window.hpp>
#include <Fission/core/graphics.hpp>
#include <Fission/core/font.hpp>
#include <Fission/core/layer.hpp>
#include <Fission/core/scene.hpp>
#include <Fission/base/version.hpp>
#include <Fission/graphics/renderer_2d.hpp>
#include <chrono>

typedef struct FT_LibraryRec_* FT_Library;
namespace fs {
    struct Defaults;
    struct Engine;
}

extern fs::Engine engine;

// ******************************************************************
// user implemented functions
// - App_Info()
// - on_create()
// - on_create_scene()

struct App_Info {
	fs::compressed_version version      = fs::make_compressed_version<0,1,0>;
	fs::string             version_info = "dev";
	fs::string             name         = "app name";

	App_Info();
};

// called before engine/graphics/window creation
extern auto on_create() -> fs::Defaults;

// called after engine/graphics/window creation to load a new scene
extern auto on_create_scene(fs::Scene_Key const& key) -> fs::Scene*;

// ******************************************************************

__FISSION_BEGIN__

extern auto OS_CALL render_main(void*) noexcept -> os::Thread_Result;

FISSION_API auto timestamp() -> s64;
FISSION_API auto seconds_elasped(s64 start, s64 end) -> double;
FISSION_API auto seconds_elasped_and_reset(s64& last) -> double;

// TODO: WHY IS THIS HERE?
// out_size = in_size
FISSION_API void convert_utf8_to_utf16(string_utf16* output_buffer, string source);
// out_size = in_size * 3
FISSION_API void convert_utf16_to_utf8(string* output_buffer, string_utf16 source);

// Temparary Alloc
FISSION_API auto talloc(u64 size) -> void*;

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

	inline void bind_font(VkCommandBuffer cmd, Font_Static const* p_font) {
		VkDescriptorSet sets[] = { transform_2d.set, p_font->texture };
		FS_VK_BIND_DESCRIPTOR_SETS(cmd, textured_renderer_2d.pipeline_layout, 2, sets);
	}

	////////////////////////////////////////////////////////////////////////////
	// Members
    Logger               logger;
	Window               window;
	Graphics             graphics;

	// Engine overlay's render pass, expects current
	//	swap chain image to be in layout: COLOR_ATTACHMENT_OPTIMAL
	VkRenderPass         overlay_render_pass;
	VkFramebuffer*       frame_buffers;

	Renderer_2D          renderer_2d;
	Textured_Renderer_2D textured_renderer_2d;

	Debug_Layer          debug_layer;
	Console_Layer        console_layer;

	Scene*               current_scene;
	double               delta_time = 0.0;
	s64                  last_timestamp;
	u64                  frame_count {0};
		
	u64                  flags;
	u64                  modifier_keys;
	float                fps_limit = 60.0f;

	App_Info app_info;
	
	compressed_version const version;

	struct {
		FT_Library library;

		Font_Static debug;
		Font_Static console;

	//	std::unordered_map<std::string_view, Font*> table;

		VkSampler sampler;
	} font;

	struct {
		VkDescriptorSet     set;
		Transform_2D_Layout layout;
		VkBuffer            buffer;
		VmaAllocation       allocation;
	} transform_2d;

	Texture_Layout texture_layout;

	// Pool for Uniform Buffers and Combined image-samplers
	//	(what the engine uses internally)
	VkDescriptorPool descriptor_pool;

	VkBuffer      screenshot_buffer     {};
	VmaAllocation screenshot_allocation {};

	void* _ts_base = nullptr;
	u32   _ts_allocated = 0;
	u32   _ts_size = 0;

	Scene_Key next_scene_key;

	array<Display> displays;

    os::Thread render_thread;
    int exit_code = EXIT_SUCCESS;

public:
    auto create (Defaults const& defaults) -> bool;
    void run ();
    void destroy ();

private:
#ifdef _os_main
    friend _os_main();
#endif
    friend auto OS_CALL render_main(void*) noexcept -> os::Thread_Result;
	
	auto setup () -> bool;
    auto render_frame () -> bool;
	void shutdown ();

	void resize ();
	auto create_layers () -> bool;
	auto create_frame_buffers (u32 old_count) -> bool;
	auto create_screenshot_buffer () -> bool;
	void save_frame (Render_Context& ctx);
	void write_frame ();
};

__FISSION_END__

/**
 *	MIT License
 *
 *	Copyright (c) 2021-2025 lazergenixdev
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
