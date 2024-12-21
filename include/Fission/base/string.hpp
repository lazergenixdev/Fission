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
#include <Fission/config.hpp>
#include <Fission/base/array.hpp>
#include <Fission/base/math/library.hpp>
#include <string>
__FISSION_BEGIN__

struct string {
    size_t count = 0;
    c8* data  = nullptr;

    inline constexpr string() = default;

    template <size_t literal_size>
    inline constexpr string(char const(&literal)[literal_size])
    :	count(literal_size-1),
        data((c8*)literal)
    {}

    template <size_t buffer_size>
    inline constexpr string(char (&buffer)[buffer_size])
    :	count(buffer_size),
        data((c8*)buffer)
    {}

    template <typename string_type>
    inline constexpr string(string_type const& s)
    :   count(s.size()),
        data((c8*)s.data())
    {}

    inline constexpr string(void const* ptr, u64 size)
    :	count(size),
        data((c8*)ptr)
    {}

    template <size_t buffer_size>
    inline constexpr string from_buffer(char (&buffer)[buffer_size]) {
        return {buffer, buffer_size};
    }
    
    NO_DISCARD inline std::string str() const {
        return {(char*)data, count};
    }
    NO_DISCARD inline std::string_view view() const {
        return {(char*)data, count};
    }

    inline constexpr string substr(u64 offset, u64 max_count = 0xFFFFFFFF) const {
        return {data + offset, math::min(count - offset, max_count)};
    }

    inline constexpr bool is_empty() const { return count == 0; }

    NO_DISCARD inline constexpr c8* begin() const { return data; }
    NO_DISCARD inline constexpr c8* end  () const { return data + count; }
};

// str == "string"
template <size_t Right_Size>
static constexpr bool operator==(string const& left, char const(&right)[Right_Size]) {
    if (left.count != Right_Size - 1) return false;
    for (u64 i = 0; i < left.count; ++i) {
        if (left.data[i] != (c8)right[i]) {
            return false;
        }
    }
    return true;
}

struct string_utf16 {
    u64 count = 0;
    c16* data = nullptr;
};



// std library is crying rn 😭😭😭
// -> it's really this simple..
struct string_view {
    u32 offset;
    u32 count;

    inline constexpr string absolute(c8* base) const {
        return string{base + offset, count};
    }
};


struct string_array {
    dynamic_array<c8> buffer;

    string_array() : buffer(64) {}

    void insert_string(string s) {
        buffer.reserve(u32(buffer.count) + u32(s.count) + 1);
        for (auto&& c: s) buffer.data[buffer.count++] = c;
        buffer.data[buffer.count++] = 0; // null terminator
    //  memcpy(buffer.data + buffer.count, s.data, s.count); // better to do a memcpy?
    }

    struct string_array_iterator {
        c8* value;

        constexpr auto operator!=(string_array_iterator const& iter) const { return value < iter.value; }
        constexpr c8* operator*() const { return value; }
        constexpr string_array_iterator& operator++() { do { ++value; } while (*value != 0); ++value; return*this; }
    };

    inline constexpr auto begin()const { return string_array_iterator{ buffer.data }; }
    inline constexpr auto end()const { return string_array_iterator{ buffer.data + (buffer.count? buffer.count - 1 : 0)}; }
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
