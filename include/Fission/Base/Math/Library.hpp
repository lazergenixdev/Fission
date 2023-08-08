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

        template <typename T>
        static inline constexpr auto sqrt(const T& _X) { return static_cast<T>(0); }
    };

    struct std_library
    {
        template <typename T>
        static inline constexpr auto sin(const T &_X) { return ::std::sin(_X); }

        template <typename T>
        static inline constexpr auto cos(const T &_X) { return ::std::cos(_X); }

        template <typename T> requires std::is_floating_point_v<T>
        static inline constexpr auto sqrt(const T &_X) {
            if constexpr (std::is_same_v<T, float>) {
                return::sqrtf(_X);
            }
            if constexpr (std::is_same_v<T, double>) {
                return::sqrt(_X);
            }
        }
    };

/* ========================================= [Constants] ========================================= */

	// Transcendentals:

	static constexpr float pi  = static_cast<float>(FS_PI);
	static constexpr float tau = static_cast<float>(FS_TAU);

	// Rationals:

	static constexpr float one_third  = static_cast<float>(1.0/3.0);
	static constexpr float two_thirds = static_cast<float>(2.0/3.0);


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