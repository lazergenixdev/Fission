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
#include <Fission/base/rect.hpp>
#include <Fission/base/color.hpp>
#include <Fission/base/string.hpp>
#include <Fission/core/graphics.hpp>
#include <Fission/core/font.hpp>
#include <Fission/graphics/util.hpp>

__FISSION_BEGIN__

namespace impl {
    struct Frame_Data {
        VkBuffer      vertex_buffer     {};
        VkBuffer       index_buffer     {};
        VmaAllocation vertex_allocation {};
        VmaAllocation  index_allocation {};

        Frame_Data() = default;

        Frame_Data(VmaAllocator allocator, u32 max_vertex_size, u32 max_index_size);

        void send(VmaAllocator allocator, void* vertex_data, void* index_data, u32 vertex_size, u32 index_size);
    };
}

enum Border_Style {
    BORDER_OUTSIDE = 0,
    BORDER_INSIDE  = 1,
    BORDER_CENTER  = 2,
};

struct Renderer_2D {

	void add_triangle(v2f32 p0, v2f32 p1, v2f32 p2, color c);

	void add_triangle(v2f32 p0, v2f32 p1, v2f32 p2, color c1, color c2, color c3);

	void add_line(v2f32 start, v2f32 end, float stroke, color c_start, color c_end);

	void add_rect(rf32 rect, color c);

	void add_rect(rf32 rect, color c_top_left, color c_top_right, color c_bot_left, color c_bot_right);

	void add_rect_outline(rf32 rect, float thick, Border_Style style, color c);

	void add_circle(v2f32 position, float radius, color c);

	inline void add_circle_outline(v2f32 position, float radius, float inner_radius, color c) {
		add_circle_outline(position, radius, inner_radius, c, c);
	}
	void add_circle_outline(v2f32 position, float radius, float inner_radius, color c_out, color c_in);

    // Sumbit one draw call
	inline void draw(Render_Context const& ctx) { draw_pipeline(ctx, pipeline); }
    void draw_pipeline(Render_Context const& ctx, VkPipeline pipeline);

    // Call this when it is time to submit command buffer
	void end_render(Render_Context const& ctx);

	void create(Graphics* gfx, VkRenderPass render_pass, Transform_2D_Layout layout);

	void destroy();

    struct Vertex {
        v2f32 position;
        rgba  color;
    };

	VkPipeline pipeline;
	VkPipelineLayout pipeline_layout;

	u32 max_vertex_count;
	u32 max_index_count;

	impl::Frame_Data frame_data [2];

	Vertex* vertex_data;
	u16*    index_data;

	Draw_Data d;

    // Shaders free to use for other pipelines,
    // will always be alive until engine gets destroyed
	VkShaderModule vert;
	VkShaderModule frag;
};

struct Textured_Renderer_2D {

	void add_rect(rf32 const& rect, rf32 const& uv, color c);

	void add_glyph(Glyph const* g, v2f32 origin, float scale, color c);

	// exists so that there is no need to pass extra parameter to add_string
	inline void set_font(struct Font* font) { current_font = font; }

	v2f32 add_string(string str, v2f32 top_left, color col);

	// right-aligned string
	v2f32 add_string_rtl(string str, v2f32 top_right, color col);

    // Sumbit one draw call
	inline void draw(Render_Context const& ctx) { draw_pipeline(ctx, pipeline); }
    void draw_pipeline(Render_Context const& ctx, VkPipeline pipeline);

    // Call this when it is time to submit command buffer
	void end_render(Render_Context const& ctx);

	void create(Graphics* gfx, VkRenderPass render_pass, Transform_2D_Layout transform_layout, Texture_Layout texture_layout);

	void destroy();

    struct Vertex {
        v2f32 position;
	    v2f32 texcoord;
        rgba  color;
    };

	VkPipeline       pipeline;
	VkPipelineLayout pipeline_layout;

	u32 max_vertex_count;
	u32 max_index_count;

	impl::Frame_Data frame_data [2];

	Vertex* vertex_data;
	u16*    index_data;

	Font* current_font;

	Draw_Data d;

    // Shaders free to use for other pipelines,
    // will always be alive until engine gets destroyed
	VkShaderModule vert;
	VkShaderModule frag;
};

__FISSION_END__

/**
 *	MIT License
 *
 *	Copyright (c) 2024 lazergenixdev
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