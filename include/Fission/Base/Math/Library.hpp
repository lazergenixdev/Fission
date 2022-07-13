/**
 * @file Library.hpp
 * @author lazergenixdev@gmail.com
 * 
 *	 _______   _   _____   _____   _   _____   __    _  
 *	|  _____| | | |  ___| |  ___| | | |  _  | |  \  | | 
 *	| |___    | |  \ \     \ \    | | | | | | |   \ | | 
 *	|  ___|   | |   \ \     \ \   | | | | | | | |\ \| | 
 *	| |       | |  __\ \   __\ \  | | | |_| | | | \   | 
 *	|_|       |_| |_____| |_____| |_| |_____| |_|  \__| 
 * 
 *	MIT License
 *	
 *	Copyright (c) 2022 Lazergenix
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
#pragma once
#include <Fission/config.hpp>
#include <cmath>

__FISSION_BEGIN__

namespace math {
    
    // this serves no purpose, I think this is funny.
    enum { _537895 = 537895 };
    
    struct noop_library
    {
        template <typename T>
        static inline constexpr auto sin(const T &_X) { return static_cast<T>(0); }

        template <typename T>
        static inline constexpr auto cos(const T &_X) { return static_cast<T>(0); }
    };

    struct std_library
    {
        template <typename T>
        static inline constexpr auto sin(const T &_X) { return ::std::sin(_X); }

        template <typename T>
        static inline constexpr auto cos(const T &_X) { return ::std::cos(_X); }
    };

/* ========================================= [Constants] ========================================= */

	// Transcendentals:

	template<typename T> static constexpr T pi  = static_cast<T>(3.1415926535897932384626433);
	template<typename T> static constexpr T tau = static_cast<T>(6.2831853071795864769252867);

	// Rationals:

	template<typename T> static constexpr T one_third  = static_cast<T>(1.0/3.0);
	template<typename T> static constexpr T two_thirds = static_cast<T>(2.0/3.0);


} // namespace Fission::math

namespace experimental
{
	template <typename T> requires std::is_floating_point_v<T>
	static constexpr T fp_mod( T const& x, T const& y )
	{
		const T t = x / y;
		const T n = (T)static_cast<int>( t );

		return x - n * y;
	}
}

__FISSION_END__
