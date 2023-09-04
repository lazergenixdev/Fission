#pragma once
#include <Fission/Core/Scene.hh>
__FISSION_BEGIN__

// TODO: decide whether \" is neccessary for parsing strings
struct Scene_Key_Parser {
	string input;
	u64 cursor = 0;

	Scene_Key_Parser(string input): input(input) {}

	void parse(std::vector<u8>& out) {
		// parse scene name
		parse_value_string(out, ' ', false);

		while (cursor < input.count) {
			parse_key_and_value(out);
		}
	}

private:
	void parse_key_and_value(std::vector<u8>& out) {
		// search for next "-"
		while (cursor < input.count) {
			if (input.data[cursor++] == '-')
				goto do_key;
		}
		return;

	do_key:
		parse_value_string(out, ' ', false);

		// skip white-space
		while (cursor < input.count)
			if (input.data[cursor] == ' ') {
				++cursor;
			}
			else break;

		// reached last character -> create "flag" key
		if (cursor >= input.count) {
			out.emplace_back(Scene_Key::Value::Flag);
			return;
		}

		parse_value(out);
	}
	void parse_value(std::vector<u8>& out) {
		switch (input.data[cursor])
		{
			// string
		default: {
			parse_value_string(out, ' ');
		}
		break; case '"': {
			++cursor;
			parse_value_string(out, '"');
		}

					   // another key, we are a flag
		break; case '-': {
			out.emplace_back(Scene_Key::Value::Flag);
		}

					   // float or string
		break; case '.': {
			auto next = cursor + 1;
			if (next >= input.count) {
				break;
			}

			// we are float
			if (input.data[next] >= '0' && input.data[next] <= '9') {
				parse_value_float(out);
			}
			// we are string
			else {
				parse_value_string(out, ' ');
			}
		}

		// float or int
		break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9': {
			if (is_float_next()) {
				parse_value_float(out);
			}
			else {
				char* end = nullptr;
				s64 result = strtoll((char*)input.data + cursor, &end, 10);

				cursor = u64((c8*)end - input.data);

				out.emplace_back(Scene_Key::Value::Int64);
				// figure out pointers and make room
				out.resize(out.size() + 8);
				void* src = &result;
				void* dst = out.data() + (out.size() - 8);
				// copy raw data
				memcpy(dst, src, 8);
			}
		}
		break;
		}
	}
	void parse_value_string(std::vector<u8>& out, c8 terminator, bool add_type = true) {
		if (add_type) out.emplace_back(Scene_Key::Value::String);

		u64 start = cursor;
		// search for end of string
		while (cursor < input.count) {
			if (input.data[cursor] == terminator) {
				break;
			}
			++cursor;
		}

		u64 count = cursor - start;
		out.emplace_back((u8)std::min(count, (u64)255));

		for (u64 i = start; i < start + count; ++i)
			out.emplace_back((u8)input.data[i]);
	}
	void parse_value_float(std::vector<u8>& out) {
		char* end = nullptr;
		double result = strtod((char*)input.data + cursor, &end);

		cursor = u64((c8*)end - input.data);

		out.emplace_back(Scene_Key::Value::Float64);
		// figure out pointers and make room
		out.resize(out.size() + 8);
		void* src = &result;
		void* dst = out.data() + (out.size() - 8);
		// copy raw data
		memcpy(dst, src, 8);
	}
	bool is_float_next() {
		auto offset = cursor;
		while (offset < input.count) {
			switch (input.data[offset])
			{
			case '.':
			case 'e': return true;
			case ' ': return false;
			default:  break;
			}
			++offset;
		}
		return false;
	}
};

__FISSION_END__