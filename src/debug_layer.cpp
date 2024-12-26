#include <Fission/core/layer.hpp>
#include <Fission/core/engine.hpp>
#include <Fission/core/input/keys.hpp>
#include "internal.hpp"
#include <format>
#include <random>
#if defined(FISSION_PLATFORM_WINDOWS)
#include <intrin.h>
#endif

#define FS_DEBUG_LAYER_SHOW_HARDWARE    1 //FISSION_DEBUG
#define FS_DEBUG_FRAME_GRAPH_HEART_BEAT 0

extern fs::Engine engine;
extern fs::string platform_version;
extern fs::string cpu_name;

using namespace fs;

void copy_to(string s, std::vector<c8>& out) {
	for_n (s.count) out.emplace_back(s.data[i]);
}

void Debug_Layer::create() {
	frame_count = 128; // wha?
	frame_times = (float*)FISSION_DEFAULT_ALLOC(frame_count * sizeof(float));
	for_n(frame_count) frame_times[i] = 0.001f;

	character_buffer.reserve(512);
	left_strings.reserve(16);
	right_strings.reserve(16);

#   define next_view string_view{.offset = (u32)offset, .count = u32(character_buffer.size() - offset)}

	size_t offset = 0;
	fmt::format_to(std::back_inserter(character_buffer), "{} ({}.{}.{}/{})",
		engine.app_info.name.str(),
        engine.app_info.version.uncompress().Major,
        engine.app_info.version.uncompress().Minor,
        engine.app_info.version.uncompress().Patch,
        engine.app_info.version_info.str()
    );
	app_info_string = next_view;

	offset = character_buffer.size();
	fmt::format_to(std::back_inserter(character_buffer), "{} ({}.{}.{})",
		(std::random_device{}() % 128 ? "Fission Engine" : "Unreal Engine"),
        engine.version.Major,
        engine.version.Minor,
        engine.version.Patch
    );
	right_strings.emplace_back(next_view);
	
	offset = character_buffer.size();
	copy_to(platform_version, character_buffer);
	right_strings.emplace_back(next_view);
	
	auto vk_version = engine.graphics.api_version();
	offset = character_buffer.size();
	fmt::format_to(std::back_inserter(character_buffer), "Vulkan ({}.{}.{})",
		vk_version.Major, vk_version.Minor, vk_version.Patch);
	right_strings.emplace_back(next_view);

#if FS_DEBUG_LAYER_SHOW_HARDWARE
	right_strings.emplace_back();

	offset = character_buffer.size();
	fmt::format_to(std::back_inserter(character_buffer), "CPU: {}", cpu_name.str());
	right_strings.emplace_back(next_view);

	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(engine.graphics.physical_device, &props);
	offset = character_buffer.size();
	fmt::format_to(std::back_inserter(character_buffer), "GPU: {}", std::string_view(props.deviceName));
	right_strings.emplace_back(next_view);
#endif
	
	character_count_initial = (int)character_buffer.size();
}

void Debug_Layer::destroy() {
	FISSION_DEFAULT_FREE(frame_times);
}

void Debug_Layer::add(string s) {
	left_strings.emplace_back( string_view {
        .offset = (u32)character_buffer.size(),
        .count = (u32)s.count
    });
	for_n (s.count) character_buffer.emplace_back(s.data[i]); // copy
}

void Debug_Layer::handle_events(std::vector<Event>& events) {
	for (auto it = events.begin(); it != events.end(); ) {
		if (it->type == Event_Key_Down) {
			if (it->key_down.key_id == keys::F3) {
				if ((flags& layer::enable) && !(engine.console_layer.flags& layer::show))
					flags ^= layer::show;
				it = events.erase(it); // TODO: do this more?
				continue;
			}
			else if (it->key_down.key_id == fs::keys::F4) {
				if (flags & layer::show) engine.debug_layer.flags ^= fs::layer::debug_show_verbose;
			}
			else if (it->key_down.key_id == fs::keys::F10) {
				engine.flags |= Engine::Save_Current_Frame;
			}
		}
		else if (it->type == Event_Key_Up) {
			if (it->key_up.key_id == keys::F3 || it->key_down.key_id == fs::keys::F4) {
				it = events.erase(it);
				continue;
			}
		}
		++it;
	}
}

static constexpr float padding = 4.0f;

void reset(Debug_Layer& db) {
	db.character_buffer.resize(db.character_count_initial);
	db.left_strings.clear();
}

