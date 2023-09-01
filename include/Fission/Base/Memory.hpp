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
 *
 * Some reference: https://semver.org/
 */
#pragma once
#include <Fission/config.hpp>
#include <memory.h>

__FISSION_BEGIN__

#define FISSION_DEFAULT_ALLOC(Size) _aligned_malloc(Size, 64)
#define FISSION_DEFAULT_FREE(Ptr)   _aligned_free(Ptr)

//! @brief Default Bump Allocator
struct bump_allocator
{
	u8* base            = nullptr;
	u64 bytes_allocated = 0;
	u64 capacity        = 0;

	void* alloc(u64 size) {
		auto ptr = base + bytes_allocated;
		bytes_allocated += size;
		if (bytes_allocated > capacity) {
			return nullptr; // have fun! :)
		}
		return ptr;
	}

	template <typename T>
	T* alloc(u64 count) {
		auto ptr = base + bytes_allocated;
		bytes_allocated += count * sizeof(T);
		if (bytes_allocated > capacity) {
			return nullptr; // have fun! :)
		}
		return reinterpret_cast<T*>(ptr);
	}

	bump_allocator() = default;
	bump_allocator(u64 capacity)
	:	base((u8*)FISSION_DEFAULT_ALLOC(capacity)), capacity(capacity)
	{}

	void create(u64 capacity) {
		this->capacity = capacity;
		bytes_allocated = 0;
		base = (u8*)FISSION_DEFAULT_ALLOC(capacity);
	}

	void* release() {
		auto ptr = base;
		base = nullptr;
		return ptr;
	}

	~bump_allocator() {
		if(base) FISSION_DEFAULT_FREE(base);
	}
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