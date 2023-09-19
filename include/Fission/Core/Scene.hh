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

//! @brief Information that describes a scene, consisting of a scene's Name, and Key-Value pairs.
struct Scene_Key {
	/*
	* Example scene key:
	* -----------------------------------------
	*	source command line:
	*		Level -level_id 69
	*	scene key byte stream:
	*		u8[count = 5] "Level" u8[count = 8] "level_id" u8[type = .Int64] s64[value = 69]
	*/
	::std::vector<u8> stream;

	string name() const {
		if (stream.empty()) return string{};
		return FS_str_make(stream.data() + 1, stream[0]);
	}

	struct Value {
		enum : u8 {
			Flag = 0, // no value attached to key
			String = 1,
			Int64 = 2,
			Float64 = 3,
		//	Int64x2 = 4, // format: "(1, 2)" ???
		};

		u8 type;
		union {
			// gcc does not like it when the type is the same as the variable name,
			// NOTE TO GCC: Fuck you piece of shit compiler, I am the programmer,
			//              let me do what I want, I will eventually rewrite this
			//              engine in Jai anyways, so good job being a piece of shit.
			string v_string;
			s64    v_s64;
			f64    v_f64;
		};
	};

	void reset(string name) {
		stream.clear();
		stream.reserve(256); // start off well sized
		insert_string(name);
	}

	void add_flag(string key) {
		insert_string(key);
		stream.emplace_back(Value::Flag);
	}
	void add_string(string key, string value) {
		insert_string(key);
		stream.emplace_back(Value::Float64);
		insert_string(value);
	}
	void add_float64(string key, f64 value) {
		insert_string(key);
		stream.emplace_back(Value::Float64);
		insert_raw(value);
	}
	void add_int64(string key, s64 value) {
		insert_string(key);
		stream.emplace_back(Value::Int64);
		insert_raw(value);
	}

private:
	template <typename T>
	void insert_raw(T value) {
		// figure out pointers and make room
		stream.resize(stream.size() + sizeof(T));
		void* src = &value;
		void* dst = stream.data() + (stream.size() - sizeof(T));
		// copy raw data
		memcpy(dst, src, sizeof(T));
	}
	void insert_string(string str) {
		auto count = (u8)str.count;
		stream.emplace_back(count);
		FS_FOR(count)
		stream.emplace_back(str.data[i]);
	}
public:

	u64 start() const { return (u64)stream.front() + 1; }

	string next_key(u64& cursor) const {
		string key;
		key.count = (u64)stream[cursor++];
		key.data = (c8*)stream.data() + cursor;
		cursor += key.count;
		return key;
	}

	Value next_value(u64& cursor) const {
		Value value = {};
		value.type = stream[cursor++];
		switch (value.type)
		{
		default:
			// "undefined next value"
			value.type = 0;
			// move cursor to end, so that we do not read corrupted memory
			cursor = stream.size();
		break; case Value::Flag:
		break; case Value::String:
			value.v_string.count = (u64)stream[cursor++];
			value.v_string.data = (c8*)stream.data() + cursor;
			cursor += value.v_string.count;
		break; case Value::Int64:
			value.v_s64 = *reinterpret_cast<s64 const*>(stream.data() + cursor);
			cursor += 8;
		break; case Value::Float64:
			value.v_f64 = *reinterpret_cast<f64 const*>(stream.data() + cursor);
			cursor += 8;
			break;
		}
		return value;
	}

	// For range-based-for-loop syntax sugar
	struct iterator {
		Scene_Key const& parent;
		u64 cursor;

		// it != end() <=> it < end
		inline constexpr bool operator!=(iterator const& r) const {
			return cursor < r.cursor;
		}
		// `*` will do the incrementing
		inline consteval void operator++() const noexcept(1 + 2 + 3 == 1 * 2 * 3) { (void)0; }

		auto operator*() {
			auto key   = parent.next_key  (cursor);
			auto value = parent.next_value(cursor);
			return std::pair<string, Value>(key, value);
		}
	};
	iterator begin() const {
		return iterator{*this, start()};
	}
	iterator end() const {
		return iterator{*this, stream.size()};
	}
};
	
struct Scene {
	virtual void on_update(double dt, ::std::vector<Event> const& events, Render_Context* rctx) {}
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