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
#include <optional>

__FISSION_BEGIN__

struct version;
struct compressed_version;

/*! @brief Semantic Version */
struct version
{
	// Version: Major.Minor.Patch
	// e.g. 2.2.0
	u32 Major,Minor,Patch;

	constexpr version() : Major(0), Minor(0), Patch(0) {}

	template <typename T>
	constexpr version(T major, T minor, T patch):
		Major(static_cast<u32>(major)),
		Minor(static_cast<u32>(minor)),
		Patch(static_cast<u32>(patch))
	{}

	constexpr auto operator<=>(version const&) const = default;
	constexpr bool operator ==(version const&) const = default;

	//constexpr std::optional<compressed_version> compress();
};

/*! @brief Compact Version Number */
struct compressed_version
{
	// Format:  Major      Minor         Patch
	//          01010101   01010101010   0101010101010
	// Max:     255        2047          8191          = 255.2047.8191
	// 
	//          8 bits   + 11 bits     + 13 bits       = 32 bit
	//            High <--          --> Low
	
	u32 Major  :  8;
	u32 Minor  : 11;
	u32 Patch  : 13;

	constexpr compressed_version(): Major(0), Minor(0), Patch(0) {}

	// assume all numbers are within limits
	constexpr compressed_version(u32 major, u32 minor, u32 patch):
		Major(major), Minor(minor), Patch(patch)
	{}

	// assume all numbers are within limits
	constexpr compressed_version(const version& v):
		Major(v.Major), Minor(v.Minor), Patch(v.Patch)
	{}

	constexpr version uncompress() const {
		return version(Major, Minor, Patch);
	}

	template <u32 major, u32 minor, u32 patch>
	static constexpr compressed_version make =
		std::conditional_t<major < 256 && minor < 2048 && patch < 8192,
						   compressed_version, void>(major, minor, patch);

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