float Debug_Layer::draw_frame_time_graph(v2f32 top_left) {
	float const height = 50.0f;
	float const bottom = top_left.y + height;
	float const width  = 3.0f * (float)(frame_count-1);
#if FS_DEBUG_FRAME_GRAPH_HEART_BEAT
	engine.renderer_2d.add_rect(rf32::from_topleft(top_left, width, height), colors::Black);
	engine.renderer_2d.add_rect(rf32::from_topleft(top_left.x, bottom - (1.0f / 60.0f) * 2000.0f, width, 1.0f), colors::Lime);
	FS_FOR(frame_count - 1) {
		auto const& y0 = frame_times[i];
		auto const& y1 = frame_times[i + 1];

		auto const x0 = top_left.x + 3.0f * (float)i;
		auto const x1 = top_left.x + 3.0f * (float)(i + 1);

		engine.renderer_2d.add_line({x0, bottom - y0 * 2000.0f}, {x1, bottom - y1 * 2000.0f}, 1.0f, colors::White, colors::White);
	}
	engine.renderer_2d.add_rect(rf32::from_topleft(3.0f * (float)frame_time_index, top_left.y, 1.0f, height), colors::Red);
	return height;
#else
	engine.renderer_2d.add_rect(rf32::from_topleft(top_left, width, height), colors::black);
	engine.renderer_2d.add_rect(rf32::from_topleft(top_left.x, bottom - (1.0f / 60.0f) * 2000.0f, width, 1.0f), colors::lime);
	float x = top_left.x;
	for (int i = frame_time_index; i > 0; --i) {
		auto const y0 = frame_times[i];
		auto const y1 = frame_times[i - 1];
		auto const xn = x + 3.0f;

		engine.renderer_2d.add_line({x, bottom - y0 * 2000.0f}, {xn, bottom - y1 * 2000.0f}, 1.0f, colors::white, colors::white);
		x = xn;
	}
	float earlier = frame_times[0];
	for (int i = frame_count-1; i > frame_time_index; --i) {
		auto const y0 = earlier;
		auto const y1 = frame_times[i];
		auto const xn = x + 3.0f;

		engine.renderer_2d.add_line({x, bottom - y0 * 2000.0f}, {xn, bottom - y1 * 2000.0f}, 1.0f, colors::white, colors::white);
		x = xn;
		earlier = y1;
	}
	return height;
#endif
}

bool Debug_Layer::visible() const {
	return (flags & layer::show) && !(engine.console_layer.flags & layer::show);
}

void Debug_Layer::on_update(double dt, Render_Context* ctx) {
	if (++frame_time_index >= frame_count) frame_time_index = 0;
	frame_times[frame_time_index] = (float)dt;

	if (!visible()) return reset(*this);

	engine.textured_renderer_2d.set_font(&engine.font.debug);

	static constexpr auto bg_color = color(colors::black, 0.95f);

	float height = engine.font.debug.height;
	float offset = 0.0f;
	auto add_text = [&](string s) {
		if (s.count) {
			auto bounds = engine.textured_renderer_2d.add_string(s, { 0.0f, offset }, colors::white);
			engine.renderer_2d.add_rect({0.0f, bounds.x+padding, offset, offset+bounds.y}, bg_color);
		}
		offset += height;
	};
	float right = (float)engine.graphics.sc_extent.width;
	auto add_text_right = [&](string s) {
		if (s.count) {
			auto bounds = engine.textured_renderer_2d.add_string_rtl(s, { right, offset }, colors::white);
			engine.renderer_2d.add_rect({right-bounds.x-padding, right, offset, offset + bounds.y}, bg_color);
		}
		offset += height;
	};

	float mean_frame_time = 0.0f;
	for_n (frame_count) mean_frame_time += frame_times[i];
	mean_frame_time /= (float)frame_count;

	auto base = character_buffer.data();
	
	add_text(app_info_string.absolute(base));
	add_text(fmt::format("{:.1f} FPS ({:.2f} ms)", 1.0f / mean_frame_time, mean_frame_time * 1000.0f));

	if (flags& layer::debug_show_verbose) {
		add_text(fmt::format("CPU time: {:.4f} ms", cpu_time * 1000.f));
		offset += draw_frame_time_graph({0.0f, offset});
	}
	else offset += height;

	for (auto&& s : left_strings) add_text(s.absolute(base));

	offset = 0.0f;
	for (auto&& s : right_strings) add_text_right(s.absolute(base));

	engine.renderer_2d         .draw(*ctx);
	engine.textured_renderer_2d.draw(*ctx);

	reset(*this);
}