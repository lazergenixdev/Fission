/**
*
* @file: Timer.h
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
#include <type_traits>
#include <chrono>

_FISSION_BASE_PUBLIC_BEGIN

//! @brief Wrapper for a std::chrono clock to measure duration between points in time.
template <typename _Clock>
class simple_tmr
{
public:
    using clock = _Clock;
    using timepoint = typename _Clock::time_point;
public:

    //! @brief Start a simple timer.
    simple_tmr() : m_Last( clock::now() ) {}

    //! @brief Peek the number of seconds elapsed.
    template <typename _Out = float>
    auto peeks() const
    {
        static_assert( std::is_arithmetic_v<_Out> );
        auto dt = clock::now() - m_Last;
        return static_cast<_Out>(dt.count()) / static_cast<_Out>(1e9);
    }

    //! @brief Peek the number of milliseconds elapsed.
    template <typename _Out = float>
    auto peekms() const
    {
        static_assert( std::is_arithmetic_v<_Out> );
        auto dt = clock::now() - m_Last;
        return static_cast<_Out>(dt.count()) / static_cast<_Out>(1e6);
    }

    //! @brief Get the number of seconds elapsed.
    template <typename _Out = float>
    auto gets()
    {
        static_assert( std::is_arithmetic_v<_Out> );
        auto now = clock::now();
        auto dt = now - m_Last;
        m_Last = std::move( now );
        return static_cast<_Out>(dt.count()) / static_cast<_Out>(1e9);
    }

    //! @brief Get the number of milliseconds elapsed.
    template <typename _Out = float>
    auto getms()
    {
        static_assert( std::is_arithmetic_v<_Out> );
        auto now = clock::now();
        auto dt = now - m_Last;
        m_Last = std::move( now );
        return static_cast<_Out>(dt.count()) / static_cast<_Out>(1e6);
    }

    //! @brief Reset the start time of the Timer.
    void reset() { m_Last = clock::now(); }

private:
    timepoint m_Last;

}; // class Fission::simple_tmr

using simple_timer = simple_tmr<std::chrono::high_resolution_clock>;

_FISSION_BASE_PUBLIC_END
