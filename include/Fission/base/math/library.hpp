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

namespace math
{
	template <typename type, typename from>
	inline constexpr type max(type a, from b) {
        auto const _b = static_cast<type>(b);
        return (a > _b) ? a : _b;
	}

    template <typename type, typename from>
    inline constexpr type min(type a, from b) {
        auto const _b = static_cast<type>(b);
        return (a < _b) ? a : _b;
    }
	
	template <typename T, typename F>
	inline constexpr T lerp(T const& left, T const& right, F x) {
		return left * ((F)1 - x) + right * x;
	}

    template <typename T>
    static inline constexpr T lerp_speed(T dt, T speed) {
        return T(1.0) - std::pow(T(0.5), dt * speed);
    }

    template <typename T>
    static inline constexpr T exp_update(T current, T target, T dt, T speed) {
        return lerp(current, target, lerp_speed(dt, speed));
    }

    template <typename T>
    static constexpr T floor(T const& x) {
        static_assert(std::is_floating_point_v<T>);
        T const q = static_cast<T>(static_cast<int>(x));
        return q - static_cast<T>(x < 0);
    }

    template <typename T>
    static constexpr T mod(T const& x, T const& y)
    {
        // This is slightly faster than glm::mod's `a - b * floor(a / b)`
        // Tested with clang -O2 and with clang -O3
        if constexpr (std::is_floating_point_v<T>) {
            T const q = static_cast<T>(static_cast<int>(x / y));
            T const f = static_cast<T>((y < 0)^(x < 0)); // fix for q
            // x = q * y + r   with  0 <= r < y (generally)
            return x - (q - f) * y;
        }
    }

    struct noop_library
    {
        template <typename T>
        static inline constexpr auto sin(T const&) { return static_cast<T>(0); }

        template <typename T>
        static inline constexpr auto cos(T const&) { return static_cast<T>(0); }

        template <typename T>
        static inline constexpr auto sqrt(T const&) { return static_cast<T>(0); }
    };

    struct std_library
    {
        template <typename T>
        static inline constexpr auto sin(T const&x) { return ::std::sin(x); }

        template <typename T>
        static inline constexpr auto cos(T const&x) { return ::std::cos(x); }

        template <typename T>
        static inline constexpr auto sqrt(T const&x) { return std::sqrt(x); }
    };

/* ========================================= [Constants] ========================================= */

	// Transcendentals:

	static constexpr float pi  = static_cast<float>(FS_PI);
	static constexpr float tau = static_cast<float>(FS_TAU);

	// Rationals:

	static constexpr float one_third  = static_cast<float>(1.0/3.0);
	static constexpr float two_thirds = static_cast<float>(2.0/3.0);

} // namespace Fission::math

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
