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
#include <Fission/Base/String.hpp>
#include <vector>

__FISSION_BEGIN__

struct Event;
struct Render_Context;

// scenes:
//   OOOOOOOO
//auto FS_COMBINE2(_,__LINE__) = "Main";
//auto FS_COMBINE2(_,__LINE__) = "Intro";
//auto FS_COMBINE2(_,__LINE__) = "Default_Editor";
//auto FS_COMBINE2(_,__LINE__) = "Editor";
//auto FS_COMBINE2(_,__LINE__) = "Level";
//auto FS_COMBINE2(_,__LINE__) = "Level_Select";
//auto FS_COMBINE2(_,__LINE__) = "User_Select";
//auto FS_COMBINE2(_,__LINE__) = "Online_Select";
//auto FS_COMBINE2(_,__LINE__) = "Search";

// rythmn_dash.exe Level --path "asidufhsia.rdl" --start --song_offset 450
// id       [SIZE]:8 ["Level"]
// argument [KEY_SIZE]:8 ["path"]        [STRING_ID]:8 [STRING_SIZE]:8 ["asidufhsia.rdl"]
// argument [KEY_SIZE]:8 ["start"]       [FLAG_ID]:8
// argument [KEY_SIZE]:8 ["song_offset"] [INT64_ID]:8  [450]

struct Scene_Key {
	std::vector<u8> stream;

	string id() const {
		if (stream.empty()) return string{};

		return FS_str_make(stream.data() + 1, stream[0]);
	}

	struct Value {
		enum {
			Flag, // no value attached to key
			String,
			Int64,
		};

		u8 type;
		union {
			string string;
			s64    int64;
		};
	};

	u64 start() const { return 0; }

	string next_key(u64& cursor) const {
		return {};
	}

	Value next_value(u64& cursor) const {
		return {};
	}
};

#if 0
namespace test {
	enum {
		key_path,
		key_start,
		key_song_offset,
	};
	int map(string);

	void do_thing(Scene_Key scene_key) {
		u64 cursor = scene_key.start();

		// scene details
		string _path;
		bool _start = false;
		s64 _offset = 0;

		while (cursor < scene_key.stream.size()) {
			auto key   = scene_key.next_key(cursor);
			auto value = scene_key.next_value(cursor);

			switch (map(key))
			{
			default:
			break;case key_path: {
				if (value.type == value.String) {
					_path = value.string;
				}
			}
			break;case key_start: {
				if (value.type == value.Flag) {
					_start = true;
				}
			}
			break;case key_song_offset: {
				if (value.type == value.Int64) {
					_offset = value.int64;
				}
			}
			}
		}

	}
}
#endif
	
struct Scene {
	virtual void on_update(double dt, std::vector<Event> const& events, Render_Context* rctx) {}
	virtual void on_resize() {}
	virtual ~Scene() = default;
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