#include <Fission/Core/Layer.hh>
#include <Fission/Core/Engine.hh>
#include <Fission/Core/Input/Keys.hh>
#include <format>
#include <intrin.h>

#define FS_DEBUG_LAYER_SHOW_HARDWARE 1 //FISSION_DEBUG

extern fs::Engine engine;
extern fs::string platform_version;

using namespace fs;

void get_cpu_string(string& buffer) {
	int CPUInfo[4] = { -1 };
	unsigned   nExIds, i = 0;
	// Get the information associated with each extended ID.
	__cpuid(CPUInfo, 0x80000000);
	nExIds = CPUInfo[0];
	for (i = 0x80000000; i <= nExIds; ++i)
	{
		__cpuid(CPUInfo, i);
		// Interpret CPU brand string
		if (i == 0x80000002)
			memcpy(buffer.data,      CPUInfo, sizeof(CPUInfo));
		else if (i == 0x80000003)
			memcpy(buffer.data + 16, CPUInfo, sizeof(CPUInfo));
		else if (i == 0x80000004)
			memcpy(buffer.data + 32, CPUInfo, sizeof(CPUInfo));
	}

	size_t size = strlen((char*)buffer.data) - 1;
	while (buffer.data[size] == ' ') size--;
	buffer.count = size + 1;
}

template <>
struct ::std::formatter<fs::compressed_version> {
	constexpr auto parse(std::format_parse_context& ctx) {
		return ctx.begin();
	}

	auto format(const fs::compressed_version& cv, std::format_context& ctx) {
		auto v = cv.uncompress();
		return std::format_to(ctx.out(), "{}.{}.{}", v.Major, v.Minor, v.Patch);
	}
};

void copy_to(string s, std::vector<char>& out) {
	FS_FOR(s.count) out.emplace_back(s.data[i]);
}

void Debug_Layer::create() {
	frame_count = 240; // wha?
	frame_times = (float*)_aligned_malloc(frame_count * sizeof(float), 32);
	FS_FOR(frame_count) frame_times[i] = 1.0f;

	character_buffer.reserve(512);
	left_strings.reserve(16);
	right_strings.reserve(16);

	auto p = (c8*)character_buffer.data();
#define end(V) (c8*)(V.data() + V.size())
	
	std::format_to(std::back_inserter(character_buffer), "{} ({}/{})",
		engine.app_name.str(), engine.app_version, engine.app_version_info.str());
	{
		auto start = p;
		p = end(character_buffer);
		app_info_string = string{ .count = u64(p - start), .data = start };
	}
	copy_to(engine.get_version_string(), character_buffer);
	{
		auto start = p;
		p = end(character_buffer);
		right_strings.emplace_back(string{.count = u64(p-start), .data = start});
	}
	copy_to(platform_version, character_buffer);
	{
		auto start = p;
		p = end(character_buffer);
		right_strings.emplace_back(string{.count = u64(p-start), .data = start});
	}
	auto vk_version = engine.graphics.get_api_version();
	std::format_to(std::back_inserter(character_buffer), "Vulkan ({}.{}.{})",
		vk_version.Major, vk_version.Minor, vk_version.Patch);
	{
		auto start = p;
		p = end(character_buffer);
		right_strings.emplace_back(string{.count = u64(p-start), .data = start});
	}
#if FS_DEBUG_LAYER_SHOW_HARDWARE
	right_strings.emplace_back();
	char CPUBrandString[0x40];
	string cpu_string = FS_str_buffer(CPUBrandString);
	get_cpu_string(cpu_string);
	std::format_to(std::back_inserter(character_buffer), "CPU: {}", cpu_string.str());
	{
		auto start = p;
		p = end(character_buffer);
		right_strings.emplace_back(string{ .count = u64(p - start), .data = start });
	}
	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(engine.graphics.physical_device, &props);
	std::format_to(std::back_inserter(character_buffer), "GPU: {}",
		std::string_view(props.deviceName));
	{
		auto start = p;
		p = end(character_buffer);
		right_strings.emplace_back(string{.count = u64(p-start), .data = start});
	}
#endif
#undef end
	character_count = (int)character_buffer.size();
}

void Debug_Layer::destroy() {
	_aligned_free(frame_times);
}

void Debug_Layer::add(string s) {
	left_strings.emplace_back(string{ .count = s.count, .data = (c8*)(character_buffer.data() + character_buffer.size()) });
	FS_FOR(s.count) {
		character_buffer.emplace_back(s.data[i]);
	}
}

void Debug_Layer::handle_events(std::vector<Event>& events) {
	for (auto it = events.begin(); it != events.end(); ) {
		if (it->type == Event_Key_Down) {
			if (it->key_down.key_id == keys::F3) {
				flags ^= layer::show;
				it = events.erase(it);
				continue;
			}
		}
		else if (it->type == Event_Key_Up) {
			if (it->key_down.key_id == keys::F3) {
				it = events.erase(it);
				continue;
			}
		}
		++it;
	}
}

static constexpr float padding = 4.0f;

void reset(Debug_Layer& db) {
	db.character_buffer.resize(db.character_count);
	db.left_strings.clear();
}

void Debug_Layer::on_update(double dt, Render_Context* ctx) {
	frame_times[frame_time_index] = (float)dt;
	if (++frame_time_index >= frame_count) frame_time_index = 0;

	if (!(flags & layer::show)) return reset(*this);

	engine.textured_renderer_2d.set_font(&engine.fonts.debug);

	float height = engine.fonts.debug.height;
	float offset = 0.0f;
	auto add_text = [&](string s) {
		if (s.count) {
			auto bounds = engine.textured_renderer_2d.add_string(s, { 0.0f, offset }, colors::White);
			engine.renderer_2d.add_rect({0.0f, bounds.x+padding, offset, offset+bounds.y}, color(colors::Black, 0.9f));
		}
		offset += height;
	};
	float right = (float)engine.graphics.sc_extent.width;
	auto add_text_right = [&](string s) {
		if (s.count) {
			auto bounds = engine.textured_renderer_2d.add_string_rtl(s, { right, offset }, colors::White);
			engine.renderer_2d.add_rect({right-bounds.x-padding, right, offset, offset + bounds.y}, color(colors::Black, 0.9f));
		}
		offset += height;
	};

	float mean_frame_time = 0.0f;
	FS_FOR(frame_count) mean_frame_time += frame_times[i];
	mean_frame_time /= frame_count;

	char buffer[64];
	int count = sprintf(buffer, "%.1f FPS (%.2f ms)", 1.0f / mean_frame_time, mean_frame_time * 1000.0f);

	add_text(app_info_string);
	add_text(FS_str_make(buffer, count));
	offset += height;
	for (auto&& s : left_strings) add_text(s);

	offset = 0.0f;
	for (auto&& s : right_strings) add_text_right(s);

	engine.renderer_2d         .draw(*ctx);
	engine.textured_renderer_2d.draw(*ctx);

	reset(*this);
}
