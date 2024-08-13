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

__FISSION_BEGIN__

template <typename T>
struct array {
	u64 count = 0;
	T* data = nullptr;

	constexpr T* begin() const { return data; }
	constexpr T* end()   const { return data + count; }

	//struct reverse_iterator {
	//	T* _it;
	//
	//	constexpr reverse_iterator& operator++() { --_it; return *this; }
	//	constexpr bool operator==(reverse_iterator const&) const = default;
	//};
	//
	//struct reversed_array {
	//	T* _end;
	//	T* _it;
	//
	//	constexpr T* begin() const { return reverse_iterator{ _it }; }
	//	constexpr T* end()   const { return reverse_iterator{ _end }; }
	//};
	//
	//reversed_array reverse() const {
	//	return { ._end = data - 1, ._it = data + count - 1 };
	//}

	void shift_remove(T* elem) {
		if (elem < data || elem >= data + count)
			return;
		for (; elem != data + count; ++elem)
			0[elem] = 0[elem + 1];
		--count;
	}
};

template <typename T>
static constexpr array<T> make_array(void* ptr, u64 count) {
	return array<T>{ .count = count, .data = reinterpret_cast<T*>(ptr) };
}

// TODO: better name?
template <typename T>
struct iterable {
private:
	struct iterator {
		u8* _ptr;
		u32 _stride;

		constexpr T&       operator*()       { return *reinterpret_cast<T*>(_ptr); }
		constexpr T const& operator*() const { return *reinterpret_cast<T*>(_ptr); }
		constexpr iterator& operator++() { _ptr += _stride; return *this; }
		constexpr bool operator==(iterator const& _Right) const { return this->_ptr == _Right._ptr; };
	};

public:
	void* data = nullptr;
	u32 count = 0;
	u32 stride = 0;

	constexpr iterator begin() const { return iterator{ (u8*)data, stride }; }
	constexpr iterator end()   const { return iterator{ (u8*)data + count*stride, stride }; }
};

__FISSION_END__

/**
 *	MIT License
 *
 *	Copyright (c) 2023-2024 lazergenixdev
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