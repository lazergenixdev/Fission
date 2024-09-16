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
#include <string>
__FISSION_BEGIN__

struct string {
    u64 count = 0; //! note: u32 is probably good enough for all string counts
    c8* data  = nullptr;

    inline constexpr string() = default;

    template <size_t _Literal_Size>
    inline constexpr string(char const(&_Literal)[_Literal_Size])
    :	count(_Literal_Size-1),
        data((c8*)_Literal)
    {}

    template <size_t _Buffer_Size>
    inline constexpr string(char (&_Buffer)[_Buffer_Size])
    :	count(_Buffer_Size),
        data((c8*)_Buffer)
    {}

    template <typename _String_Type>
    inline constexpr string(_String_Type const& _String)
    :   count(_String.size()),
        data((c8*)_String.data())
    {}

    inline constexpr string(void const* _Data, u64 _Count)
    :	count(_Count),
        data((c8*)_Data)
    {}

    template <size_t _Buffer_Size>
    inline constexpr string from_buffer(char (&_Buffer)[_Buffer_Size]) {
        return string{_Buffer, _Buffer_Size};
    }
    
    inline std::string      str () const { return std::string     ((char*)data, count); }
    inline std::string_view view() const { return std::string_view((char*)data, count); }

    inline constexpr string substr(u64 _Offset, u64 _Count = 0xFFFFFFFF) const {
        return string{data + _Offset, std::min(_Count - _Offset, _Count)};
    }

    inline constexpr bool is_empty() const {
        return count == 0;
    }

    inline constexpr c8* begin() const { return data; }
    inline constexpr c8* end  () const { return data + count; }
};

////////////////////////////////////////////////////////////////////
// EXAMPLE USAGE:
//	char buffer[128];
//	console::print("I am %i and my name is %s"_fmt(buffer, age, name));
struct _Formattable_String {
    const char* format;

    template <size_t buffer_size, typename...T>
    string operator()(char (&buffer)[buffer_size], T&&...args) const {
        auto count = (u64)snprintf(buffer, buffer_size, format, std::forward<T>(args)...);
        return string{reinterpret_cast<c8*>(buffer), count};
    }
};
static _Formattable_String constexpr operator""_fmt(const char* str, std::size_t) {
    return {str};
}
////////////////////////////////////////////////////////////////////

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

template <typename C>
u64 strlen (C const* c_string) {
    u64 count = 0;
    while (c_string[count] != 0) ++count;
    return count;
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
        return string{base + offset, (u64)this->count};
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

        constexpr auto operator!=(string_array_iterator const& iter) const { return value < iter.value; };
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
