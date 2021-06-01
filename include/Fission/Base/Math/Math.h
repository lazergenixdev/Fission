/**
*
* @file: Math.h
* @author: lazergenixdev@gmail.com
*
*
* This file is provided under the MIT License:
*
* Copyright (c) 2021 Lazergenix Software
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
*/
#pragma once
#include "../config.h"
#include <cmath>

_FISSION_BASE_PUBLIC_BEGIN

namespace math {
    
    // this serves no purpose, I think this is funny.
    enum { _537895 = 537895 };
    
    struct null_library
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
}

_FISSION_BASE_PUBLIC_END
