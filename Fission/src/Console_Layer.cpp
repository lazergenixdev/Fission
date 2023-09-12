#include <Fission/Core/Engine.hh>
#include <Fission/Core/Input/Keys.hh>
#include <Fission/Core/Console.hh>
#include <unordered_map>

extern fs::Engine engine;

#define ESCAPE 0x1b

__FISSION_BEGIN__

//////////////////////////////////////////////////////////////////////////////
// Public facing interface
namespace console {

	// std::string because I am lazy
	static std::unordered_map<std::string, console_callback_proc> callback_table;
	static std::mutex access_mutex;

	void register_command(string name, console_callback_proc proc) {
		callback_table.emplace(std::string(name.str()), proc);
	}
	void unregister_command(string name) {
		callback_table.erase(std::string(name.str()));
	}

	void clear() {
		std::scoped_lock lock{access_mutex};
		engine.console_layer.buffer_view.count = 0;
	}

	void println(string text) {
		std::scoped_lock lock{access_mutex};
		u64 space_needed = text.count + 1;

		if (space_needed >= Console_Layer::default_buffer_size - 1)
			return; // silent fail... too long didn't read

		engine.console_layer._reserve_space_for(space_needed);
		auto& console_view = engine.console_layer.buffer_view;

		memcpy(console_view.data + console_view.count, text.data, text.count);
		console_view.count += text.count;
		console_view.data[console_view.count++] = '\n';
	}

	void println(string text, rgb8 color) {
		std::scoped_lock lock{access_mutex};
		u64 space_needed = text.count + 1 + 4 + 1;

		if (space_needed >= Console_Layer::default_buffer_size - 1)
			return; // silent fail... too long didn't read

		engine.console_layer._reserve_space_for(space_needed);
		auto& console_view = engine.console_layer.buffer_view;

		c8 color_code[4] = { ESCAPE, color.r, color.g, color.b };
		memcpy(console_view.data + console_view.count, color_code, sizeof(color_code));
		console_view.count += sizeof(color_code);

		memcpy(console_view.data + console_view.count, text.data, text.count);
		console_view.count += text.count;

		console_view.data[console_view.count++] = ESCAPE;
		console_view.data[console_view.count++] = '\n';
	}

	void print(string text) {
		std::scoped_lock lock{access_mutex};
		if (text.count >= Console_Layer::default_buffer_size - 1)
			return; // silent fail... too long didn't read

		engine.console_layer._reserve_space_for(text.count);
		auto& console_view = engine.console_layer.buffer_view;

		memcpy(console_view.data + console_view.count, text.data, text.count);
		console_view.count += text.count;
	}
	void print(string text, rgb8 color) {
		std::scoped_lock lock{access_mutex};
		u64 space_needed = text.count + 4 + 1;

		if (space_needed >= Console_Layer::default_buffer_size - 1)
			return; // silent fail... too long didn't read

		engine.console_layer._reserve_space_for(space_needed);
		auto& console_view = engine.console_layer.buffer_view;

		c8 color_code[4] = { ESCAPE, color.r, color.g, color.b };
		memcpy(console_view.data + console_view.count, color_code, 4);
		console_view.count += 4;

		memcpy(console_view.data + console_view.count, text.data, text.count);
		console_view.count += text.count;

		console_view.data[console_view.count++] = ESCAPE;
	}

}
//////////////////////////////////////////////////////////////////////////////

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

void Console_Layer::setup_console_api() {
	input.count = 2;
	input.data  = input_buffer;
	memset(input_buffer, 0, sizeof(input_buffer));
	input_buffer[0] = '>';
	input_buffer[1] = ' ';

	buffer_capacity = default_buffer_size;
	buffer_view.data = (c8*)_aligned_malloc(buffer_capacity, 32);
	buffer_view.count = 0;

	command_history_buffer.reserve(256);
	command_history_ends.reserve(32);

	console::println(FS_str("Fission Console! :)\nHello \x1b\xFF\0\0r\x1b\x1b\0\xFF\0g\x1b\x1b\0\0\xFF""b\x1b!"));

	{
		auto callback = [](string) { console::clear(); };
		console::register_command(FS_str("clear"), callback);
	} {
		auto callback = [](string input) { console::println(input); };
		console::register_command(FS_str("echo"), callback);
	}
}

void Console_Layer::destroy() {
	_aligned_free(buffer_view.data);
}

string find_command_action(string s) {
	u64 len = 0;
	while (len < s.count) {
		if (s.data[len] == ' ')
			break;
		++len;
	}
	return string{.count = len, .data = s.data};
}

