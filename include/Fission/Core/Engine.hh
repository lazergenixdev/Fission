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
#include <Fission/Core/Window.hh>
#include <Fission/Core/Graphics.hh>
#include <Fission/Core/Font.hh>
#include <Fission/Core/Layer.hh>
#include <Fission/Core/Scene.hh>
#include <Fission/Base/Version.hpp>
#include <Fission/Core/Renderer_2D.hh>
#include <mutex>
#include <condition_variable>

typedef struct FT_LibraryRec_* FT_Library;
namespace fs { struct Defaults; }

// ******************************************************************
// user defined functions

// called before engine/graphics/window creation
extern fs::Defaults on_create();

// called after engine/graphics/window creation to load a new scene
extern fs::Scene* on_create_scene(fs::Scene_Key const& key);

__FISSION_BEGIN__

// ONLY meant for HARD-CODED defaults
// (engine will handle saving/loading settings to/from file)
struct Defaults {
	string      window_title   = FS_str(":)");
	int         window_width   = 1280;
	int         window_height  = 720;
	Window_Mode window_mode    = Windowed;
};

struct FISSION_API Engine {

	enum Flag: u64 {
		fScene_Change                  = 1 << 0,
		fRunning                       = 1 << 1,
		fWindow_Minimized              = 1 << 2,
		fWindow_Resized                = 1 << 3,
		fWindow_Destroy_Enable         = 1 << 4,
		fGraphics_Recreate_Swap_Chain  = 1 << 5,
		fWindow_Mode_Change            = 1 << 6,
	};

	////////////////////////////////////////////////////////////////////////////
	Window               window;
	Graphics             graphics;
	Render_Pass          overlay_render_pass;
	Renderer_2D          renderer_2d;
	Textured_Renderer_2D textured_renderer_2d;

	Debug_Layer   debug_layer;
	Console_Layer console_layer;

	Scene* current_scene;
		
	// use flags?
	bool running;
	bool minimized; // note: rename to window_minimized
	bool window_resized;
	u64 flags = 0;
	u64 modifier_keys = 0;

	Texture_Layout texture_layout;

	struct {
		VkDescriptorSet     set;
		Transform_2D_Layout layout;
		VkBuffer            buffer;
		VmaAllocation       allocation;
	} transform_2d;

	// Pool for Uniform Buffers and Combined image-samplers
	VkDescriptorPool descriptor_pool;

	struct {
		FT_Library library;

		Font_Static debug;
		Font_Static console;

	//	std::unordered_map<std::string_view, Font*> table;

		VkSampler sampler;
	} fonts;

	std::vector<Display> displays;

	// only used to put thread to sleep when minimized
	::std::mutex              _mutex;
	::std::condition_variable _event;

	union {
		Scene_Key next_scene_key;
		struct {
			VkPresentModeKHR present_mode;
		} swap_chain_info;
	};
	std::vector<u8> _scene_key_memory;

	void* _temp_memory_base = nullptr;
	u64   _temp_memory_size = 0;
		
	// Version stuff
	compressed_version const version;
	// Version stuff for app
	compressed_version app_version;
	string             app_version_info;
	string             app_name;

	////////////////////////////////////////////////////////////////////////////

	void* talloc(u64 size);

	void reset_scene_key_memory();
	string alloc_scene_key_string(string s);

	void run();
	int create(platform::Instance const& instance, Defaults const& defaults);
	int destroy();

	string get_version_string();

private:
	void resize();
	int create_layers();
};

__FISSION_END__

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