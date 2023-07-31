#include <Fission/Core/Engine.hh>
#include <Fission/Core/Input/Keys.hh>
#include <Fission/Core/Console.hh>
#include <unordered_map>

extern fs::Engine engine;

__FISSION_BEGIN__

namespace console {

	std::unordered_map<std::string, console_callback_proc> callback_table;

	bool register_command(string name, console_callback_proc proc) {
		auto std = std::string(name.str());
		callback_table[std] = proc;
		return true;
	}
	void unregister_command(string name) {
		(void)name;
	}

	void clear() {
		engine.console_layer.buffer_view.count = 0;
	}

	void println(string text) {
		if (text.count >= Console_Layer::default_buffer_size - 1)
			return; // silent fail... too long didn't read

		engine.console_layer._reserve_space_for(text.count + 1);
		auto& console_view = engine.console_layer.buffer_view;

		memcpy(console_view.data + console_view.count, text.data, text.count);
		console_view.count += text.count;
		console_view.data[console_view.count++] = '\n';

	//	char b[32];
	//	sprintf(b, "buffer count = %i\n", (int)console_view.count);
	//	OutputDebugStringA(b);
	}

	void println(string text, rgb8 color) {
		if (text.count >= Console_Layer::default_buffer_size - 1)
			return; // silent fail... too long didn't read

		engine.console_layer._reserve_space_for(text.count + 1 + 4 + 1);
		auto& console_view = engine.console_layer.buffer_view;

		c8 color_code[4] = { 0x1b, color.r, color.g, color.b };
		memcpy(console_view.data + console_view.count, color_code, 4);
		console_view.count += 4;

		memcpy(console_view.data + console_view.count, text.data, text.count);
		console_view.count += text.count;

		console_view.data[console_view.count++] = 0x1b;
		console_view.data[console_view.count++] = '\n';

	//	char b[32];
	//	sprintf(b, "buffer count = %i\n", (int)console_view.count);
	//	OutputDebugStringA(b);
	}

	void print(string text) {
		if (text.count >= Console_Layer::default_buffer_size - 1)
			return; // silent fail... too long didn't read

		engine.console_layer._reserve_space_for(text.count);
		auto& console_view = engine.console_layer.buffer_view;

		memcpy(console_view.data + console_view.count, text.data, text.count);
		console_view.count += text.count;
	}
	void print(string text, rgb8 color) {

	}

}

void Console_Layer::_reserve_space_for(u64 added_count) {
	if (buffer_view.count + added_count < buffer_capacity) return;

	auto new_start = buffer_view.data + minimum_buffer_delete_count;
	auto end       = buffer_view.data + maximum_buffer_delete_count;

	if (added_count > maximum_buffer_delete_count) {
		new_start = buffer_view.data + added_count;
	}
	else {
		while (new_start < end) {
			if (*new_start == '\n') {
				++new_start;
				break;
			}
			++new_start;
		}
		new_start = std::min(new_start, end);
	}
	
	buffer_view.count = (buffer_view.data + buffer_view.count) - new_start;
	memmove(buffer_view.data, new_start, buffer_view.count);
}

void Console_Layer::create() {
	input.count = 2;
	input.data  = input_buffer;
	memset(input_buffer, 0, sizeof(input_buffer));
	input_buffer[0] = '>';
	input_buffer[1] = ' ';

	buffer_capacity = default_buffer_size;
	buffer_view.data = (c8*)_aligned_malloc(buffer_capacity, 32);
	buffer_view.count = 0;

	console::println(FS_str("Fission Console! :)\nHello \x1b\xFF\0\0r\x1b\x1b\0\xFF\0g\x1b\x1b\0\0\xff""b\x1b!"));

	auto clear_proc = [](string) { console::clear(); };
	console::register_command(FS_str("clear"), clear_proc);
}

void Console_Layer::destroy() {
	_aligned_free(buffer_view.data);
}

void Console_Layer::handle_character_input(Event::Character_Input in) {
	char ch = (char)in.codepoint;
	switch (ch)
	{
		// ignored
	case '\x1b':
	case '\n':
	case 127:

	break; case '\b':
		if (input.count > 2) input.count -= 1;
	break; case '\r': {
		console::println(input); // Execute Command
		auto cmd = input.substr(2);
		auto it = console::callback_table.find(std::string(cmd.str()));
		if (it != console::callback_table.end()) {
			it->second(cmd);
		}
	//	string cmd;
	//	input.data[input.count] = '\0';
	//	FORMAT_STRING(cmd, "unknown command: %s", (char*)input.data + 2);
	//	console::println(cmd, rgb8(255, 50, 50));
		input.count = 2;
	}
	break; default:
		if (input.count < 71) input.data[input.count++] = (u8&)ch;
		break;
	}
}