string find_command_arguments(string s) {
	u64 cursor = 0;
	bool found_space = false;

	while (cursor < s.count) {
		if (s.data[cursor] == ' ') {
			found_space = true;
			break;
		}
		++cursor;
	}

	if (!found_space) return string{.count = 0, .data = s.data + s.count};

	++cursor;

	// find anything other than a space:
	while (cursor < s.count) {
		if (s.data[cursor] != ' ')
			break;
		++cursor;
	}

	return string{.count = s.count - cursor, .data = s.data + cursor};
}

// For Pasting from clipboard
bool acceptable_character(char c) {
	switch (c)
	{
	case '\n':
	case '\r':
		return false;
	default:return c;
	}
}

void Console_Layer::handle_character_input(Event::Character_Input in) {
	char ch = (char)in.codepoint;
	switch (ch)
	{
		// ignored
	case ESCAPE:
	case '\n':

	break; case 22: { // Ctrl+V
#if defined(FISSION_PLATFORM_WINDOWS)
#define debug_print(X)
		HGLOBAL   hglb;
		LPCSTR    lptstr;

		if (!IsClipboardFormatAvailable(CF_TEXT))
		{ debug_print("format not available\n"); break; }
		if (!OpenClipboard(engine.window._handle))
		{ debug_print("open clipboard failed\n"); break; }

		hglb = GetClipboardData(CF_TEXT);
		if (hglb != NULL)
		{
			lptstr = (LPCSTR)GlobalLock(hglb);
			if (lptstr != NULL)
			{
				while (input.count < 71 && *lptstr != 0) {
					char ch = *lptstr++;
					if(acceptable_character(ch))
						input.data[input.count++] = ch;
				}
				input_cursor = (int)input.count;
				GlobalUnlock(hglb);
			}
		}
		CloseClipboard();
#undef debug_print
#elif defined(FISSION_PLATFORM_LINUX)
#endif // FISSION_PLATFORM_
	}

	break; case '\b': {
		// copy command into input
		if (current_command != -1) {
			auto cmd = command_from_history();

			memcpy(input.data + 2, cmd.data, cmd.count);

			input.count = cmd.count + 2;
			input_cursor = (int)input.count;
			current_command = -1;
		}

		if (input_cursor > 2) {
			// move characters down
			for (int i = input_cursor - 1; i < input.count; ++i) {
				input.data[i] = input.data[i + 1];
			}

			--input.count;
			--input_cursor;
		}
	}
	break; case '\r': {
		if (input.count <= 2 && current_command == -1) break;

		auto command = (current_command == -1)? input.substr(2) : command_from_history();

		console::print(FS_str("> "));
		console::println(command);

		auto action = find_command_action(command);

		auto it = console::callback_table.find(std::string(action.str()));
		if (it != console::callback_table.end()) {
		// Execute Command
			it->second(find_command_arguments(command));
		} else {
			console::print(FS_str("\x1b\xFF\x25\x25unknown command: "));
			console::print(action);
			console::print(FS_str("\x1b\n"));
		}

		// copy command to command history buffer
		FS_FOR(command.count) {
			command_history_buffer.emplace_back(command.data[i]);
		}

		// insert command end position into end buffer
		command_history_ends.emplace_back((u32)command_history_buffer.size());

		input.count  = 2;
		input_cursor = 2;
		current_command = -1;
		buffer_view_offset = 0;
	}
	break; default: {
		// copy command into input
		if (current_command != -1) {
			auto cmd = command_from_history();

			memcpy(input.data + 2, cmd.data, cmd.count);

			input.count = cmd.count + 2;
			input_cursor = (int)input.count;
			current_command = -1;
		}

		// have enough room for a new character
		if (input.count < 71) {
			
			// shift characters to make room for new one
			for (int i = int(input.count-1); i >= input_cursor; --i) {
				input.data[i + 1] = input.data[i];
			}

			input.data[input_cursor] = (u8&)ch;

			++input_cursor;
			++input.count;
		}
		break;
	}
	}
}

string Console_Layer::command_from_history()
{
	string command;

	auto index = (s64)command_history_ends.size() - 1 - current_command;
	u32 start = (index > 0) ? command_history_ends[index - 1] : 0;
	
	command.data = command_history_buffer.data() + start;
	command.count = command_history_ends[index] - start;
	return command;
}

