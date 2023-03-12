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
 *
 * How to do version numbers:
 *   https://semver.org/
 * 
 */
#include <Fission/Base/Types.hpp>
#include <format>

__FISSION_BEGIN__

/*! @brief Normal Version Number */
struct normal_version
{
	using type = i32;

	// Version: Major.Minor.Patch
	// e.g. 2.2.0
	type Major,Minor,Patch;


	constexpr normal_version() noexcept : Major(0), Minor(0), Patch(0) {}

	template <std::integral T>
	constexpr normal_version( T major, T minor, T patch ) noexcept:
		Major( static_cast<type>(major) ),
		Minor( static_cast<type>(minor) ),
		Patch( static_cast<type>(patch) )
	{}

	constexpr auto operator<=>(normal_version const&) const noexcept = default;
	constexpr bool operator ==(normal_version const&) const noexcept = default;

	std::string string() const { return std::format( "{}.{}.{}", Major, Minor, Patch ); }

}; // Fission::normal_version

using version = normal_version;

// vvv This is cancer vvv

//struct version
//{
//public:
//	struct identifier
//	{
//		union {
//			char str[8];
//			struct {
//				u32 _;
//				u32 number;
//			};
//		};
//	};
//
//public:
//	normal_version ver;
//	identifier ids[2];
//
//
//};

__FISSION_END__

/**
 *	MIT License
 *
 *	Copyright (c) 2021-2023 Lazergenix Software
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
 * 
 */