void Console_Layer::handle_events(std::vector<Event>& events) {
	for (auto it = events.begin(); it != events.end(); ) {
		switch (it->type)
		{
		case Event_Key_Down: {
			if (it->key_down.key_id == keys::F1) {
				flags ^= layer::show;
				it = events.erase(it);
				continue;
			}
			if(flags & layer::show) { it = events.erase(it); continue; }
			break;
		}
		case Event_Key_Up: {
			if (it->key_down.key_id == keys::F1) {
				it = events.erase(it);
				continue;
			}
			if(flags & layer::show) { it = events.erase(it); continue; }
			break;
		}
		case Event_Character_Input: {
			if (flags & layer::show) {
				handle_character_input(it->character_input);
				it = events.erase(it);
				continue;
			}
			break;
		}
		default:break;
		}
		++it;
	}
}

// "hello\nweh\n"
//  ↑         ↑
//  start     cursor
// OUTPUT == "weh"
string get_string(c8 const* cursor, c8 const* start) {
	c8 const* end = cursor;

	// subtract until we land on a newline
	while (--cursor > start && *cursor != '\n');

	// go to the character after the newline
	if(*cursor == '\n') ++cursor;
	
	// no progress was made, return empty string
	if (cursor == end) return string{.count = 0, .data = (c8*)cursor};
	
	return string{.count = u64(end - cursor), .data = (c8*)cursor};
}

void add_string(Textured_Renderer_2D& r, string str, v2f32 pos) {
	fs::Glyph const* glyph;
	rgb color = colors::White;
	bool use_color = false;

	FS_FOR(str.count) {
		u32 c = str.data[i];
		
		// handle colored console text
		if (c == '\x1b') {
			if (use_color) {
				color = colors::White;
			} else {
				if(i + 2 < str.count) // "not enough bytes for color value [add_string]"
					return; // silent fail
				u8 red   = str.data[++i];
				u8 green = str.data[++i];
				u8 blue  = str.data[++i];
				color = rgb8(red, green, blue);
			}
			use_color = !use_color;
			continue;
		}
		else if (c == '\t') {
			glyph = engine.fonts.console.lookup(' ');
			pos.x += glyph->advance * 4;
			continue;
		}

		glyph = engine.fonts.console.lookup(c);

		if (c != ' ') {
			r.add_glyph(glyph, pos, 1.0f, color);
		}

		pos.x += glyph->advance;
	}
}

void Console_Layer::draw_console_buffer(Textured_Renderer_2D& r, float top, float ystride) {
	auto start = buffer_view.data;
	auto end   = start + buffer_view.count;
	auto cursor = end - 1;

	while (cursor > start) {
		if (*cursor == '\n')
			--cursor;
		else break;
	}
	cursor = std::min(cursor + 1, end);

	while (top >= -ystride && cursor != start) {
		auto str = get_string(cursor, start);
		
		add_string(r, str, {4, top});

		cursor = std::max(str.data - 1, start);
		top -= ystride;

		if (cursor == start) break;
	}
}

// TODO: put this in a header somewhere?
#define Exp_Update(CURRENT, TARGET, LERP_SPEED) fs::lerp(CURRENT, TARGET, 1.0f - std::powf(0.5f, (float)dt * LERP_SPEED))

void Console_Layer::on_update(double dt, Render_Context* ctx) {
	auto& font = engine.fonts.console;
	
	// update current position
	float _top = -(font.height + 1.0f);
	float target = (flags & layer::show)? font.height * 10.0f : _top;
	current = Exp_Update(current, target, 20.0f);

	if (current <= _top + 0.001f) return; // not visible

	auto& r2d = engine.renderer_2d;
	auto& tr2d = engine.textured_renderer_2d;
	float screen_width = (float)engine.graphics.sc_extent.width;

	tr2d.set_font(&font);
	r2d.add_rect({ 0, screen_width, 0, current }, color(colors::Black, 0.94f));
	auto bot = current + font.height;
	r2d.add_rect({ 0, screen_width, current, bot }, color(colors::Black, 0.96f));

	r2d.add_rect({ 0, screen_width, bot, bot + 1 }, colors::Black);
	tr2d.add_string(input, {4, current}, colors::White);

	draw_console_buffer(tr2d, current - font.height, font.height);

	engine.renderer_2d.draw(*ctx);
	engine.textured_renderer_2d.draw(*ctx);
}

__FISSION_END__