void Console_Layer::handle_events(std::vector<Event>& events) {
	for (auto it = events.begin(); it != events.end(); ) {
		switch (it->type)
		{
		case Event_Key_Down: {
			if (it->key_down.key_id == keys::F1) {
				if (flags & layer::enable) flags ^= layer::show;
				it = events.erase(it);
				continue;
			}
			if(flags & layer::show) {
				switch(it->key_down.key_id) {
				break; case keys::Up: {
					current_command = min(current_command+1, (s64)command_history_ends.size()-1);
				}
				break; case keys::Down: {
					current_command = max(current_command-1, -1);
				}
				break; case keys::Left: {
					input_cursor = max(input_cursor-1, 2);
				}
				break; case keys::Right: {
					input_cursor = min(input_cursor+1, input.count);
				}
				break; case keys::Mouse_WheelUp: {
					++buffer_view_offset;
				}
				break; case keys::Mouse_WheelDown: {
					buffer_view_offset = max(buffer_view_offset-1, 0);
				}
				break; case keys::Delete: {
					if (input_cursor < input.count) {
						// move characters down
						for (int i = input_cursor; i < input.count; ++i) {
							input.data[i] = input.data[i + 1];
						}

						--input.count;
					}
				}
				}
				it = events.erase(it);
				continue;
			}
			break;
		}
		case Event_Key_Up: {
			if (it->key_up.key_id == keys::F1) {
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
//	engine.debug_layer.add("current_cmd = %i", (int)current_command);
//	engine.debug_layer.add("buffer_view_offset = %i", buffer_view_offset);
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
		if (c == ESCAPE) {
			if (use_color) {
				color = colors::White;
			} else {
				if(i + 2 >= str.count) // "not enough bytes for color value [add_string]"
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

	{
		int newline_count = -1;
		while (cursor > start) {
			if (*cursor == '\n')
				++newline_count;

			--cursor;
		}

		buffer_view_offset = min(buffer_view_offset, newline_count);

		if (buffer_view_offset == newline_count)
			flags |=  layer::console_end_of_buffer;
		else
			flags &=~ layer::console_end_of_buffer;
	}

	int count = -1;
	cursor = end - 1;
	while (true) {
		if (*cursor == '\n')
			++count;

		if (cursor > start && count < buffer_view_offset); else
			break;

		--cursor;
	}

	while (top >= -ystride && cursor > start) {
		auto str = get_string(cursor, start);
		
		add_string(r, str, {4, top});

		cursor = std::max(str.data - 1, start);
		top -= ystride;

		if (cursor == start) break;
	}
}

void Console_Layer::on_update(double dt, Render_Context* ctx) {
	auto& font = engine.fonts.console;
	
	// update current position
	float _top = -(font.height + 1.0f);
	float target = (flags & layer::show)? font.height * 10.0f : _top;
	position = math::exp_update(position, target, (float)dt, 20.0f);

	if (position <= _top + 0.001f) return; // not visible

	auto& r2d = engine.renderer_2d;
	auto& tr2d = engine.textured_renderer_2d;
	float screen_width = (float)engine.graphics.sc_extent.width;

	tr2d.set_font(&font);
	r2d.add_rect({ 0, screen_width, 0, position }, color(colors::Black, 0.94f));
	auto bot = position + font.height;
	r2d.add_rect({ 0, screen_width, position, bot }, color(colors::Black, 0.96f));

	if (flags & layer::console_end_of_buffer)
	r2d.add_rect({ 0, screen_width, position, position + 1 }, colors::Gray);

	r2d.add_rect({ 0, screen_width, bot, bot + 1 }, colors::Black);
	v2f32 string_size;
	
	// Show from user input
	if(current_command == -1) {
		tr2d.add_string(input, {4, position}, colors::White);
		auto width = font.table.fallback.advance;
		r2d.add_rect(rf32::from_topleft(4+width*float(input_cursor), position+1, 1, font.height-2), colors::White);

	// Show from history
	} else {
		string_size = tr2d.add_string(FS_str("> "), {4, position}, colors::White);

		auto index = (s64)command_history_ends.size() - 1 - current_command;

		u32 start = (index > 0) ? command_history_ends[index - 1] : 0;
		string command;
		command.data  = command_history_buffer.data() + start;
		command.count = command_history_ends[index] - start;
		tr2d.add_string(command, {4+string_size.x, position}, colors::White);
	}

	draw_console_buffer(tr2d, position - font.height, font.height);

	engine.renderer_2d.draw(*ctx);
	engine.textured_renderer_2d.draw(*ctx);
}

__FISSION_END__