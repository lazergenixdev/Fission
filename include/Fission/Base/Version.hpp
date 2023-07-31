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
#include <compare>
#include <concepts>

#ifdef assert
#undef assert
#endif
#if FISSION_DEBUG
#define assert(E) 
#else
#define assert(E) 
#endif

__FISSION_BEGIN__

/*! @brief Normal Version Number */
struct version
{
	using type = u32;

	// Version: Major.Minor.Patch
	// e.g. 2.2.0
	type Major,Minor,Patch;


	constexpr version() noexcept : Major(0), Minor(0), Patch(0) {}

	template <::std::integral T >
	constexpr version( T major, T minor, T patch ) noexcept:
		Major( static_cast<type>(major) ),
		Minor( static_cast<type>(minor) ),
		Patch( static_cast<type>(patch) )
	{}

	constexpr auto operator<=>(version const&) const noexcept = default;
	constexpr bool operator ==(version const&) const noexcept = default;

}; // Fission::version

/*! @brief Compact Version Number */
struct compressed_version
{
	// Format:  Major  | Minor         | Patch
	//          010101   0101010101010   0101010101010
	// Max:     63       8,191           8,191         => 63.8191.8191
	// 
	//          6 bits + 13 bits       + 13 bits       = 32 bit
	//          High Bits <--          --> Low Bits
	u32 _version;

	static constexpr u32 _Mask_Major = 0xFC00'0000;
	static constexpr u32 _Mask_Minor = 0x03FF'E000;
	static constexpr u32 _Mask_Patch = 0x0000'1FFF;

	constexpr compressed_version() noexcept: _version(0) {}

	constexpr compressed_version( int major, int minor, int patch ) noexcept:
		_version( (major << 26) | (minor << 13) | patch )
	{
		assert(major > 0 && major < 64);
		assert(minor > 0 && minor < 8192);
		assert(patch > 0 && patch < 8192);
	}

	// assume all numbers are within limits
	constexpr compressed_version( const version& v ) noexcept:
		_version( (v.Major << 26) | (v.Minor << 13) | v.Patch )
	{
		assert(v.Major > 0 && v.Major < 64);
		assert(v.Minor > 0 && v.Minor < 8192);
		assert(v.Patch > 0 && v.Patch < 8192);
	}

	constexpr version uncompress() const {
		return version( (_version) >> 26, (_version & _Mask_Minor) >> 13, _version & _Mask_Patch );
	}

}; // Fission::compressed_version

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