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
#include <Fission/Base/Memory.hpp>

__FISSION_BEGIN__

//! @note: DO NOT use with types that have a move constructor,
//!          only does a shallow copy when reallocating buffer
template <typename _Ty>
struct dynamic_array
{
	using type = _Ty;
	
	type* data = nullptr;
	u32   count = 0; // no need for u64, arrays will never get that big (hopefully, or we have a bigger problem)
	u32   allocated = 0;
	
	constexpr dynamic_array() = default;
	
	dynamic_array(u32 initial_capacity)
	:	allocated(initial_capacity), count(0), data(nullptr)
	{
		data = FISSION_DEFAULT_ALLOC(initial_capacity * sizeof(type));
	}
	
	void push_back(type const& value) {
		reserve(count+1);
		data[count++] = value;
	}

	void reserve(u32 new_capacity) {
		if (new_capacity <= allocated) return;
		
		auto ptr = FISSION_DEFAULT_ALLOC(new_capacity);
		if (data != nullptr) {
			memcpy(ptr, data, count * sizeof(type));
			FISSION_DEFAULT_FREE(data);
		}
		data = ptr;
	}

	constexpr type const* begin() const { return data; }
	constexpr type      * begin()       { return data; }
	constexpr type const* end() const { return data + count; }
	constexpr type      * end()       { return data + count; }
};

__FISSION_END__

/**
 *	MIT License
 *
 *	Copyright (c) 2023 lazergenixdev
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